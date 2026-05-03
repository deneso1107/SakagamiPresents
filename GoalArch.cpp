#include "GoalArch.h"
#include "CarDriveScene.h"
void GoalArch::Init()
{
	m_mesh.Load(
		"assets/model/Arch/Meshy_AI_Golden_Fur_Arch_0503074811_texture.fbx",
		"assets/model/");
	m_meshrenderer.Init(m_mesh);
	m_shader.Create(
		"shader/vertexLightingVS.hlsl",
		"shader/vertexLightingPS.hlsl");

	m_Scale = Vector3(75.0f, 1.0f, 75.0f);
	// 位置・回転はここでは設定しない（外から渡す）
	m_BoundingSphere.center = m_Position;
	m_BoundingSphere.radius = 1.0f * m_Scale.x;

}
void GoalArch::Update(float deltaTime)
{

}
void GoalArch::Draw()
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