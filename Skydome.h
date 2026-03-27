#pragma once

#include "system/commontypes.h"
#include "system/CShader.h"
#include "system/CStaticMesh.h"
#include "system/CStaticMeshRenderer.h"
#include"Billboard.h"

class Skydome {

	// SRT情報（姿勢情報）ここ大文字にしてるのは、カメラと同じように、外部からアクセスする可能性があるから
	Vector3	m_Position = Vector3(0.0f, 0.0f, 0.0f);
	Vector3	m_Rotation = Vector3(0.0f, 0.0f, 0.0f);
	Vector3	m_Scale = Vector3(1.0f, 1.0f, 1.0f);

	CStaticMeshRenderer	m_meshrenderer;	// メッシュレンダラ(昼)
	CStaticMesh			m_mesh;			// メッシュ（昼）
	CStaticMeshRenderer	m_meshnight_renderer;	// メッシュレンダラ(夜)
	CStaticMesh			m_mesh_night;	// メッシュレンダラ(夜)
	CShader				m_shader;		// シェーダ(夜)

	Billboard			m_sunBillboard;	// ビルボード

	//上半球と下半球を描画する内部関数
	void DrawUpperHemisphere(bool isboost);
	void DrawLowerHemisphere(bool isboost);
public:
	void Init();
	void Update(Vector3);
	void Draw(bool);
	void Dispose();
};