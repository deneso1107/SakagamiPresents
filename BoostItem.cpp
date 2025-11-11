#include "BoostItem.h"
#include "CarDriveScene.h"
void BoostItem::Init()
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
	m_Position = Vector3{ 200.0f,0.0f,1.0f };
	m_Rotation = Vector3{ 1.7f,2.5f,1.0f };
	m_BoundingSphere.center = m_Position; // 当たり判定の中心を設定
	m_BoundingSphere.radius = 1.0f * m_Scale.x; // 半径はスケールに応じて調整
}
void BoostItem::Update(float deltaTime)
{
	// 非アクティブの場合は更新処理をスキップ
	if (!m_isActive) {
		return;
	}

	// ここに回転アニメーションなどの更新処理を追加
	// 例：アイテムを回転させる
	m_Rotation.y += 0.02f * deltaTime/* / 1000.0f*/;
}
void BoostItem::CollectItem()
{
	if (!m_isCollected && m_isActive)
	{
		m_isCollected = true;
		// 必要に応じて消滅エフェクトやサウンドをここで再生
	}
}
void BoostItem::PlayerBoostGauge(Player* p)
{
	if (m_isActive && !m_isCollected) 
	{
		p->SetBoostGauge(10.0f);
		CollectItem();  // アイテムを取得状態にする
	}
}
void BoostItem::Draw()
{
	// 非アクティブまたは取得済みの場合は描画しない
	if (!m_isActive || m_isCollected) 
	{
		return;
	}
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