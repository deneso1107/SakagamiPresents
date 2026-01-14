#pragma once

#include "system/commontypes.h"
#include "system/CShader.h"
#include "system/CStaticMesh.h"
#include "system/CStaticMeshRenderer.h"
#include"Billboard.h"

class Skydome {

	// SRT情報（姿勢情報）
	Vector3	m_Position = Vector3(0.0f, 0.0f, 0.0f);
	Vector3	m_Rotation = Vector3(0.0f, 0.0f, 0.0f);
	Vector3	m_Scale = Vector3(1.0f, 1.0f, 1.0f);

	CStaticMeshRenderer	m_meshrenderer;	// メッシュレンダラ(昼)
	CStaticMesh			m_mesh;			// メッシュ（昼）
	CStaticMeshRenderer	m_meshnight_renderer;	// メッシュレンダラ(夜)
	CStaticMesh			m_mesh_night;	// メッシュレンダラ(夜)
	CShader				m_shader;		// シェーダ(夜)

	Billboard			m_SunBillboard;	// ビルボード
public:
	void Init();
	void Update();
	void Draw(bool);
	void Dispose();
};