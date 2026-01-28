#include	"CShader.h"
#include	"dx11helper.h"
#include	"renderer.h"

void CShader::Create(std::string vs, std::string ps, std::string gs)
{

	// 頂点データの定義
//	D3D11_INPUT_ELEMENT_DESC layout[] =
//	{
//		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,		0,	D3D11_APPEND_ALIGNED_ELEMENT,	D3D11_INPUT_PER_VERTEX_DATA, 0 },
//		{ "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT,		0,	D3D11_APPEND_ALIGNED_ELEMENT,	D3D11_INPUT_PER_VERTEX_DATA, 0 },
//		{ "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT,	0,	D3D11_APPEND_ALIGNED_ELEMENT,	D3D11_INPUT_PER_VERTEX_DATA, 0 },
//		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,			0,	D3D11_APPEND_ALIGNED_ELEMENT,   D3D11_INPUT_PER_VERTEX_DATA, 0 }
//	};

	// ワンスキン対応　20231227追加
	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION",	0, DXGI_FORMAT_R32G32B32_FLOAT,		0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL",		0, DXGI_FORMAT_R32G32B32_FLOAT,		0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR",		0, DXGI_FORMAT_R32G32B32A32_FLOAT,	0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD",	0, DXGI_FORMAT_R32G32_FLOAT,		0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "BONEINDEX",	0, DXGI_FORMAT_R32G32B32A32_SINT,	0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "BONEWEIGHT",	0, DXGI_FORMAT_R32G32B32A32_FLOAT,	0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};

	unsigned int numElements = ARRAYSIZE(layout);

	ID3D11Device* device;
	device = Renderer::GetDevice();

	// 頂点シェーダーオブジェクトを生成、同時に頂点レイアウトも生成
	bool sts = CreateVertexShader(device,
			vs.c_str(),
			"main",
			"vs_5_0",
			layout,
			numElements,
			&m_pVertexShader,
			&m_pVertexLayout);
	if (!sts) {
		MessageBox(nullptr, "CreateVertexShader error", "error", MB_OK);
		return;
	}

	// ピクセルシェーダーを生成
	sts = CreatePixelShader(			// ピクセルシェーダーオブジェクトを生成
		device,							// デバイスオブジェクト
		ps.c_str(),
		"main",
		"ps_5_0",
		&m_pPixelShader);
	if (!sts) {
		MessageBox(nullptr, "CreatePixelShader error", "error", MB_OK);
		return;
	}

	if (gs != "") {
		// ピクセルシェーダーを生成
		sts = CreateGeometryShader(			// ピクセルシェーダーオブジェクトを生成
			device,							// デバイスオブジェクト
			gs.c_str(),
			"main",
			"gs_5_0",
			&m_pGeometryShader);
		if (!sts) {	
			MessageBox(nullptr, "CreateGeometryShader error", "error", MB_OK);
			return;
		}
	}

	return;
}

bool CShader::CreateParticleBillboard(const char* vsPath, const char* psPath)
{
	HRESULT hr;

	// 頂点シェーダーのコンパイル
	ID3DBlob* pVSBlob = nullptr;
	hr = CompileShaderFromFile(vsPath, "main", "vs_5_0", &pVSBlob);
	if (FAILED(hr))
	{
		OutputDebugStringA("頂点シェーダーのコンパイル失敗\n");
		return false;
	}

	// 頂点シェーダーの作成
	hr = Renderer::GetDevice()->CreateVertexShader(
		pVSBlob->GetBufferPointer(),
		pVSBlob->GetBufferSize(),
		nullptr,
		m_pVertexShader.GetAddressOf()
	);

	if (FAILED(hr))
	{
		pVSBlob->Release();
		OutputDebugStringA("頂点シェーダーの作成失敗\n");
		return false;
	}

	//インプットレイアウトの定義（インスタンシング対応）
	D3D11_INPUT_ELEMENT_DESC layout[] = {
		// 頂点バッファ（スロット0）
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },

		// インスタンスバッファ（スロット1）
		{ "TEXCOORD", 1, DXGI_FORMAT_R32G32B32_FLOAT, 1, 0, D3D11_INPUT_PER_INSTANCE_DATA, 1 },   // worldPosition (12 bytes)
		{ "TEXCOORD", 2, DXGI_FORMAT_R32_FLOAT, 1, 12, D3D11_INPUT_PER_INSTANCE_DATA, 1 },        // size (4 bytes)
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 16, D3D11_INPUT_PER_INSTANCE_DATA, 1 }   // color (16 bytes)
	};

	UINT numElements = ARRAYSIZE(layout);

	// インプットレイアウトの作成
	hr = Renderer::GetDevice()->CreateInputLayout(
		layout,
		numElements,
		pVSBlob->GetBufferPointer(),
		pVSBlob->GetBufferSize(),
		m_pVertexLayout.GetAddressOf()
	);

	// pVSBlobを解放（ここで1回だけ）
	pVSBlob->Release();

	if (FAILED(hr))
	{
		char errorMsg[256];
		sprintf_s(errorMsg, "インプットレイアウト作成失敗: HRESULT = 0x%08X\n", hr);
		OutputDebugStringA(errorMsg);
		return false;
	}

	// ピクセルシェーダーのコンパイル
	ID3DBlob* pPSBlob = nullptr;
	hr = CompileShaderFromFile(psPath, "main", "ps_5_0", &pPSBlob);
	if (FAILED(hr))
	{
		OutputDebugStringA("ピクセルシェーダーのコンパイル失敗\n");
		return false;
	}

	// ピクセルシェーダーの作成
	hr = Renderer::GetDevice()->CreatePixelShader(
		pPSBlob->GetBufferPointer(),
		pPSBlob->GetBufferSize(),
		nullptr,
		m_pPixelShader.GetAddressOf()
	);

	pPSBlob->Release();

	if (FAILED(hr))
	{
		OutputDebugStringA("ピクセルシェーダーの作成失敗\n");
		return false;
	}

	OutputDebugStringA("ParticleBillboard シェーダー作成成功\n");
	return true;
}

bool CShader::CreateParticle(std::string vs, std::string ps)
{
	// パーティクル用のInputLayout定義
	D3D11_INPUT_ELEMENT_DESC layout[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,  D3D11_INPUT_PER_VERTEX_DATA,   0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, 12, D3D11_INPUT_PER_VERTEX_DATA,   0 },
		{ "WORLD", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 0,  D3D11_INPUT_PER_INSTANCE_DATA, 1 },
		{ "WORLD", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 16, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
		{ "WORLD", 2, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 32, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
		{ "WORLD", 3, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 48, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 64, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
	};
	unsigned int numElements = ARRAYSIZE(layout);

	ID3D11Device* device = Renderer::GetDevice();

	// 頂点シェーダー作成
	bool sts = CreateVertexShader(device, vs.c_str(), "main", "vs_5_0",
		layout, numElements, &m_pVertexShader, &m_pVertexLayout);
	if (!sts) {
		MessageBox(nullptr, "CreateVertexShader error (Particle)", "error", MB_OK);
		return false;
	}

	// ピクセルシェーダー作成
	sts = CreatePixelShader(device, ps.c_str(), "main", "ps_5_0", &m_pPixelShader);
	if (!sts) {
		MessageBox(nullptr, "CreatePixelShader error (Particle)", "error", MB_OK);
		return false;
	}

	return true;
}

// CShaderクラスに2D用メソッドを追加
void CShader::Create2D(std::string vs, std::string ps,std::string gs)
{
	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};

	unsigned int numElements = ARRAYSIZE(layout);
	// 残りの処理は同じ

	ID3D11Device* device;
	device = Renderer::GetDevice();

	// 頂点シェーダーオブジェクトを生成、同時に頂点レイアウトも生成
	bool sts = CreateVertexShader(device,
		vs.c_str(),
		"main",
		"vs_5_0",
		layout,
		numElements,
		&m_pVertexShader,
		&m_pVertexLayout);
	if (!sts) {
		MessageBox(nullptr, "CreateVertexShader error", "error", MB_OK);
		return;
	}

	// ピクセルシェーダーを生成
	sts = CreatePixelShader(			// ピクセルシェーダーオブジェクトを生成
		device,							// デバイスオブジェクト
		ps.c_str(),
		"main",
		"ps_5_0",
		&m_pPixelShader);
	if (!sts) {
		MessageBox(nullptr, "CreatePixelShader error", "error", MB_OK);
		return;
	}

	if (gs != "") {
		// ピクセルシェーダーを生成
		sts = CreateGeometryShader(			// ピクセルシェーダーオブジェクトを生成
			device,							// デバイスオブジェクト
			gs.c_str(),
			"main",
			"gs_5_0",
			&m_pGeometryShader);
		if (!sts) {
			MessageBox(nullptr, "CreateGeometryShader error", "error", MB_OK);
			return;
		}
	}

	return;
}

void CShader::SetGPU() {

	ID3D11DeviceContext* devicecontext;

	devicecontext = Renderer::GetDeviceContext();

	devicecontext->VSSetShader(m_pVertexShader.Get(), nullptr, 0);		// 頂点シェーダーをセット
	devicecontext->PSSetShader(m_pPixelShader.Get(), nullptr, 0);		// ピクセルシェーダーをセット
	devicecontext->GSSetShader(m_pGeometryShader.Get(), nullptr, 0);	// ジオメトリシェーダーをセット
	devicecontext->IASetInputLayout(m_pVertexLayout.Get());				// 頂点レイアウトセット
}

