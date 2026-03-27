#pragma once
#include "Tree.h"
#include <vector>
#include <memory>
#include <random>

// 配置パターンの列挙型
enum class TreeFormation
{
    RANDOM,      // ランダム配置
    LINE,        // 直線配置
    LINE_X,        // 直線配置(X軸方向)
    CIRCLE,      // 円形配置
    GRID,        // グリッド配置
    FOREST,      // 森（ランダムだが密集）
    BORDER       // 境界線配置（四角い枠）
};

// 木の配置設定構造体
struct TreeFormationConfig
{
    TreeFormation formation = TreeFormation::RANDOM;
    Vector3 centerPos = Vector3(0.0f, 0.0f, 0.0f);  // 配置の中心座標
    float spacing = 20.0f;           // 木同士の間隔
    float circleRadius = 100.0f;     // 円形配置の半径
    float randomRange = 500.0f;      // ランダム配置の範囲
    int columns = 10;                // グリッド配置の列数
    int treeCount = 0;               // この配置パターンで生成する木の数（0なら残り全部）

    // スケールのランダム化設定
    float minScale = 8.0f;           // 最小スケール
    float maxScale = 12.0f;           // 最大スケール
    bool randomizeScale = true;      // スケールをランダム化するか

    // 回転のランダム化設定
    bool randomizeRotation = true;   // Y軸回転をランダム化するか

    // Y軸オフセット（地面に埋まらないように調整）
    float yOffset = 25.0f;            // Y座標に加算する値（モデルの高さの半分程度を推奨）

    TreeFormationConfig() = default;

    // 便利なコンストラクタ
    TreeFormationConfig(TreeFormation form, int count, Vector3 center = Vector3(0.0f, 0.0f, 0.0f))
        : formation(form), treeCount(count), centerPos(center) {
    }
};

// 複数配置パターンをまとめた構造体
struct MultiTreeFormationConfig
{
    std::vector<TreeFormationConfig> formations;
    int totalTreeCount = 100;  // 生成する木の総数

    // 配置パターンを追加
    void AddFormation(const TreeFormationConfig& config)
    {
        formations.push_back(config);
    }

    // バリデーション（配置数の合計チェック）
    bool Validate() const
    {
        int specified = 0;
        for (const auto& config : formations)
        {
            specified += config.treeCount;
        }
        // 指定された数が総数を超えていないかチェック
        return specified <= totalTreeCount;
    }
};

class TreeManager
{
private:
    std::vector<std::unique_ptr<Tree>> m_trees;

    // 共有リソース（全ての木で共有）
    static CStaticMesh m_treeMesh;
    static CStaticMeshRenderer m_treeMeshRenderer;
    static CShader m_treeShader;
    static bool m_resourcesInitialized;

    // 内部関数：配置パターンに応じた位置・回転・スケールを計算
    void CalculateTreeTransform(
        const TreeFormationConfig& config,
        int index,
        int startIndex,
        Vector3& outPos,
        Vector3& outRot,
        Vector3& outScale,
        std::mt19937& mt);

    // 共有リソースの初期化
    void InitializeSharedResources();

public:
    TreeManager() = default;
    ~TreeManager() = default;

    // 初期化（単一パターン）
    void Init(const TreeFormationConfig& config);

    // 初期化（複数パターン）
    void Init(const MultiTreeFormationConfig& config);

    void InitShared();

    // 更新
    void Update(float deltaTime);

    // 描画
    void Draw();

    // 破棄
    void Dispose();

    // 共有リソースの破棄（アプリ終了時に呼ぶ）
    static void DisposeSharedResources();

    // 木の取得
    std::vector<std::unique_ptr<Tree>>& GetAllTrees() { return m_trees; }
    const std::vector<std::unique_ptr<Tree>>& GetAllTrees() const { return m_trees; }

    // 木の数を取得
    int GetTreeCount() const { return static_cast<int>(m_trees.size()); }
};