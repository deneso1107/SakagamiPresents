#include "system/commontypes.h"
#include <vector>
#include <memory>
#include <random>
#include <algorithm>
#include "system/CStaticMesh.h"
#include "system/CStaticMeshRenderer.h"
#include "system/CShader.h"
#include "Enemies.h"

// 敵多数
std::vector<std::unique_ptr<Enemy>> g_Enemies;

std::vector<std::unique_ptr<WeavingEnemy>> g_WeavingEnemies;
// 敵のメッシュデータ
static CStaticMesh g_EnemyMesh{};
static CStaticMeshRenderer g_EnemyMeshRenderer{};
// シェーダー
static CShader g_Shader{};

// メッシュ
CStaticMesh* GetEnemyMesh()
{
    return &g_EnemyMesh;
}

// 内部関数：指定された配置パターンで位置と回転を計算
static void CalculateEnemyTransform(
    const FormationConfig& config,
    int index,
    int startIndex,
    Vector3& outPos,
    Vector3& outRot,
    std::mt19937& mt,
    std::uniform_real_distribution<float>& posdist,
    std::uniform_real_distribution<float>& rotdist)
{
    outRot = Vector3(0.0f, 0.0f, 0.0f);
    int localIndex = index - startIndex;  // このパターン内でのインデックス

    switch (config.formation)
    {
    case EnemyFormation::RANDOM:
        // ランダム配置
        outPos = Vector3(posdist(mt), 0.0f, posdist(mt));
        outRot.y = rotdist(mt);
        break;

    case EnemyFormation::LINE:
        // 縦列配置（Z軸方向に並べる）
        outPos = config.centerPos;
        outPos.z += localIndex * config.spacing;
        outRot.y = 0.0f;
        break;

    case EnemyFormation::CIRCLE:
    {
        // 円形配置
        int enemyCountInFormation = config.enemyCount > 0 ? config.enemyCount : 1;
        float angle = (2.0f * PI * localIndex) / enemyCountInFormation;
        outPos = config.centerPos;
        outPos.x += cos(angle) * config.circleRadius;
        outPos.z += sin(angle) * config.circleRadius;
        // 中心を向くように回転
        outRot.y = angle + PI;
        break;
    }

    case EnemyFormation::GRID:
    {
        // グリッド配置
        int row = localIndex / config.columns;
        int col = localIndex % config.columns;
        outPos = config.centerPos;
        outPos.x += (col - config.columns / 2.0f) * config.spacing;
        outPos.z += row * config.spacing;
        outRot.y = 0.0f;
        break;
    }

    case EnemyFormation::DOUBLE_LINE:
    {
        // 2列配置
        int row = localIndex % 2;  // 0 or 1
        int colIndex = localIndex / 2;
        outPos = config.centerPos;
        outPos.x += (row == 0 ? -config.spacing * 0.5f : config.spacing * 0.5f);
        outPos.z += colIndex * config.spacing;
        outRot.y = 0.0f;
        break;
    }

    case EnemyFormation::DIAGONAL:
    {
        // 斜め配置
        // 角度を度数法からラジアンに変換
        float angleRad = config.diagonalAngle * (PI / 180.0f);

        // 配置の開始位置
        outPos = config.centerPos;

        // 斜め方向のベクトルを計算
        float dirX = sin(angleRad);  // X方向の成分
        float dirZ = cos(angleRad);  // Z方向の成分

        // インデックスに応じて斜め方向に配置
        outPos.x += dirX * localIndex * config.spacing;
        outPos.z += dirZ * localIndex * config.spacing;

        // 敵の向きは進行方向に設定
        outRot.y = angleRad;
        break;
    }

    default:
        // デフォルトはランダム
        outPos = Vector3(posdist(mt), 0.0f, posdist(mt));
        outRot.y = rotdist(mt);
        break;
    }
}

// 既存のランダム配置版（互換性維持）
void InitEnemies(IScene* currentscene, Field* field)
{
    FormationConfig config;
    config.formation = EnemyFormation::RANDOM;
    config.enemyCount = ENEMYMAX;
    InitEnemiesWithFormation(currentscene, field, config);
}

// 単一配置パターン版
void InitEnemiesWithFormation(IScene* currentscene, Field* field, const FormationConfig& config)
{
    MultiFormationConfig multiConfig;
    multiConfig.totalEnemyCount = config.enemyCount > 0 ? config.enemyCount : ENEMYMAX;
    multiConfig.AddFormation(config);
    InitEnemiesWithMultiFormation(currentscene, field, multiConfig);
}

// 複数配置パターン版（メイン実装）
void InitEnemiesWithMultiFormation(IScene* currentscene, Field* field, const MultiFormationConfig& config)
{
    // バリデーションチェック
    if (!config.Validate())
    {
        // エラー：指定された敵の数が総数を超えている
        printf("Error: Total enemy count in formations exceeds totalEnemyCount!\n");
        return;
    }

    if (config.formations.empty())
    {
        // エラー：配置パターンが指定されていない
        printf("Error: No formation patterns specified!\n");
        return;
    }

    // 既存の敵をクリア
    g_Enemies.clear();

    std::mt19937 mt{ std::random_device{}() };
    std::uniform_real_distribution<float> posdist{ -500.0f, 500.0f };
    std::uniform_real_distribution<float> rotdist{ 0.0f, PI };

    // モデルの初期化
    g_EnemyMesh.Load(
        "assets/model/kakasi/Untitled.fbx",
        "assets/model/");
    // レンダラ初期化
    g_EnemyMeshRenderer.Init(g_EnemyMesh);
    // シェーダーの初期化
    g_Shader.Create(
        "shader/unlitTextureVS.hlsl",
        "shader/unlitTexturePS.hlsl");

    // 各配置パターンで指定された数の合計を計算
    int specifiedTotal = 0;
    for (const auto& formation : config.formations)
    {
        specifiedTotal += formation.enemyCount;
    }

    // 残りの敵の数を計算
    int remainingEnemies = config.totalEnemyCount - specifiedTotal;
    if (remainingEnemies < 0)
    {
        remainingEnemies = 0;
    }

    int currentIndex = 0;

    // 各配置パターンで敵を生成
    for (const auto& formation : config.formations)
    {
        int countForThisFormation = formation.enemyCount;

        // enemyCountが0の場合は残り全部を使用
        if (countForThisFormation == 0)
        {
            countForThisFormation = remainingEnemies;
            remainingEnemies = 0;
        }

        // 安全チェック：総数を超えないように
        if (currentIndex + countForThisFormation > config.totalEnemyCount)
        {
            countForThisFormation = config.totalEnemyCount - currentIndex;
        }

        // この配置パターンで敵を生成
        for (int i = 0; i < countForThisFormation; i++)
        {
            if (currentIndex >= config.totalEnemyCount)
            {
                break;  // 総数に達したら終了
            }

            std::unique_ptr<Enemy> enemy = std::make_unique<Enemy>(currentscene);

            // nullptrチェック
            if (!enemy)
            {
                printf("Error: Failed to create enemy at index %d\n", currentIndex);
                continue;
            }

            enemy->Init();

            Vector3 pos, rot;
            CalculateEnemyTransform(formation, currentIndex, currentIndex - i, pos, rot, mt, posdist, rotdist);

            enemy->SetPosition(pos);
            enemy->SetRotation(rot);
            enemy->SetMeshRenderer(&g_EnemyMeshRenderer);

            // fieldのnullptrチェック
            if (field)
            {
                enemy->SetField(field);
            }
            else
            {
                printf("Warning: Field pointer is null for enemy at index %d\n", currentIndex);
            }

            g_Enemies.emplace_back(std::move(enemy));
            currentIndex++;
        }
    }

    // 最終チェック：生成された敵の数を確認
    printf("Initialized %d enemies (requested: %d)\n", (int)g_Enemies.size(), config.totalEnemyCount);
}

void InitWeavingEnemies(
    IScene* currentscene,
    Field* field,
    MoveDirection direction,
    int           count,
    Vector3       startPos,
    float         spacing,
    bool clearFirst,
    BaseRoad* linkedRoad)
{
    if (clearFirst)  // ← 最初の1回だけclear
        g_WeavingEnemies.clear();

    // メッシュは既存の g_EnemyMesh / g_EnemyMeshRenderer を共用
    // （InitEnemies 系をあらかじめ呼んでおくこと）

    // 前進方向ベクトルを取得して縦列に並べる
    Vector3 forwardDir = { 0,0,0 };
    switch (direction)
    {
    case MoveDirection::NORTH: forwardDir = { 0.0f, 0.0f,  1.0f }; break;
    case MoveDirection::SOUTH: forwardDir = { 0.0f, 0.0f, -1.0f }; break;
    case MoveDirection::EAST:  forwardDir = { 1.0f, 0.0f,  0.0f }; break;
    case MoveDirection::WEST:  forwardDir = { -1.0f, 0.0f,  0.0f }; break;
    }

    for (int i = 0; i < count; ++i)
    {
        auto enemy = std::make_unique<WeavingEnemy>(currentscene);

        // 縦列に spacing ずつ後ろにオフセット
        Vector3 pos = startPos + forwardDir * (-spacing * i);
        enemy->SetPosition(pos);

        // Init はSetPosition後に呼ぶ（startPosition記録のため）
        enemy->Init();

        enemy->SetMoveDirection(direction);
        enemy->SetMeshRenderer(GetEnemyMesh() ? &g_EnemyMeshRenderer : nullptr);

        // ↓ パラメータは必要に応じて調整
         //enemy->SetMoveSpeed(5.0f);
         //enemy->SetWeaveAmplitude(10.0f);
         //enemy->SetWeaveFrequency(1.5f);
         //enemy->SetTravelLimit(200.0f);
         //enemy->SetTimeLimit(5.0f);

        if (field) enemy->SetField(field);

        enemy->SetLinkedRoad(linkedRoad);  // ← 紐づけ追加

        g_WeavingEnemies.emplace_back(std::move(enemy));
    }

    printf("Initialized %d WeavingEnemies\n", (int)g_WeavingEnemies.size());
}

void UpdateEnemies(float deltaTime)
{
    // 空チェック
    if (g_Enemies.empty())
    {
        return;
    }

    for (auto& e : g_Enemies)
    {
        // nullptrチェック
        if (e)
        {
            e->Update(deltaTime);
        }
    }

    for (auto& e : g_WeavingEnemies)
    {
        if (e)
        {
            e->Update(deltaTime);
        }
    }
}

void DrawEnemies()
{
    // 空チェック
    if (g_Enemies.empty())
    {
        return;
    }

    g_Shader.SetGPU();
    for (auto& e : g_Enemies)
    {
        // nullptrチェックとアクティブチェック
        if (e && e->GetActive())
        {
            e->Draw();
        }
    }
    // --- WeavingEnemy ---
    for (auto& e : g_WeavingEnemies)
    {
         if (e && e->GetActive())
         {
             e->Draw();
		 }
    }
}

void DisposeEnemies()
{
    g_Enemies.clear();
    g_WeavingEnemies.clear();
}

std::vector<SRT> GetAllRTS()
{
    std::vector<SRT> allrts;

    // 空チェック
    if (g_Enemies.empty())
    {
        return allrts;
    }

    allrts.reserve(g_Enemies.size());  // メモリ効率化

    for (const auto& e : g_Enemies)
    {
        // nullptrチェック
        if (e)
        {
            SRT r;
            r.pos = e->GetPosition();
            r.rot = e->GetRotation();
            r.scale = e->GetScale();
            allrts.emplace_back(r);
        }
    }
    return allrts;
}

std::vector<std::unique_ptr<Enemy>>& GetAllEnemys()
{
    return g_Enemies;
}

std::vector<std::unique_ptr<WeavingEnemy>>& GetAllWeavingEnemies()
{
    return g_WeavingEnemies;
}

GM31::GE::Collision::BoundingSphere GetEnemyBoundingSphere(const Enemy& enemy)
{
    GM31::GE::Collision::BoundingSphere sphere;
    sphere.center = enemy.GetPosition();
    sphere.radius = sphere.center.x;
    return sphere;
}