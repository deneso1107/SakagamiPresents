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


	const Vector3 GetModelBaseSize() { return m_basesize; }



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