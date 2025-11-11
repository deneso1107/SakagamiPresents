#include "system/commontypes.h"
#include "system/CStaticMesh.h"
#include "system/CStaticMeshRenderer.h"
#include "system/CDirectInput.h"
#include "skydome.h"

void Skydome::Init()
{
	m_Position = Vector3(0.0f, 0.0f, 0.0f);
	m_Rotation = Vector3(0.0f, 0.0f, 0.0f);
	m_Scale = Vector3(1.0f, 1.0f, 1.0f);

	// モデルの初期化
	m_mesh.Load(
		"assets/model/skydome.x",
		"assets/model/");

	// レンダラ初期化
	m_meshrenderer.Init(m_mesh);

	// シェーダーの初期化
	m_shader.Create(
		"shader/unlitTextureVS.hlsl",		// 頂点シェーダー
		"shader/unlitTexturePS.hlsl");		// ピクセルシェーダー

}

void Skydome::Update()
{
}

void Skydome::Draw()
{
	SRT srt;

	// SRT情報作成
	srt.pos = m_Position;			// 位置
	srt.rot = m_Rotation;			// 姿勢
	srt.scale = m_Scale;			// 拡縮

	Matrix4x4 worldmtx;

	worldmtx = srt.GetMatrix();

	Renderer::SetWorldMatrix(&worldmtx);		// GPUにセット
	m_shader.SetGPU();		// シェーダのセット
	m_meshrenderer.Draw();	// メッシュレンダラの描画
}

void Skydome::Dispose()
{

}