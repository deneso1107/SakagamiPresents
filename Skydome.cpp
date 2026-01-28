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
		"assets/model/skydome/Dome3.fbx",
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
    // シェーダのセット
    m_shader.SetGPU();

    //上半球を描画
    DrawUpperHemisphere(isboost);

    //下半球を描画
    DrawLowerHemisphere(isboost);

    // 太陽を描画
    m_SunBillboard.Draw();
}

void Skydome::DrawUpperHemisphere(bool isboost)
{
    // 上半球（通常の向き）
    SRT srt;
    srt.pos = m_Position;
    srt.rot = m_Rotation;
    srt.scale = m_Scale;

    Matrix4x4 worldmtx = srt.GetMatrix();
    Renderer::SetWorldMatrix(&worldmtx);

    if (isboost) {
        m_meshnight_renderer.Draw();
    }
    else {
        m_meshrenderer.Draw();
    }
}

void Skydome::DrawLowerHemisphere(bool isboost)
{
    // 下半球（上下反転）
    SRT srt;
    srt.pos = m_Position;

    //X軸で180度回転（上下反転）
    srt.rot = Vector3(DirectX::XM_PI, 0.0f, 0.0f);

    Matrix4x4 worldmtx = srt.GetMatrix();
    Renderer::SetWorldMatrix(&worldmtx);

    if (isboost) {
        m_meshnight_renderer.Draw();
    }
    else {
        m_meshrenderer.Draw();
    }
}

void Skydome::Dispose()
{

}