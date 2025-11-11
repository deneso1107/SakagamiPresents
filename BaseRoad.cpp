#include "BaseRoad.h"
// BaseRoad.cpp - 基底クラスの実装
void BaseRoad::Init() {
    // 既に初期化済みの場合は何もしない
    if (m_isInitialized) {
        return;
    }

    try {
        // モデルの初期化
        m_mesh.Load(GetModelFileName().c_str(), "assets/model/");
        m_terrainMesh.Load(GetModelFileName().c_str(), "assets/model/");

        // 空間分割を初期化
        m_terrainMesh.InitializeSpatialGrid(20.0f);
        m_spatialGridInitialized = true;

        // レンダラ初期化
        m_meshrenderer.Init(m_mesh);

        // シェーダーの初期化
        m_shader.Create("shader/vertexLightingVS.hlsl", "shader/vertexLightingPS.hlsl");

        // 初期化完了フラグをセット
        m_isInitialized = true;

        

        // 既存のコードとの互換性のため、度数法の初期値を弧度法に変換
        // 例: 既存コードで m_Rotation = Vector3{0.0f, 0.0f, 90.0f} のような設定がある場合
        // m_Rotation = MathUtils::DegreesToRadians(m_Rotation);

    }
    catch (...) {
        // 初期化に失敗した場合のクリーンアップ
        m_spatialGridInitialized = false;
        m_isInitialized = false;
        throw; // 例外を再スロー
    }
}

void BaseRoad::Update(float deltatime) {
    // ワールド変換行列を更新
    SRT srt;
    srt.pos = m_Position;
    srt.rot = m_Rotation;
    srt.scale = m_Scale;
    Matrix4x4 worldMatrix = srt.GetMatrix();
    m_terrainMesh.SetWorldMatrix(worldMatrix);
}

void BaseRoad::Draw() {
    SRT srt;
    srt.pos = m_Position;
    srt.rot = m_Rotation;
    srt.scale = m_Scale;
    Matrix4x4 worldmtx = srt.GetMatrix();
    Matrix4x4 viewmtx = srt.CreateViewMatrix();

    Renderer::SetWorldMatrix(&worldmtx);
    m_shader.SetGPU();
    m_meshrenderer.Draw();

    // デバッグ描画
    Color bscolor(1, 1, 1, 0.5f);
    SphereDrawerDraw(m_BoundingSphere.radius, bscolor, m_Position.x, m_Position.y, m_Position.z);
}

void BaseRoad::Dispose() {
    // メッシュリソースの解放
    // 注意: CStaticMeshやCTerrainMeshにDisposeメソッドがある場合
    // m_mesh.Dispose();
    // m_terrainMesh.Dispose();

    // レンダラーの解放
    // m_meshrenderer.Dispose();

    // シェーダーの解放
    // m_shader.Release(); または m_shader.Dispose();

    // 空間分割の初期化フラグをリセット
    m_spatialGridInitialized = false;

    // その他のクリーンアップが必要なリソースがあればここに追加
}


