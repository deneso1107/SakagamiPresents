#pragma once
#include <vector>
#include <memory>
#include"BaseRoad.h"
#include"StraightRoad.h"
#include"RightTurnRoad.h"
#include"LeftTurnRoad.h"
#include <optional> 
#include"Start.h"
class RoadManager {
private:
    std::vector<std::vector<std::unique_ptr<BaseRoad>>> m_roadGrid;
    std::vector<std::vector<RoadSegment>> m_roadLayout;

    int m_gridWidth;
    int m_gridHeight;
    float m_roadSize;  // 道路1つ分のサイズ

    float m_gridSpacingX;  // グリッドのX方向の間隔
    float m_gridSpacingZ;  // グリッドのZ方向の間隔

	float m_RoadLength=0.0f; // 道路の長さ（デフォルト値）
	float startHeight = 0.0f; // 道路の開始高さ
	float m_slopeangle = 5.0f; // 坂の角度

    // ファクトリーメソッド - 道路タイプに応じて適切なインスタンスを生成
    std::unique_ptr<BaseRoad> CreateRoad(RoadType type, Direction direction);

    // 隣接する道路との接続チェック
    bool ValidateConnection(int x, int y, const RoadSegment& segment);

    // 方向に基づく回転角度を計算
    Vector3 CalculateRotation(Direction direction, RoadType roadType);

	float GetPreviousRoadEndHeight(int x, int y, Direction direction);
	float GetRoadEndHeight(RoadType type, Direction direction, float roadLength, float currentHeight);

    float CalculateAccumulatedPosition(int x, int y, Direction direction, float thisSpacing, float, float);

public:
    RoadManager(float roadSize = 18.0f) : m_roadSize(roadSize), m_gridWidth(0), m_gridHeight(0) {
        // デフォルト設定を初期化（Z軸回転をデフォルトに）
       //m_modelConfig.SetAnglesForCurrentModel();
    }
    ~RoadManager() = default;

    // レイアウトを設定してサーキットを生成
    void InitializeCircuit(const std::vector<std::vector<RoadSegment>>& layout);

    // 個別の道路を配置
    void SetRoad(int x, int y, RoadType type, Direction direction = Direction::NORTH);

    // 道路を削除
    void RemoveRoad(int x, int y);

    // グリッドサイズを変更
    void ResizeGrid(int width, int height);

    // 全道路を更新
    void UpdateAll(uint64_t deltatime);

    // 全道路を描画
    void DrawAll();

    // リソース解放
    void DisposeAll();

    void InitializeGridSpacing()
    {
        // 仮の道路を作成してサイズを測定
        auto tempRoad = std::make_unique<StraightRoad>(Direction::NORTH);
        tempRoad->SetScale(Vector3(m_roadSize, 1.0f, m_roadSize));
        tempRoad->Init();

        Vector3 size = tempRoad->GetActualModelSize();
        m_gridSpacingX = std::max(size.x, size.z) * m_roadSize;
        m_gridSpacingZ = std::max(size.x, size.z) * m_roadSize;

        tempRoad.reset();

        printf("Grid spacing initialized: X=%.2f, Z=%.2f\n", m_gridSpacingX, m_gridSpacingZ);
    }

    void SetRoadRotation(int, int, const Vector3&);
    // プレイヤーとの地形追従・当たり判定
    bool GetTerrainHeight(const Vector3& position, float& height, Vector3& normal);
    void UpdatePlayerCollision(BoundingSphere& player, Vector3& velocity);

    // アクセサ
    int GetGridWidth() const { return m_gridWidth; }
    int GetGridHeight() const { return m_gridHeight; }
    float GetRoadSize() const { return m_roadSize; }
    void SetRoadSize(float size) { m_roadSize = size; }

    // 度数法での回転設定をサポート
    void SetRoadRotationDegrees(int x, int y, float pitch, float yaw, float roll) {
        if (x < 0 || x >= m_gridWidth || y < 0 || y >= m_gridHeight) return;
        if (m_roadGrid[y][x]) {
            m_roadGrid[y][x]->SetRotationDegrees(pitch, yaw, roll);  // -> 演算子を使用
        }
    }

    void SetRoadRotationDegrees(int x, int y, const Vector3& rotationDegrees) {
        if (x < 0 || x >= m_gridWidth || y < 0 || y >= m_gridHeight) return;
        if (m_roadGrid[y][x]) {
            m_roadGrid[y][x]->SetRotationDegrees(rotationDegrees);  // -> 演算子を使用
        }
    }

    std::optional<Vector3> GetStartPos();

    // デバッグ用: 回転軸を指定して角度を設定
    enum class RotationAxis { X_PITCH, Y_YAW, Z_ROLL };
    void SetRoadRotationAxis(int x, int y, RotationAxis axis, float degrees) {
        if (x < 0 || x >= m_gridWidth || y < 0 || y >= m_gridHeight) return;
        if (m_roadGrid[y][x]) {
            Vector3 rotation = Vector3(0, 0, 0);
            switch (axis) {
            case RotationAxis::X_PITCH:
                rotation.x = MathUtils::DegreesToRadians(degrees);
                break;
            case RotationAxis::Y_YAW:
                rotation.y = MathUtils::DegreesToRadians(degrees);
                break;
            case RotationAxis::Z_ROLL:
                rotation.z = MathUtils::DegreesToRadians(degrees);
                break;
            }
            m_roadGrid[y][x]->SetRotation(rotation);  // -> 演算子を使用
        }
    }

    // デバッグ用 - サーキットの妥当性をチェック
    bool ValidateCircuit() const;

    // プリセットレイアウト生成メソッド
    static std::vector<std::vector<RoadSegment>> CreateSimpleOval(int width, int height);
    static std::vector<std::vector<RoadSegment>> CreateFigureEight(int size);
    static std::vector<std::vector<RoadSegment>> CreateTestStraightGrid(int width, int height);  // デバッグ用
    static std::vector<std::vector<RoadSegment>> CreateCustomCircuit();  // 3x3カスタムサーキット
    static std::vector<std::vector<RoadSegment>> CreateCustomRectangleCircuit(int width, int height);  // 可変サイズ
};