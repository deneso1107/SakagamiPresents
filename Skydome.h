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

	CStaticMeshRenderer	m_meshrenderer;	// メッシュレンダラ
	CStaticMesh			m_mesh;			// メッシュレンダラ
	CShader				m_shader;		// シェーダ

	Billboard			m_SunBillboard;	// ビルボード
public:
	void Init();
	void Update();
	void Draw();
	void Dispose();
};