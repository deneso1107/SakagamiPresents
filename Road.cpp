#include "Road.h"

void Road::Init()
{
    // モデルの初期化
    m_mesh.Load(
        "assets/model/Road/road.fbx",                // モデル名
        "assets/model/");                       // テクスチャのパス

    // 同じモデルを当たり判定用にもロード
    m_terrainMesh.Load(
        "assets/model/Road/road.fbx",                // 同じモデル
        "assets/model/");                       // 同じテクスチャパス

    // 空間分割を初期化（当たり判定の高速化）
    m_terrainMesh.InitializeSpatialGrid(20.0f); // セルサイズは地形に応じて調整
    m_spatialGridInitialized = true;

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

    m_Position = Vector3{ 200.0f,1.0f,1.0f };
    m_Rotation = Vector3{ 0.0f,0.0f,3.0f };
    m_Scale = Vector3{ 18.0f,1.0f,18.0f };
}

void Road::Update(float deltatime)
{
	// ここでは特に更新する内容はないが、将来的に必要な場合はここに処理を追加する
       // ワールド変換行列を更新（重要！）
    SRT srt;
    srt.pos = m_Position;
    srt.rot = m_Rotation;
    srt.scale = m_Scale;
    Matrix4x4 worldMatrix = srt.GetMatrix();

    // 地形メッシュにワールド変換行列を設定
    m_terrainMesh.SetWorldMatrix(worldMatrix);
}
void Road::Draw()
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
	// デバッグ用のグローバル変数に値をセット
	Color bscolor(1, 1, 1, 0.5f);
	SphereDrawerDraw(m_BoundingSphere.radius, bscolor, m_Position.x, m_Position.y, m_Position.z); // 球体を描画
}
void Road::Dispose()
{

}
