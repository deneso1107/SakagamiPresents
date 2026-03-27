#pragma once

#include "system/commontypes.h"
#include "system/CVertexBuffer.h"
#include "system/CIndexBuffer.h"
#include "system/CShader.h"
#include "system/CMaterial.h"
#include "system/CTexture.h"
#include "system/collision.h"

class Field {
	// SRT情報（姿勢情報）
	//ここだけ大文字じゃないと気持ち悪いので命名規則無視
	Vector3	m_Position = Vector3(0.0f, 0.0f, 0.0f);
	Vector3	m_Rotation = Vector3(0.0f, 0.0f, 0.0f);
	Vector3	m_Scale = Vector3(1.0f, 1.0f, 1.0f);

	// 描画の為の情報（メッシュに関わる情報）
	CIndexBuffer				m_indexBuffer;				// インデックスバッファ
	CVertexBuffer<VERTEX_3D>	m_VertexBuffer;				// 頂点バッファ

	// 描画の為の情報（見た目に関わる部分）
	CShader						m_shader;					// シェーダー
	CMaterial					m_material;					// マテリアル
	CTexture					m_texture;					// テクスチャ
	GM31::GE::Collision::BoundingBoxAABB m_fieldSquare;	// フィールドの当たり判定
public:
	void Init();
	void Draw();
	void Dispose();
	Vector3 GetPosition() const { return m_Position; }
	GM31::GE::Collision::BoundingBoxAABB GetFieldCollision()const { return m_fieldSquare; }

	const GM31::GE::Collision::BoundingBoxAABB& GetFieldSquare() const {return m_fieldSquare;}


};