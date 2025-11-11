#include	"Field.h"

void Field::Init()
{
	// 頂点データ
	std::vector<VERTEX_3D>	vertices;

	vertices.resize(4);

	vertices[0].Position = Vector3(-500,  0,  500);
	vertices[1].Position = Vector3( 500,  0,  500);
	vertices[2].Position = Vector3(-500,  0, -500);
	vertices[3].Position = Vector3( 500,  0, -500);

	vertices[0].Diffuse = Color(1, 1, 1, 1);
	vertices[1].Diffuse = Color(1, 1, 1, 1);
	vertices[2].Diffuse = Color(1, 1, 1, 1);
	vertices[3].Diffuse = Color(1, 1, 1, 1);

	vertices[0].TexCoord = Vector2(0, 0);
	vertices[1].TexCoord = Vector2(10, 0);
	vertices[2].TexCoord = Vector2(0, 10);
	vertices[3].TexCoord = Vector2(10, 10);

	// 頂点バッファ生成
	m_VertexBuffer.Create(vertices);

	// インデックスバッファ生成
	std::vector<unsigned int> indices;
	indices.resize(4);

	indices[0] = 0;
	indices[1] = 1;
	indices[2] = 2;
	indices[3] = 3;

	// インデックスバッファ生成
	m_IndexBuffer.Create(indices);

	// シェーダオブジェクト生成
	m_Shader.Create("shader/unlitTextureVS.hlsl","shader/unlitTexturePS.hlsl");

	// マテリアル生成
	MATERIAL	mtrl;
	mtrl.Ambient = Color(0, 0, 0, 0);
	mtrl.Diffuse = Color(1, 1, 1, 1);
	mtrl.Emission = Color(0, 0, 0, 0);
	mtrl.Specular = Color(0, 0, 0, 0);
	mtrl.Shiness = 0;
	mtrl.TextureEnable = TRUE;

	m_Material.Create(mtrl);

	m_FieldSquare.min = Vector3(-500, -0.1f, -500);  // 各軸の最小値
	m_FieldSquare.max = Vector3(500, 0.1f, 500);  // 各軸の最大値
	
	// テクスチャロード
	bool sts = m_Texture.Load("assets\\texture\\field000.jpg");
	assert(sts == true);
}


void Field::Draw()
{
	// SRT情報作成
	SRT srt;
	srt.pos = m_Position;       // 平行移動
	srt.rot = m_Rotation;       // 回転
	srt.scale = m_Scale;        // スケール

	// SRTを行列に変換    
	Matrix4x4 worldmtx = srt.GetMatrix();
	Renderer::SetWorldMatrix(&worldmtx);        // GPUにセット

	// 描画の処理
	ID3D11DeviceContext* devicecontext = Renderer::GetDeviceContext();

	// === プリミティブトポロジーを明示的に設定 ===
	devicecontext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	// シェーダー・バッファ・マテリアル・テクスチャの設定
	m_Shader.SetGPU();
	m_VertexBuffer.SetGPU();
	m_IndexBuffer.SetGPU();
	m_Material.SetGPU();
	m_Texture.SetGPU();

	// 描画実行
	devicecontext->DrawIndexed(
		4,      // 描画するインデックス数（四角形なので4）
		0,      // 最初のインデックスバッファの位置
		0);

	// === 描画後に共通設定をリセット（オプション） ===
	// 他のオブジェクトに影響しないよう、デフォルトの設定に戻す
	 devicecontext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void Field::Dispose()
{

}