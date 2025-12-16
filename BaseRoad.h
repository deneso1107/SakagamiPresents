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
    GOAL_LINE,  // スタートライン
    DIRT,  // ダート
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

enum class EdgeType {
    NONE,           // 中央
    LEFT,           // 左端
    RIGHT,          // 右端
    FRONT,          // 前端
    BACK,           // 後端
    CORNER          // 角
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
        : type(t), direction(d), position(0, 0, 0), rotation(0, 0, 0), scale(14, 1, 14), endHeight(0.0f) {//道路のサイズを変える場合はRoadManagerのコンストラクタも変更
	}//また、パーティクルを出す位置も変える必要がある

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
    CShader m_shadowShader;        // 通常描画用（影あり）- 新規追加
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


    //路面タイプを取得
    RoadType GetSurfaceType() const 
    {
        return m_roadType;  // RoadTypeがそのまま路面タイプを示す
    }

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

    Vector3 GetModelEdge()
    {
        Vector3 actualSize = GetActualModelSize();
        return Vector3(
            actualSize.x + m_Scale.x/2,
            actualSize.y,
            actualSize.z
		);
    }

    bool IsPlayerOnEdge(const Vector3& playerPos, float edgeThreshold = 1.0f) const;
    EdgeType GetPlayerEdgeType(const Vector3& playerPos, float edgeThreshold = 1.0f) const;
};