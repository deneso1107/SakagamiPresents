#pragma once
#include"ObjectBase.h"
#include"CTerrainMesh.h"
class Road : public ObjectBase
{
    // 描画の為の情報（メッシュに関わる情報）
    CStaticMeshRenderer m_meshrenderer;
    CStaticMesh         m_mesh;                         // メッシュデータ
    // 当たり判定用の地形メッシュ（描画は行わない）
    CTerrainMesh        m_terrainMesh;                  // 当たり判定専用
    // 目標回転角度
    Vector3 m_Destrot = { 0.0f,0.0f,0.0f };
    // 描画の為の情報（見た目に関わる部分）
    CShader m_shader;   // シェーダ
    // 速度ベクトル（前回の移動方向を保持）
    Vector3 m_Velocity = Vector3(0.0f, 0.0f, 0.0f);
    // 空間分割の初期化フラグ
    bool m_spatialGridInitialized = false;

public:
    void Init() override;
    void Update(float) override;
    void Draw() override;
    void Dispose() override;


    const CStaticMesh& GetMesh() {
        return m_mesh;
    }

    // プレイヤーの地形追従用メソッド
    bool GetTerrainHeight(const Vector3& position, float& height, Vector3& normal) {
        if (!m_spatialGridInitialized) {
            return false;
        }
        return m_terrainMesh.GetTerrainHeightOptimized(position, height, normal);
    }

    // プレイヤーの当たり判定更新
    void UpdatePlayerCollision(BoundingSphere& player, Vector3& velocity) {
        if (!m_spatialGridInitialized) {
            return;
        }
        m_terrainMesh.UpdatePlayerCollision(player, velocity);
    }
};