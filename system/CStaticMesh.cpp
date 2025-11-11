#include    "commontypes.h"
#include	"CStaticMesh.h"
#include	"AssimpPerse.h"

void CStaticMesh::Load(std::string filename, std::string texturedirectory)
{
	std::vector<GM31::GE::myAssimp::SUBSET> subsets{};					// サブセット情報
	std::vector<std::vector<GM31::GE::myAssimp::VERTEX>> vertices{};	// 頂点データ（メッシュ単位）
	std::vector<std::vector<unsigned int>> indices{};					// インデックスデータ（メッシュ単位）
	std::vector<GM31::GE::myAssimp::MATERIAL> materials{};				// マテリアル
	std::vector<std::unique_ptr<CTexture>> embededtextures{};			// 内蔵テクスチャ群

	// assimpを使用してモデルデータを取得
	GM31::GE::myAssimp::GetModelData(filename, texturedirectory);

	subsets = GM31::GE::myAssimp::GetSubsets();							// サブセット情報取得
	vertices = GM31::GE::myAssimp::GetVertices();						// 頂点データ（メッシュ単位）
	indices = GM31::GE::myAssimp::GetIndices();							// インデックスデータ（メッシュ単位）
	materials = GM31::GE::myAssimp::GetMaterials();						// マテリアル情報取得

	m_diffusetextures = GM31::GE::myAssimp::GetDiffuseTextures();		// ｄｉｆｆｕｓｅテクスチャ情報取得	

	// 頂点データ作成
	int meshidx = 0;

	for (const auto& mv : vertices)
	{
		for (auto& v : mv)
		{
			VERTEX_3D vertex{};
			vertex.Position = Vector3(v.pos.x, v.pos.y, v.pos.z);
			vertex.Normal = Vector3(v.normal.x, v.normal.y, v.normal.z);
			vertex.TexCoord = Vector2(v.texcoord.x, v.texcoord.y);
			vertex.Diffuse = Color(v.color.r, v.color.g, v.color.b, v.color.a);

			vertex.bonecnt = v.bonecnt;
			for (int i = 0; i < 4; i++)
			{
				vertex.BoneIndex[i] = 0;
				vertex.BoneWeight[i] = 0.0f;
				vertex.BoneName[i] = "";
			}

			for (int i = 0; i < v.bonecnt; i++) 
			{
				vertex.BoneIndex[i] = v.BoneIndex[i];
				vertex.BoneWeight[i] = v.BoneWeight[i];
				vertex.BoneName[i] = v.BoneName[i];
			}
			m_vertices.emplace_back(vertex);
		}
	}

	// インデックスデータ作成
	for (const auto& mi : indices)
	{
		for (auto& index : mi)
		{
			m_indices.emplace_back(index);
		}
	}

	// サブセットデータ作成
	for (const auto& sub : subsets)
	{
		SUBSET subset{};
		subset.VertexBase = sub.VertexBase;
		subset.VertexNum = sub.VertexNum;
		subset.IndexBase = sub.IndexBase;
		subset.IndexNum = sub.IndexNum;
		subset.MtrlName = sub.mtrlname;
		subset.MaterialIdx = sub.materialindex;					//	マテリアル配列のインデックス
		m_subsets.emplace_back(subset);
	}

	// マテリアルデータ作成(表示のための)
	for (const auto& m : materials)
	{
		MATERIAL material{};
		material.Ambient = Color(m.Ambient.r, m.Ambient.g, m.Ambient.b, m.Ambient.a);
		material.Diffuse = Color(m.Diffuse.r, m.Diffuse.g, m.Diffuse.b, m.Diffuse.a);
		material.Specular = Color(m.Specular.r, m.Specular.g, m.Specular.b, m.Specular.a);
		material.Emission = Color(m.Emission.r, m.Emission.g, m.Emission.b, m.Emission.a);
		material.Shiness = m.Shiness;
		if (m.diffusetexturename.empty())
		{
			material.TextureEnable = FALSE;
			m_diffusetexturenames.emplace_back("");
		}
		else
		{
			material.TextureEnable = TRUE;
			m_diffusetexturenames.emplace_back(m.diffusetexturename);
		}

		m_materials.emplace_back(material);
	}
	CalculateBoundingBox();
}

void CStaticMesh::CalculateBoundingBox()
{
	if (m_vertices.empty()) {
		m_boundingBoxMin = Vector3(0, 0, 0);
		m_boundingBoxMax = Vector3(0, 0, 0);
		m_modelSize = Vector3(0, 0, 0);
		return;
	}

	// 最初の頂点で初期化
	m_boundingBoxMin = m_vertices[0].Position;
	m_boundingBoxMax = m_vertices[0].Position;

	// 全頂点を走査して最小値・最大値を求める
	for (const auto& vertex : m_vertices) {
		// X座標
		if (vertex.Position.x < m_boundingBoxMin.x) {
			m_boundingBoxMin.x = vertex.Position.x;
		}
		if (vertex.Position.x > m_boundingBoxMax.x) {
			m_boundingBoxMax.x = vertex.Position.x;
		}

		// Y座標
		if (vertex.Position.y < m_boundingBoxMin.y) {
			m_boundingBoxMin.y = vertex.Position.y;
		}
		if (vertex.Position.y > m_boundingBoxMax.y) {
			m_boundingBoxMax.y = vertex.Position.y;
		}

		// Z座標
		if (vertex.Position.z < m_boundingBoxMin.z) {
			m_boundingBoxMin.z = vertex.Position.z;
		}
		if (vertex.Position.z > m_boundingBoxMax.z) {
			m_boundingBoxMax.z = vertex.Position.z;
		}
	}

	// モデルのサイズを計算（最大値 - 最小値）
	m_modelSize = Vector3(
		m_boundingBoxMax.x - m_boundingBoxMin.x,
		m_boundingBoxMax.y - m_boundingBoxMin.y,
		m_boundingBoxMax.z - m_boundingBoxMin.z
	);

	// デバッグ出力
	printf("Model Size: X=%.2f, Y=%.2f, Z=%.2f\n",
		m_modelSize.x, m_modelSize.y, m_modelSize.z);
}