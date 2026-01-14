#include "TreeManager.h"
#include <random>
#include <algorithm>

// 静的メンバの定義
CStaticMesh TreeManager::s_TreeMesh{};
CStaticMeshRenderer TreeManager::s_TreeMeshRenderer{};
CShader TreeManager::s_TreeShader{};
bool TreeManager::s_ResourcesInitialized = false;


// 共有リソースの初期化（一度だけ実行される）
void TreeManager::InitializeSharedResources()
{
    if (s_ResourcesInitialized)
    {
        return;  // 既に初期化済み
    }

    // モデルの初期化（一度だけ）
    s_TreeMesh.Load(
        "assets/model/Tree/tree_1126023743_texture.fbx",
        "assets/model/");

    // レンダラ初期化
    s_TreeMeshRenderer.Init(s_TreeMesh);

    // シェーダーの初期化
    s_TreeShader.Create(
        "shader/vertexLightingVS.hlsl",
        "shader/vertexLightingPS.hlsl");

    s_ResourcesInitialized = true;

    printf("Tree shared resources initialized.\n");
}

// 内部関数：配置パターンに応じた位置・回転・スケールを計算
void TreeManager::CalculateTreeTransform(
    const TreeFormationConfig& config,
    int index,
    int startIndex,
    Vector3& outPos,
    Vector3& outRot,
    Vector3& outScale,
    std::mt19937& mt)
{
    int localIndex = index - startIndex;  // このパターン内でのインデックス

    // スケールの初期化
    float normalScale = config.minScale + config.maxScale / 2;
    outScale = Vector3(normalScale, normalScale, normalScale);
    if (config.randomizeScale)
    {
        std::uniform_real_distribution<float> scaleDist(config.minScale, config.maxScale);
        float scale = scaleDist(mt);
        outScale = Vector3(scale, scale, scale);
    }
    // 回転の初期化
    outRot = Vector3(0.0f, 0.0f, 0.0f);
    if (config.randomizeRotation)
    {
        std::uniform_real_distribution<float> rotDist(0.0f, PI * 2.0f);
        outRot.y = rotDist(mt);
    }

	float Offset_Y = -s_TreeMesh.GetBottomY() * outScale.y; // 底面の位置に合わせてYオフセットを調整
    float adjustedYOffset = config.yOffset - Offset_Y;
        
    // 位置の計算
    switch (config.formation)
    {
    case TreeFormation::RANDOM:
    {
        // ランダム配置
        std::uniform_real_distribution<float> posDist(-config.randomRange, config.randomRange);
        outPos = Vector3(posDist(mt), Offset_Y, posDist(mt));
        break;
    }

    case TreeFormation::LINE:
    {
        // 直線配置（Z軸方向に並べる）
        outPos = config.centerPos;
        outPos.y += Offset_Y;  // Y軸オフセット追加
        outPos.z += localIndex * config.spacing;

        // 少しランダムなずれを追加（自然な感じに）
        if (config.randomizeScale)  // randomizeScaleをランダムずれの判定にも使用
        {
            std::uniform_real_distribution<float> offsetDist(-config.spacing * 0.2f, config.spacing * 0.2f);
            outPos.x += offsetDist(mt);
        }
        break;
    }

    case TreeFormation::LINE_X:
    {
        // 直線配置（X軸方向に並べる・横）
        outPos = config.centerPos;
        outPos.y += adjustedYOffset;  // 調整済みY軸オフセット使用
        outPos.x += localIndex * config.spacing;

        // 少しランダムなずれを追加（自然な感じに）
        if (config.randomizeScale)
        {

            std::uniform_real_distribution<float> offsetDist(-config.spacing * 0.2f, config.spacing * 0.2f);
            outPos.z += offsetDist(mt);
        }
        break;
    }

    case TreeFormation::CIRCLE:
    {
        // 円形配置
        int treeCountInFormation = config.treeCount > 0 ? config.treeCount : 1;
        float angle = (2.0f * PI * localIndex) / treeCountInFormation;
        outPos = config.centerPos;
        outPos.y += Offset_Y;  // Y軸オフセット追加
        outPos.x += cos(angle) * config.circleRadius;
        outPos.z += sin(angle) * config.circleRadius;
        break;
    }

    case TreeFormation::GRID:
    {
        // グリッド配置
        int row = localIndex / config.columns;
        int col = localIndex % config.columns;
        outPos = config.centerPos;
        outPos.y += Offset_Y;  // Y軸オフセット追加
        outPos.x += (col - config.columns / 2.0f) * config.spacing;
        outPos.z += row * config.spacing;

        // 少しランダムなずれを追加
        if (config.randomizeScale)
        {
            std::uniform_real_distribution<float> offsetDist(-config.spacing * 0.15f, config.spacing * 0.15f);
            outPos.x += offsetDist(mt);
            outPos.z += offsetDist(mt);
        }
        break;
    }

    case TreeFormation::FOREST:
    {
        // 森（ランダムだが密集）
        std::uniform_real_distribution<float> posDist(-config.randomRange * 0.5f, config.randomRange * 0.5f);
        outPos = config.centerPos;
        outPos.y += Offset_Y;  // Y軸オフセット追加
        outPos.x += posDist(mt);
        outPos.z += posDist(mt);
        break;
    }

    case TreeFormation::BORDER:
    {
        // 境界線配置（四角い枠）
        int treeCountInFormation = config.treeCount > 0 ? config.treeCount : 1;
        int treesPerSide = treeCountInFormation / 4;
        int sideIndex = localIndex / treesPerSide;
        int positionOnSide = localIndex % treesPerSide;

        float halfSize = config.circleRadius;  // circleRadiusを四角の半径として使用

        outPos = config.centerPos;
        outPos.y += Offset_Y;  // Y軸オフセット追加

        switch (sideIndex)
        {
        case 0: // 上辺
            outPos.x = -halfSize + (positionOnSide * config.spacing);
            outPos.z = -halfSize;
            break;
        case 1: // 右辺
            outPos.x = halfSize;
            outPos.z = -halfSize + (positionOnSide * config.spacing);
            break;
        case 2: // 下辺
            outPos.x = halfSize - (positionOnSide * config.spacing);
            outPos.z = halfSize;
            break;
        case 3: // 左辺
            outPos.x = -halfSize;
            outPos.z = halfSize - (positionOnSide * config.spacing);
            break;
        }
        break;
    }

    default:
        // デフォルトはランダム
        std::uniform_real_distribution<float> posDist(-config.randomRange, config.randomRange);
        outPos = Vector3(posDist(mt), Offset_Y, posDist(mt));
        break;
    }
}

// 単一パターン版
void TreeManager::Init(const TreeFormationConfig& config)
{
    MultiTreeFormationConfig multiConfig;
    multiConfig.totalTreeCount = config.treeCount > 0 ? config.treeCount : 100;
    multiConfig.AddFormation(config);
    Init(multiConfig);
}

void TreeManager::InitShared()
{
    // 共有リソースを初期化（一度だけ実行される）
    InitializeSharedResources();
}

// 複数パターン版（メイン実装）
void TreeManager::Init(const MultiTreeFormationConfig& config)
{
    // 共有リソースを初期化（一度だけ実行される）
        InitializeSharedResources();

    // バリデーションチェック
    if (!config.Validate())
    {
        printf("Error: Total tree count in formations exceeds totalTreeCount!\n");
        return;
    }

    if (config.formations.empty())
    {
        printf("Error: No formation patterns specified for trees!\n");
        return;
    }

    // 既存の木をクリア
    m_Trees.clear();

    std::mt19937 mt{ std::random_device{}() };

    // 各配置パターンで指定された数の合計を計算
    int specifiedTotal = 0;
    for (const auto& formation : config.formations)
    {
        specifiedTotal += formation.treeCount;
    }

    // 残りの木の数を計算
    int remainingTrees = config.totalTreeCount - specifiedTotal;
    if (remainingTrees < 0)
    {
        remainingTrees = 0;
    }

    int currentIndex = 0;

    // 各配置パターンで木を生成
    for (const auto& formation : config.formations)
    {
        int countForThisFormation = formation.treeCount;

        // treeCountが0の場合は残り全部を使用
        if (countForThisFormation == 0)
        {
            countForThisFormation = remainingTrees;
            remainingTrees = 0;
        }

        // 安全チェック：総数を超えないように
        if (currentIndex + countForThisFormation > config.totalTreeCount)
        {
            countForThisFormation = config.totalTreeCount - currentIndex;
        }

        // この配置パターンで木を生成
        for (int i = 0; i < countForThisFormation; i++)
        {
            if (currentIndex >= config.totalTreeCount)
            {
                break;  // 総数に達したら終了
            }

            std::unique_ptr<Tree> tree = std::make_unique<Tree>();

            // nullptrチェック
            if (!tree)
            {
                printf("Error: Failed to create tree at index %d\n", currentIndex);
                continue;
            }

            // Treeの初期化はしない（モデル読み込みをスキップ）
            // 代わりに共有リソースを使用

            Vector3 pos, rot, scale;
            CalculateTreeTransform(formation, currentIndex, currentIndex - i, pos, rot, scale, mt);

            tree->SetPosition(pos);
            tree->SetRotation(rot);
            tree->SetScale(scale);

            m_Trees.emplace_back(std::move(tree));
            currentIndex++;
        }
    }

    // 最終チェック：生成された木の数を確認
    printf("Initialized %d trees (requested: %d)\n", (int)m_Trees.size(), config.totalTreeCount);
}

void TreeManager::Update(float deltaTime)
{
    // 空チェック
    if (m_Trees.empty())
    {
        return;
    }

    for (auto& tree : m_Trees)
    {
        // nullptrチェック
        if (tree)
        {
            tree->Update(deltaTime);
        }
    }
}


void TreeManager::Draw()
{
    // 空チェック
    if (m_Trees.empty())
    {
        return;
    }

    // 共有リソースが初期化されていない場合は描画しない
    if (!s_ResourcesInitialized)
    {
        printf("Warning: Tree resources not initialized!\n");
        return;
    }

    // シェーダーを一度だけセット
    s_TreeShader.SetGPU();

    // 全ての木を描画
    for (auto& tree : m_Trees)
    {
        // nullptrチェック
        if (tree)
        {
            // SRT情報作成
            SRT srt;
            srt.pos = tree->GetPosition();
            srt.rot = tree->GetRotation();
            srt.scale = tree->GetScale();

            Matrix4x4 worldmtx = srt.GetMatrix();
            DirectX::SimpleMath::Matrix rotationMatrix = DirectX::SimpleMath::Matrix::CreateRotationX(DirectX::XMConvertToRadians(90.0f)); // または -90.0f
            worldmtx = rotationMatrix * srt.GetMatrix();
            Renderer::SetWorldMatrix(&worldmtx);

            // 共有メッシュレンダラーで描画
            s_TreeMeshRenderer.Draw();
        }
    }
}

void TreeManager::Dispose()
{
    DisposeSharedResources();
    // 全ての木を破棄
    for (auto& tree : m_Trees)
    {
        if (tree)
        {
            tree->Dispose();
        }
    }

    m_Trees.clear();
}

void TreeManager::DisposeSharedResources()
{
    if (!s_ResourcesInitialized)
    {
        return;
    }

    // 必要に応じてリソースの破棄処理を追加
    s_ResourcesInitialized = false;

    printf("Tree shared resources disposed.\n");
}