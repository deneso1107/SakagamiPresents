#include "Goal.h"
void Goal ::Init()
{
    // モデルの初期化
    m_mesh.Load(
        "assets/model/car001.x",                // モデル名
        "assets/model/");                       // テクスチャのパス
    // レンダラ初期化
    m_meshrenderer.Init(m_mesh);

    // シェーダーの初期化
    m_shader.Create(
        "shader/vertexLightingVS.hlsl",         // 頂点シェーダー
        "shader/vertexLightingPS.hlsl");        // ピクセルシェーダー

    DebugUI::RedistDebugFunction([this]()
        {
            DebugObjectSRT();
        });
    // デバッグ用の空間分割情報表示を追加
   /* DebugUI::RedistDebugFunction([this]()
        {
            DebugSpatialGrid();
        });*/
    m_Position = Vector3{ 350.0f,0.0f,1.0f };
    m_Rotation = Vector3{ 1.7f,2.5f,1.0f };
    m_BoundingSphere.center = m_Position; // 当たり判定の中心を設定
	m_BoundingSphere.radius = 1.0f * m_Scale.x; // 半径はスケールに応じて調整
}

void Goal::Update(float deltaTime)
{
    // // デバッグ用のSRT情報表示
    // DebugObjectSRT();
    // // 当たり判定の更新
    // m_BoundingSphere.center = m_Position;
    // m_BoundingSphere.radius = 1.0f * m_Scale.x; // 半径はスケールに応じて調整
}
void Goal::Dispose()
{
    //// シェーダーの解放
    //m_shader.Dispose();
    //// メッシュレンダラーの解放
    //m_meshrenderer.Dispose();
    //// メッシュの解放
    //m_mesh.Dispose();
}
void Goal::Draw()
{
    // SRT情報作成
    SRT srt;
    srt.pos = m_Position;			// 位置
    srt.rot = m_Rotation;			// 姿勢
    srt.scale = m_Scale;			// 拡縮
    Matrix4x4 worldmtx;
    worldmtx = srt.GetMatrix();
    Matrix4x4 viewmtx;
    viewmtx = srt.CreateViewMatrix();
    Renderer::SetWorldMatrix(&worldmtx);		// GPUにセット
    m_shader.SetGPU();
    m_meshrenderer.Draw();
}
