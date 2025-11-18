#pragma once

#include	<simplemath.h>
#include	<string>
#include	<vector>
#include	<memory>
#include	"CTexture.h"
#include	"CMesh.h"
#include	"renderer.h"

class CStaticMesh : public CMesh {
public:
	void Load(std::string filename, std::string texturedirectory="");

	// 新規追加メソッド
	Vector3 GetModelSize() const { return m_modelSize; }
	Vector3 GetBoundingBoxMin() const { return m_boundingBoxMin; }
	Vector3 GetBoundingBoxMax() const { return m_boundingBoxMax; }

	const std::vector<MATERIAL>& GetMaterials() {
		return m_materials;
	}

	const std::vector<SUBSET>& GetSubsets() {
		return m_subsets;
	}

	const std::vector<std::string>& GetDiffuseTextureNames() {
		return m_diffusetexturenames;
	}

	std::vector<std::unique_ptr<CTexture>> GetDiffuseTextures() {
		return std::move(m_diffusetextures);
	}
	// 追加
	Vector3 GetBoundingBoxCenter() const 
	{
		return (m_boundingBoxMin + m_boundingBoxMax) * 0.5f;
	}

	// 底面の位置（ローカル座標）
	Vector3 GetBottomPosition() const
	{
		return Vector3(
			(m_boundingBoxMin.x + m_boundingBoxMax.x) * 0.5f,
			m_boundingBoxMin.y,  // 底面のY座標
			(m_boundingBoxMin.z + m_boundingBoxMax.z) * 0.5f
		);
	}
	// 底面のY座標を取得
	float GetBottomY() const { return m_boundingBoxMin.y; }

	// 高さを取得
	float GetHeight() const { return m_boundingBoxMax.y - m_boundingBoxMin.y; }



private:

	Vector3 m_boundingBoxMin;  // バウンディングボックスの最小値
	Vector3 m_boundingBoxMax;  // バウンディングボックスの最大値
	Vector3 m_modelSize;       // モデルのサイズ (max - min)
	std::vector<MATERIAL> m_materials;					// マテリアル情報
	std::vector<std::string> m_diffusetexturenames;		// ディフューズテクスチャ名
	std::vector<SUBSET> m_subsets;						// サブセット情報

	std::vector<std::unique_ptr<CTexture>>	m_diffusetextures;	// ディフューズテクスチャ群
	 Vector3  m_basesize;

	 void CalculateBoundingBox();  // バウンディングボックス計算
};