#pragma once
#include"ObjectBase.h"
#include "CTerrainMesh.h"// RoadType.h - 道路の種類を定義

// RoadType.h - 道路の種類を定義
enum class RoadType {
    NONE,        // 道路なし（重要！）
    STRAIGHT,    // 直線
    TURN_LEFT,   // 左カーブ
    TURN_RIGHT,  // 右カーブ
	SLOPE_UP,   // 上り坂
	SLOPE_DOWN, // 下り坂
    // 将来的な拡張用
    START_LINE,  // スタートライン
    CHECKPOINT,  // チェックポイント
    BRIDGE,      // 橋
    TUNNEL       // トンネル
};

// 方向を表す列挙型
enum class Direction {
    NORTH = 0,   // 北 (0度)
    EAST = 90,   // 東 (90度)
    SOUTH = 180, // 南 (180度)
    WEST = 270   // 西 (270度)
};

// 度数法から弧度法への変換ヘルパー関数
namespace MathUtils {
    constexpr float PI = 3.14159265359f;

    inline float DegreesToRadians(float degrees) {
        return degrees * PI / 180.0f;
    }

    inline float RadiansToDegrees(float radians) {
        return radians * 180.0f / PI;
    }

    inline Vector3 DegreesToRadians(const Vector3& degrees) {
        return Vector3(
            DegreesToRadians(degrees.x),
            DegreesToRadians(degrees.y),
            DegreesToRadians(degrees.z)
        );
    }

    // 左手座標系での回転方向
    // Y軸回転: 時計回りが正の角度
    // X軸: 右方向が正  
    // Y軸: 上方向が正
    // Z軸: 奥方向が正
}

// RoadSegment.h - 道路セグメントの情報を管理
struct RoadSegment {
    RoadType type;
    Direction direction;
    Vector3 position;
    Vector3 rotation;
    Vector3 scale;
    float endHeight = 0.0f;  // 道路の終端高さ
    float spacingZ = 0.0f;  // この道路の実際の占有距離

    RoadSegment(RoadType t = RoadType::NONE, Direction d = Direction::NORTH)
        : type(t), direction(d), position(0, 0, 0), rotation(0, 0, 0), scale(18, 1, 18), endHeight(0.0f) {
    }

    // 道路が存在するかチェック
    bool HasRoad() const { return type != RoadType::NONE; }
};

// BaseRoad.h - 基底クラス
class BaseRoad : public ObjectBase {
protected:
    CStaticMeshRenderer m_meshrenderer;
    CStaticMesh m_mesh;
    CTerrainMesh m_terrainMesh;
    CShader m_shader;
    bool m_spatialGridInitialized = false;
    bool m_isInitialized = false;  // 初期化状態を追跡

    // 道路の種類と方向
    RoadType m_roadType;
    Direction m_direction;

    // 各道路タイプに対応するモデルファイル名を取得
    virtual std::string GetModelFileName() const = 0;

public:
    BaseRoad(RoadType type, Direction dir) : m_roadType(type), m_direction(dir) {}
    virtual ~BaseRoad() {
        if (m_isInitialized) {
            Dispose();
        }
    }

    void Init() override;
    void Update(float deltatime) override;
    void Draw() override;
    void Dispose() override;

    // 初期化状態を確認
    bool IsInitialized() const { return m_isInitialized; }

    // 地形追従用メソッド
    bool GetTerrainHeight(const Vector3& position, float& height, Vector3& normal) {
        if (!m_spatialGridInitialized) return false;
        return m_terrainMesh.GetTerrainHeightOptimized(position, height, normal);
    }

    // 当たり判定更新
    void UpdatePlayerCollision(BoundingSphere& player, Vector3& velocity) {
        if (!m_spatialGridInitialized) return;
        m_terrainMesh.UpdatePlayerCollision(player, velocity);
    }

    // アクセサ
    RoadType GetRoadType() const { return m_roadType; }
    Direction GetDirection() const { return m_direction; }

    // 回転設定のヘルパーメソッド（度数法での設定を可能にする）
    void SetRotationDegrees(float pitch, float yaw, float roll) {
        m_Rotation = Vector3(
            MathUtils::DegreesToRadians(pitch),
            MathUtils::DegreesToRadians(yaw),
            MathUtils::DegreesToRadians(roll)
        );
    }

    void SetRotationDegrees(const Vector3& rotationDegrees) {
        m_Rotation = MathUtils::DegreesToRadians(rotationDegrees);
    }

    Vector3 GetRotationDegrees() const {
        return Vector3(
            MathUtils::RadiansToDegrees(m_Rotation.x),
            MathUtils::RadiansToDegrees(m_Rotation.y),
            MathUtils::RadiansToDegrees(m_Rotation.z)
        );
    }
    // モデルの実際のサイズを取得
    Vector3 GetActualModelSize() const {
        return m_mesh.GetModelSize();
    }
};