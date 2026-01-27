#include "system/commontypes.h"
#include "system/CStaticMesh.h"
#include "system/CStaticMeshRenderer.h"
#include "system/CDirectInput.h"
#include "skydome.h"
#include"SpringCamera.h"
#include"IntroCamera.h"
void Skydome::Init()
{
	m_Rotation = Vector3(0.0f, 0.0f, 0.0f);
	m_Scale = Vector3(7.50f, 10.0f, 10.0f);//スカイドームの大きさを変えるなら太陽の位置も変える必要がある

	// モデルの初期化
	m_mesh.Load(
		"assets/model/skydome/skyDome.fbx",
		"assets/model/");

	m_mesh_night.Load(
		"assets/model/skydome_night/night_2.fbx",
		"assets/model/");


	// レンダラ初期化
	m_meshrenderer.Init(m_mesh);
	m_meshnight_renderer.Init(m_mesh_night);

	// シェーダーの初期化
	m_shader.Create(
		"shader/unlitTextureVS.hlsl",		// 頂点シェーダー
		"shader/unlitTexturePS.hlsl");		// ピクセルシェーダー

	m_SunBillboard.Init(
		Vector3(0.0f, 0.0f, 0.0f),	//位置
		20.0f, 20.0f,				//幅、高さ
		L"assets/texture/cow_icon.png"	//テクスチャパス
	);

	m_SunBillboard.m_blendType = BillboardBlendType::Additive;
}

void Skydome::Update(Vector3 camepos)
{
	// 現在のカメラ位置を取得
	//Vector3 camPos = SpringCamera::Instance().GetPosition();
	// スカイドームを常にカメラ中心に置く
	m_Position = camepos;
	m_Position.y -= 600.0f;//下が見えてしまうので下にoffset

	Vector3 sunDir = Vector3(0.0f, 0.15f, 1.0f);
	sunDir.Normalize();
	float sunDistance = 200.0f; // スカイドーム半径より小さめ
	Vector3 sunPos = camepos + sunDir * sunDistance;

	m_SunBillboard.SetPosition(sunPos);
}

void Skydome::Draw(bool isboost)
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
	if (isboost)
	{
		m_meshnight_renderer.Draw();//ブースト時は夜
	}
	else
	{
		m_meshrenderer.Draw();	// 通常時は昼
	}

	m_SunBillboard.Draw();
}

void Skydome::Dispose()
{

}