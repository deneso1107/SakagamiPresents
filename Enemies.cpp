#include "system/commontypes.h"
#include <vector>
#include <memory>
#include <random>
#include "system/CStaticMesh.h"
#include "system/CStaticMeshRenderer.h"
#include "system/CShader.h"
#include "Enemies.h"

// 敵多数
std::vector<std::unique_ptr<Enemy>> g_Enemies;

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

    //case EnemyFormation::GRID:
    //{
    //    // グリッド配置
    //    int row = localIndex / config.columns;
    //    int col = localIndex % config.columns;
    //    outPos = config.centerPos;
    //    outPos.x += (col - config.columns / 2.0f) * config.spacing;
    //    outPos.z += row * config.spacing;
    //    outRot.y = 0.0f;
    //    break;
    //}

    //case EnemyFormation::DOUBLE_LINE:
    //{
    //    // 2列配置
    //    int row = localIndex % 2;  // 0 or 1
    //    int colIndex = localIndex / 2;
    //    outPos = config.centerPos;
    //    outPos.x += (row == 0 ? -config.spacing * 0.5f : config.spacing * 0.5f);
    //    outPos.z += colIndex * config.spacing;
    //    outRot.y = 0.0f;
    //    break;
    //}

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
        "assets/model/car001.x",
        "assets/model/");
    // レンダラ初期化
    g_EnemyMeshRenderer.Init(g_EnemyMesh);
    // シェーダーの初期化
    g_Shader.Create(
        "shader/vertexLightingVS.hlsl",
        "shader/vertexLightingPS.hlsl");

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

//void InitEnemies(IScene* currentscene,Field* field)
//{
//	std::mt19937 mt{ std::random_device{}() };
//	std::uniform_real_distribution<float> posdist{ -500.0f, 500.0f };
//
//	std::uniform_real_distribution<float> rotdist{ 0.0f, PI };
//
//	// モデルの初期化
//	g_EnemyMesh.Load(
//		"assets/model/car001.x",
//		"assets/model/");
//
//	// レンダラ初期化
//	g_EnemyMeshRenderer.Init(g_EnemyMesh);
//
//	// シェーダーの初期化
//	g_Shader.Create(
//		"shader/vertexLightingVS.hlsl",			// 頂点シェーダー
//		"shader/vertexLightingPS.hlsl");		// ピクセルシェーダー
//
//	for(int i = 0; i < ENEMYMAX; i++)
//	{
//		std::unique_ptr<Enemy> enemy = std::make_unique<Enemy>(currentscene);
//		enemy->Init();
//		enemy->SetPosition(Vector3(posdist(mt), 0.0f, posdist(mt)));
//		enemy->SetRotation(Vector3(0.0, rotdist(mt), 0.0));
//		enemy->SetMeshRenderer(&g_EnemyMeshRenderer);
//		enemy->SetField(field); // Enemy に渡す
//		g_Enemies.emplace_back(std::move(enemy));
//	}
//
//}

void UpdateEnemies(float deltaTime)
{
	for (auto& e : g_Enemies)
	{
		e->Update(deltaTime);
	}
}

void DrawEnemies() 
{

	g_Shader.SetGPU();

	for (auto& e:g_Enemies)
	{
		if (e->GetActive())
		{
			e->Draw();
		}
	}
}

void DisposeEnemies() 
{

}

std::vector<SRT> GetAllRTS() 
{
	std::vector<SRT> allrts;
	for (auto& e : g_Enemies)
	{
		SRT r;
		r.pos = e->GetPosition();
		r.rot = e->GetRotation();
		r.scale = e->GetScale();
		allrts.emplace_back(r);
	}
	return allrts;
}

std::vector<std::unique_ptr<Enemy>>& GetAllEnemys()
{
	return g_Enemies; // コピーではなく参照を返すのでOK
}

GM31::GE::Collision::BoundingSphere GetEnemyBoundingSphere(const Enemy& enemy)//Enemyの当たり判定を取得する関数
{
	GM31::GE::Collision::BoundingSphere sphere;
	sphere.center = enemy.GetPosition();
	sphere.radius = sphere.center.x; // X座標を半径として使用(カスコード)
	return sphere;
}