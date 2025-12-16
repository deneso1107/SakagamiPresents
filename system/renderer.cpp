/**
 * @file Renderer.cpp
 * @brief DirectX を用いたレンダリング処理を実装する Renderer クラスの定義（実装部）。
 *
 * このファイルでは、Direct3D デバイス、スワップチェーン、レンダーターゲット、定数バッファなどの
 * 初期化と解放、描画の開始・終了処理、各種レンダリング状態（深度、ブレンド、マトリックスなど）の設定を行います。
 */

#include <stdexcept>
#include "renderer.h"
#include "../Application.h"

//------------------------------------------------------------------------------
// スタティックメンバ変数の初期化
//------------------------------------------------------------------------------

D3D_FEATURE_LEVEL       Renderer::m_FeatureLevel = D3D_FEATURE_LEVEL_11_0;

ComPtr<ID3D11Device> Renderer::m_Device;
ComPtr<ID3D11DeviceContext> Renderer::m_DeviceContext;
ComPtr<IDXGISwapChain> Renderer::m_SwapChain;
ComPtr<ID3D11RenderTargetView> Renderer::m_RenderTargetView;
ComPtr<ID3D11DepthStencilView> Renderer::m_DepthStencilView;

ComPtr<ID3D11Buffer> Renderer::m_WorldBuffer;
ComPtr<ID3D11Buffer> Renderer::m_ViewBuffer;
ComPtr<ID3D11Buffer> Renderer::m_ProjectionBuffer;
ComPtr<ID3D11Buffer> Renderer::m_MaterialBuffer;
ComPtr<ID3D11Buffer> Renderer::m_LightBuffer;

ComPtr<ID3D11DepthStencilState> Renderer::m_DepthStateEnable;
ComPtr<ID3D11DepthStencilState> Renderer::m_DepthStateDisable;

ComPtr<ID3D11BlendState> Renderer::m_BlendState[MAX_BLENDSTATE];
ComPtr<ID3D11BlendState> Renderer::m_BlendStateATC;

Matrix4x4 Renderer::m_CurrentWorld = Matrix4x4::Identity;
Matrix4x4 Renderer::m_CurrentView = Matrix4x4::Identity;
Matrix4x4 Renderer::m_CurrentProjection = Matrix4x4::Identity;

//------------------------------------------------------------------------------
// Renderer クラスの各関数の実装
//------------------------------------------------------------------------------

/**
 * @brief Renderer の初期化処理を行います。
 *
 * Direct3D デバイスとスワップチェーンの作成、レンダーターゲットビュー、デプスステンシルビュー、
 * ビューポート、ラスタライザ、ブレンドステート、深度ステンシルステート、サンプラーステート、
 * 定数バッファの生成、初期ライトおよびマテリアルの設定などを実施します。
 */
void Renderer::Init()
{
    HRESULT hr = S_OK;

    DXGI_SWAP_CHAIN_DESC swapChainDesc{};
    swapChainDesc.BufferCount = 1;
    swapChainDesc.BufferDesc.Width = Application::GetWidth();
    swapChainDesc.BufferDesc.Height = Application::GetHeight();
    swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
    swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.OutputWindow = Application::GetWindow();
    swapChainDesc.SampleDesc.Count = 1;
    swapChainDesc.SampleDesc.Quality = 0;
    swapChainDesc.Windowed = TRUE;

    hr = D3D11CreateDeviceAndSwapChain(
        nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, D3D11_CREATE_DEVICE_BGRA_SUPPORT,
        nullptr, 0, D3D11_SDK_VERSION, &swapChainDesc,
        m_SwapChain.GetAddressOf(),
        m_Device.GetAddressOf(),
        &m_FeatureLevel,
        m_DeviceContext.GetAddressOf());

    ComPtr<ID3D11Texture2D> renderTarget;
    hr = m_SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(renderTarget.GetAddressOf()));
    if (SUCCEEDED(hr) && renderTarget) {
        m_Device->CreateRenderTargetView(renderTarget.Get(), nullptr, m_RenderTargetView.GetAddressOf());
    }
    else {
        throw std::runtime_error("Failed to retrieve render target buffer.");
    }

    ComPtr<ID3D11Texture2D> depthStencil;
    D3D11_TEXTURE2D_DESC textureDesc{};
    textureDesc.Width = swapChainDesc.BufferDesc.Width;
    textureDesc.Height = swapChainDesc.BufferDesc.Height;
    textureDesc.MipLevels = 1;
    textureDesc.ArraySize = 1;
    textureDesc.Format = DXGI_FORMAT_D16_UNORM;
    textureDesc.SampleDesc = swapChainDesc.SampleDesc;
    textureDesc.Usage = D3D11_USAGE_DEFAULT;
    textureDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    hr = m_Device->CreateTexture2D(&textureDesc, nullptr, depthStencil.GetAddressOf());
    if (FAILED(hr)) {
        throw std::runtime_error("Failed to create depthStencil.");
    }

    D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc{};
    depthStencilViewDesc.Format = textureDesc.Format;
    depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    hr = m_Device->CreateDepthStencilView(depthStencil.Get(), &depthStencilViewDesc, m_DepthStencilView.GetAddressOf());
    if (FAILED(hr)) {
        throw std::runtime_error("Failed to create depthStencilView.");
    }

    m_DeviceContext->OMSetRenderTargets(1, m_RenderTargetView.GetAddressOf(), m_DepthStencilView.Get());

    D3D11_VIEWPORT viewport;
    viewport.Width = static_cast<FLOAT>(Application::GetWidth());
    viewport.Height = static_cast<FLOAT>(Application::GetHeight());
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;
    viewport.TopLeftX = 0;
    viewport.TopLeftY = 0;
    m_DeviceContext->RSSetViewports(1, &viewport);


    // --- ラスタライザステート設定 ---
    D3D11_RASTERIZER_DESC rasterizerDesc{};
    rasterizerDesc.FillMode = D3D11_FILL_SOLID;
    rasterizerDesc.CullMode = D3D11_CULL_BACK;
    rasterizerDesc.DepthClipEnable = TRUE;

    ComPtr<ID3D11RasterizerState> rs;
    m_Device->CreateRasterizerState(&rasterizerDesc, rs.GetAddressOf());
    m_DeviceContext->RSSetState(rs.Get());

    // --- ブレンドステートの生成 ---
    D3D11_BLEND_DESC BlendDesc{};
    BlendDesc.AlphaToCoverageEnable = FALSE;
    BlendDesc.IndependentBlendEnable = TRUE;
    BlendDesc.RenderTarget[0].BlendEnable = FALSE;
    BlendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
    BlendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
    BlendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
    BlendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
    BlendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
    BlendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
    BlendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

    m_Device->CreateBlendState(&BlendDesc, m_BlendState[0].GetAddressOf());
    BlendDesc.RenderTarget[0].BlendEnable = TRUE;
    m_Device->CreateBlendState(&BlendDesc, m_BlendState[1].GetAddressOf());
    m_Device->CreateBlendState(&BlendDesc, m_BlendStateATC.GetAddressOf());

    BlendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
    m_Device->CreateBlendState(&BlendDesc, m_BlendState[2].GetAddressOf());

    BlendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_REV_SUBTRACT;
    m_Device->CreateBlendState(&BlendDesc, m_BlendState[3].GetAddressOf());

    SetBlendState(BS_ALPHABLEND);

    // --- 深度ステンシルステートの設定 ---
    D3D11_DEPTH_STENCIL_DESC depthStencilDesc{};
    depthStencilDesc.DepthEnable = TRUE;
    depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
    depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
    depthStencilDesc.StencilEnable = FALSE;

    m_Device->CreateDepthStencilState(&depthStencilDesc, m_DepthStateEnable.GetAddressOf());

    depthStencilDesc.DepthEnable = FALSE;
    depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
    m_Device->CreateDepthStencilState(&depthStencilDesc, m_DepthStateDisable.GetAddressOf());

    m_DeviceContext->OMSetDepthStencilState(m_DepthStateEnable.Get(), 0);

    // --- サンプラーステート設定 ---
    D3D11_SAMPLER_DESC samplerDesc{};
    samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
    samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
//    samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
//    samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
//    samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
    samplerDesc.MaxAnisotropy = 4;
    samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

    ComPtr<ID3D11SamplerState> samplerState;
    m_Device->CreateSamplerState(&samplerDesc, samplerState.GetAddressOf());
    m_DeviceContext->PSSetSamplers(0, 1, samplerState.GetAddressOf());

    // --- 定数バッファ生成 ---
    D3D11_BUFFER_DESC bufferDesc{};
    bufferDesc.ByteWidth = sizeof(Matrix4x4);
    bufferDesc.Usage = D3D11_USAGE_DEFAULT;
    bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    bufferDesc.CPUAccessFlags = 0;
    bufferDesc.MiscFlags = 0;
    bufferDesc.StructureByteStride = sizeof(float);

    m_Device->CreateBuffer(&bufferDesc, nullptr, m_WorldBuffer.GetAddressOf());
    m_DeviceContext->VSSetConstantBuffers(0, 1, m_WorldBuffer.GetAddressOf());

    m_Device->CreateBuffer(&bufferDesc, nullptr, m_ViewBuffer.GetAddressOf());
    m_DeviceContext->VSSetConstantBuffers(1, 1, m_ViewBuffer.GetAddressOf());

    m_Device->CreateBuffer(&bufferDesc, nullptr, m_ProjectionBuffer.GetAddressOf());
    m_DeviceContext->VSSetConstantBuffers(2, 1, m_ProjectionBuffer.GetAddressOf());

    bufferDesc.ByteWidth = sizeof(MATERIAL);
    m_Device->CreateBuffer(&bufferDesc, nullptr, m_MaterialBuffer.GetAddressOf());

    bufferDesc.ByteWidth = sizeof(LIGHT);
    m_Device->CreateBuffer(&bufferDesc, nullptr, m_LightBuffer.GetAddressOf());

    // --- ライト初期化 ---
    LIGHT light{};
    light.Enable = true;
    light.Direction = Vector4(0.5f, -1.0f, 0.8f, 0.0f);
    light.Direction.Normalize();
    light.Ambient = Color(0.2f, 0.2f, 0.2f, 1.0f);
    light.Diffuse = Color(1.5f, 1.5f, 1.5f, 1.0f);
    SetLight(light);

    // --- マテリアル初期化 ---
    MATERIAL material{};
    material.Diffuse = Color(1.0f, 1.0f, 1.0f, 1.0f);
    material.Ambient = Color(1.0f, 1.0f, 1.0f, 1.0f);
    SetMaterial(material);

    m_DeviceContext->VSSetConstantBuffers(3, 1, m_MaterialBuffer.GetAddressOf());
    m_DeviceContext->PSSetConstantBuffers(3, 1, m_MaterialBuffer.GetAddressOf());
    
    m_DeviceContext->VSSetConstantBuffers(4, 1, m_LightBuffer.GetAddressOf());
    m_DeviceContext->PSSetConstantBuffers(4, 1, m_LightBuffer.GetAddressOf());

}

/// @brief シャドウマップ用のリソースを初期化します。

void Renderer::InitShadowMap(int shadowSize)
{
	HRESULT hr;
    // シャドウマップテクスチャの作成
	D3D11_TEXTURE2D_DESC texDesc={};
	texDesc.Width = shadowSize;
	texDesc.Height = shadowSize;
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 1;
	texDesc.Format = DXGI_FORMAT_R32_TYPELESS;
	texDesc.SampleDesc.Count = 1;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
	hr = m_Device->CreateTexture2D(&texDesc, nullptr, m_ShadowMapTexture.GetAddressOf());
    if (FAILED(hr))
    {
        MessageBox(nullptr, "Failed to create shadow map texture", "Error", MB_OK);
        return;
    }


    // Depth Stencil Viewの作成
    D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
	dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
	dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Texture2D.MipSlice = 0;
    hr = m_Device->CreateDepthStencilView(m_ShadowMapTexture.Get(), &dsvDesc, m_ShadowMapDSV.GetAddressOf());
    if (FAILED(hr))
    {
        MessageBox(nullptr, "Failed to create shadow map DSV", "Error", MB_OK);
        return;
	}
    // Shader Resource Viewの作成
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = DXGI_FORMAT_R32_FLOAT/*DXGI_FORMAT_R24_UNORM_X8_TYPELESS*/;//何が違う?
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
    srvDesc.Texture2D.MipLevels = 1;
    hr = m_Device->CreateShaderResourceView(m_ShadowMapTexture.Get(), &srvDesc, m_ShadowMapSRV.GetAddressOf());
    if (FAILED(hr))
    {
        MessageBox(nullptr, "Failed to create shadow map SRV", "Error", MB_OK);
        return;
	}

    //比較サンプラーの作成
	D3D11_SAMPLER_DESC samplerDesc = {};
	samplerDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;
    samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
	samplerDesc.BorderColor[0] = 1.0f;
	samplerDesc.BorderColor[1] = 1.0f;
	samplerDesc.BorderColor[2] = 1.0f;
	samplerDesc.BorderColor[3] = 1.0f;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_LESS_EQUAL;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
	samplerDesc.MinLOD = 0;
    hr = m_Device->CreateSamplerState(&samplerDesc, m_ShadowSampler.GetAddressOf());
    if (FAILED(hr))
    {
        MessageBox(nullptr, "Failed to create shadow map sampler", "Error", MB_OK);
        return;
	}

	//ライト行列用の定数バッファ作成
	D3D11_BUFFER_DESC bufferDesc = {};
	bufferDesc.ByteWidth = sizeof(DirectX::XMMATRIX)*2;//View*projectionほんまに？
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	//bufferDesc.CPUAccessFlags = 0;
	//bufferDesc.MiscFlags = 0;
 //   bufferDesc.StructureByteStride = sizeof(float);
    hr = m_Device->CreateBuffer(&bufferDesc, nullptr, m_LightMatrixBuffer.GetAddressOf());
    if (FAILED(hr))
    {
        MessageBox(nullptr, "Failed to create light matrix buffer", "Error", MB_OK);
        return;
	}

    m_ShadowMapEnabled = true;
}

/**
 * @brief 使用していたリソースを全て解放します。
 *
 * @details
 * Direct3Dのリソースは明示的に解放しないとメモリリークが発生するため、
 * ComPtr::Reset()で安全にリソースを開放しています。
 */
void Renderer::Uninit()
{
    for (auto& bs : m_BlendState) {
        bs.Reset();
    }
    m_BlendStateATC.Reset();
    m_DepthStateEnable.Reset();
    m_DepthStateDisable.Reset();
    m_WorldBuffer.Reset();
    m_ViewBuffer.Reset();
    m_ProjectionBuffer.Reset();
    m_LightBuffer.Reset();
    m_MaterialBuffer.Reset();
    m_RenderTargetView.Reset();
    m_SwapChain.Reset();
    m_DeviceContext.Reset();
    m_Device.Reset();
}

void Renderer::UninitShadowMap()
{
    m_ShadowMapTexture.Reset();
    m_ShadowMapDSV.Reset();
    m_ShadowMapSRV.Reset();
    m_ShadowSampler.Reset();
    m_LightMatrixBuffer.Reset();
	m_ShadowMapEnabled = false;
}

/**
 * @brief 1フレームの描画を開始します。
 *
 * @details
 * - 画面を指定色（青色）でクリア
 * - 深度バッファも初期化
 *
 * 毎フレーム必ず呼び出して、前のフレームの残像を消します。
 */
void Renderer::Begin()
{

    float clearColor[4] = { 0.0f, 0.0f, 1.0f, 1.0f };
    m_DeviceContext->ClearRenderTargetView(m_RenderTargetView.Get(), clearColor);
    m_DeviceContext->ClearDepthStencilView(m_DepthStencilView.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);

    // バックバッファをクリア
    //float clearColor[4] = { 0.0f, 0.0f, 1.0f, 1.0f };
    //m_DeviceContext->ClearRenderTargetView(m_RenderTargetView.Get(), clearColor);

    //// 深度バッファをクリア
    //m_DeviceContext->ClearDepthStencilView(
    //    m_DepthStencilView.Get(),
    //    D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,
    //    1.0f, 0
    //);
}
/**
 * @brief シャドウマップの描画を開始します。（第1パス）
 *
 * @details
 * - レンダーターゲットをシャドウマップ用に切り替え
 * - 深度バッファをクリア
 * - ビューポートをシャドウマップサイズに設定
 */
void Renderer::BeginShadowPass()
{
    if (!m_ShadowMapEnabled)
    {
        OutputDebugStringA("BeginShadowPass: Shadow map is DISABLED\n");
        return;
    }

    OutputDebugStringA("\n===== BeginShadowPass =====\n");

    // DSVがnullでないか確認
    if (!m_ShadowMapDSV)
    {
        OutputDebugStringA("ERROR: m_ShadowMapDSV is NULL!\n");
        return;
    }

    OutputDebugStringA("m_ShadowMapDSV is valid\n");

    m_CurrentRenderMode = RenderMode::SHADOW_MAP;

    // 現在のレンダーターゲットとビューポートを保存
    UINT numViewports = 1;
    m_DeviceContext->OMGetRenderTargets(1, m_SavedRenderTarget.GetAddressOf(),
        m_SavedDepthStencil.GetAddressOf());
    m_DeviceContext->RSGetViewports(&numViewports, &m_SavedViewport);

    char msg[256];
    sprintf_s(msg, "Saved viewport: %.0fx%.0f\n", m_SavedViewport.Width, m_SavedViewport.Height);
    OutputDebugStringA(msg);

    // レンダーターゲットをクリア（重要！）
    ID3D11RenderTargetView* nullRTV[8] = { nullptr };
    m_DeviceContext->OMSetRenderTargets(8, nullRTV, nullptr);
    OutputDebugStringA("Cleared all render targets\n");

    // シャドウマップDSVをセット
    m_DeviceContext->OMSetRenderTargets(0, nullptr, m_ShadowMapDSV.Get());
    OutputDebugStringA("Set shadow map DSV\n");

    // 深度バッファをクリア
    m_DeviceContext->ClearDepthStencilView(m_ShadowMapDSV.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);
    OutputDebugStringA("Cleared depth to 1.0\n");

    // 深度ステートを強制的に有効
    if (m_DepthStateEnable)
    {
        m_DeviceContext->OMSetDepthStencilState(m_DepthStateEnable.Get(), 0);
        OutputDebugStringA("Depth state enabled\n");
    }
    else
    {
        OutputDebugStringA("WARNING: m_DepthStateEnable is NULL!\n");
    }

    // ラスタライザステート（カリングオフ）
   /* if (m_RasterStateCullOff)
    {
        m_DeviceContext->RSSetState(m_RasterStateCullOff.Get());
        OutputDebugStringA("Rasterizer state set (cull off)\n");
    }*/

    // ビューポート設定
    D3D11_VIEWPORT viewport = {};
    viewport.TopLeftX = 0;
    viewport.TopLeftY = 0;
    viewport.Width = 2048.0f;
    viewport.Height = 2048.0f;
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;
    m_DeviceContext->RSSetViewports(1, &viewport);

    sprintf_s(msg, "Set viewport: %.0fx%.0f\n", viewport.Width, viewport.Height);
    OutputDebugStringA(msg);

    // ライト行列を更新
    UpdateLightMatrix();

    // 最終確認：DSVが本当にセットされているか
    ID3D11DepthStencilView* currentDSV = nullptr;
    ID3D11RenderTargetView* currentRTV = nullptr;
    m_DeviceContext->OMGetRenderTargets(1, &currentRTV, &currentDSV);

    if (currentDSV == m_ShadowMapDSV.Get())
    {
        OutputDebugStringA("SUCCESS: Shadow map DSV is correctly bound\n");
    }
    else
    {
        OutputDebugStringA("ERROR: Shadow map DSV is NOT bound!\n");
    }

    if (currentDSV) currentDSV->Release();
    if (currentRTV) currentRTV->Release();

    OutputDebugStringA("===== BeginShadowPass Complete =====\n\n");
}
/**
 * @brief 描画を終了して、画面に表示します。
 *
 * @details
 * Presentでバックバッファとフロントバッファを入れ替えます。
 */
void Renderer::End()
{
    m_SwapChain->Present(1, 0);
}

void Renderer::EndShadowPass()
{
    if (!m_ShadowMapEnabled) return;


    // 通常のレンダーターゲットに戻す
    m_DeviceContext->OMSetRenderTargets(1, m_SavedRenderTarget.GetAddressOf(), m_SavedDepthStencil.Get());
    m_DeviceContext->RSSetViewports(1, &m_SavedViewport);

    // シャドウマップをピクセルシェーダーにバインド
    m_DeviceContext->PSSetShaderResources(1, 1, m_ShadowMapSRV.GetAddressOf());
    m_DeviceContext->PSSetSamplers(1, 1, m_ShadowSampler.GetAddressOf());

    m_CurrentRenderMode = RenderMode::NORMAL;

    // 保存した参照をクリア
    m_SavedRenderTarget.Reset();
    m_SavedDepthStencil.Reset();
}


void Renderer::SetLightMatrix(const DirectX::XMMATRIX& view, const DirectX::XMMATRIX& projection)
{
    m_LightViewMatrix = view;
    m_LightProjectionMatrix = projection;
}

void Renderer::UpdateLightMatrix()
{
    if (!m_ShadowMapEnabled) return;
    struct LightMatrixBuffer
    {
        DirectX::XMMATRIX view;
        DirectX::XMMATRIX projection;
    };

    LightMatrixBuffer buffer;
    buffer.view = DirectX::XMMatrixTranspose(m_LightViewMatrix);
    buffer.projection = DirectX::XMMatrixTranspose(m_LightProjectionMatrix);

    m_DeviceContext->UpdateSubresource(m_LightMatrixBuffer.Get(), 0, nullptr, &buffer, 0, 0);
    m_DeviceContext->VSSetConstantBuffers(6, 1, m_LightMatrixBuffer.GetAddressOf());
}

/**
 * @brief 深度テスト（Zバッファ）の有効/無効を切り替えます。
 * @param Enable trueなら有効、falseなら無効
 * @details
 * 深度テストは、奥にあるものを正しく手前のものの裏に描画するための機能です。
 */
void Renderer::SetDepthEnable(bool Enable)
{
    m_DeviceContext->OMSetDepthStencilState(
        Enable ? m_DepthStateEnable.Get() : m_DepthStateDisable.Get(), 0);
}

/**
 * @brief Alpha To Coverage（半透明表現用）のON/OFFを切り替えます。
 * @param Enable trueならATC有効、falseなら無効
 * @details
 * マルチサンプリング＋アルファブレンドの高度な合成を行う機能です。
 */
void Renderer::SetATCEnable(bool Enable)
{
    float blendFactor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
    m_DeviceContext->OMSetBlendState(
        Enable ? m_BlendStateATC.Get() : m_BlendState[0].Get(),
        blendFactor, 0xffffffff);
}

/**
 * @brief 2D描画用に、単純なワールド・ビュー・プロジェクション行列をセットします。
 *
 * @details
 * 画面左上を原点とする2D直交投影行列を生成し、各行列バッファに設定します。
 */
void Renderer::SetWorldViewProjection2D()
{
    Matrix4x4 world = Matrix4x4::Identity.Transpose();
    m_DeviceContext->UpdateSubresource(m_WorldBuffer.Get(), 0, nullptr, &world, 0, 0);

    Matrix4x4 view = Matrix4x4::Identity.Transpose();
    m_DeviceContext->UpdateSubresource(m_ViewBuffer.Get(), 0, nullptr, &view, 0, 0);

    Matrix4x4 projection = DirectX::XMMatrixOrthographicOffCenterLH(
        0.0f,
        static_cast<float>(Application::GetWidth()),
        static_cast<float>(Application::GetHeight()),
        0.0f,
        0.0f,
        1.0f);
    projection = projection.Transpose();
    m_DeviceContext->UpdateSubresource(m_ProjectionBuffer.Get(), 0, nullptr, &projection, 0, 0);
}

/**
 * @brief 任意のワールド行列をシェーダーにセットします。
 * @param WorldMatrix ワールド行列へのポインタ
 */
void Renderer::SetWorldMatrix(Matrix4x4* WorldMatrix)
{
    m_CurrentWorld = *WorldMatrix;
    Matrix4x4 mat = WorldMatrix->Transpose();
    m_DeviceContext->UpdateSubresource(m_WorldBuffer.Get(), 0, nullptr, &mat, 0, 0);
}

/**
 * @brief 任意のビュー行列をシェーダーにセットします。
 * @param ViewMatrix ビュー行列へのポインタ
 */
void Renderer::SetViewMatrix(Matrix4x4* ViewMatrix)
{
    m_CurrentView = *ViewMatrix;
    Matrix4x4 mat = ViewMatrix->Transpose();
    m_DeviceContext->UpdateSubresource(m_ViewBuffer.Get(), 0, nullptr, &mat, 0, 0);
}

/**
 * @brief 任意のプロジェクション行列をシェーダーにセットします。
 * @param ProjectionMatrix 射影行列へのポインタ
 */
void Renderer::SetProjectionMatrix(Matrix4x4* ProjectionMatrix)
{
    m_CurrentProjection = *ProjectionMatrix;
    Matrix4x4 mat = ProjectionMatrix->Transpose();
    m_DeviceContext->UpdateSubresource(m_ProjectionBuffer.Get(), 0, nullptr, &mat, 0, 0);
}


/**
 * @brief マテリアル（表面材質）情報をセットします。
 * @param Material マテリアル情報
 */
void Renderer::SetMaterial(MATERIAL Material)
{
    m_DeviceContext->UpdateSubresource(m_MaterialBuffer.Get(), 0, nullptr, &Material, 0, 0);
}

/**
 * @brief ライト（光源）情報をセットします。
 * @param Light ライト情報
 */
void Renderer::SetLight(LIGHT Light)
{
    m_DeviceContext->UpdateSubresource(m_LightBuffer.Get(), 0, nullptr, &Light, 0, 0);
    m_DeviceContext->VSSetConstantBuffers(4, 1, m_LightBuffer.GetAddressOf());
    m_DeviceContext->PSSetConstantBuffers(4, 1, m_LightBuffer.GetAddressOf());
}

void Renderer::SetDepthTestOnly()
{
    static ComPtr<ID3D11DepthStencilState> depthTestOnlyState;

    if (!depthTestOnlyState)
    {
        D3D11_DEPTH_STENCIL_DESC desc{};
        desc.DepthEnable = TRUE;
        desc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
        desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
        desc.StencilEnable = FALSE;

        m_Device->CreateDepthStencilState(
            &desc,
            depthTestOnlyState.GetAddressOf());
    }

    m_DeviceContext->OMSetDepthStencilState(
        depthTestOnlyState.Get(), 0);
}

/**
 * @brief 指定したブレンドステートをセットします。
 * @param nBlendState 使用するブレンドステートの種類
 */
void Renderer::SetBlendState(int nBlendState)
{
    if (nBlendState >= 0 && nBlendState < MAX_BLENDSTATE) {
        float blendFactor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
        m_DeviceContext->OMSetBlendState(m_BlendState[nBlendState].Get(), blendFactor, 0xffffffff);
    }
}

/**
 * @brief 面の除外（カリング）を無効または有効にします。
 * @param cullflag trueでカリングON（通常）、falseでカリングOFF（両面描画）
 */
void Renderer::DisableCulling(bool cullflag)
{
    D3D11_RASTERIZER_DESC rasterizerDesc{};
    rasterizerDesc.FillMode = D3D11_FILL_SOLID;
    rasterizerDesc.CullMode = cullflag ? D3D11_CULL_BACK : D3D11_CULL_NONE;
    rasterizerDesc.FrontCounterClockwise = FALSE;
    rasterizerDesc.DepthBias = 0;
    rasterizerDesc.SlopeScaledDepthBias = 0.0f;
    rasterizerDesc.DepthClipEnable = TRUE;
    rasterizerDesc.ScissorEnable = FALSE;
    rasterizerDesc.MultisampleEnable = FALSE;
    rasterizerDesc.AntialiasedLineEnable = FALSE;

    ComPtr<ID3D11RasterizerState> pRasterizerState;
    HRESULT hr = m_Device->CreateRasterizerState(&rasterizerDesc, pRasterizerState.GetAddressOf());
    if (FAILED(hr))
        return;

    m_DeviceContext->RSSetState(pRasterizerState.Get());
}


/**
 * @brief ラスタライザステートのフィルモード（塗りつぶし/ワイヤーフレーム）を設定します。
 * @param FillMode D3D11_FILL_SOLID または D3D11_FILL_WIREFRAME
 */
void Renderer::SetFillMode(D3D11_FILL_MODE FillMode)
{
    D3D11_RASTERIZER_DESC rasterizerDesc{};
    rasterizerDesc.FillMode = FillMode;
    rasterizerDesc.CullMode = D3D11_CULL_BACK;
    rasterizerDesc.DepthClipEnable = TRUE;
    rasterizerDesc.MultisampleEnable = FALSE;

    ComPtr<ID3D11RasterizerState> rs;
    m_Device->CreateRasterizerState(&rasterizerDesc, rs.GetAddressOf());
    m_DeviceContext->RSSetState(rs.Get());
}

/**
 * @brief 深度テストを常にパスさせる設定に変更します。
 *
 * @details
 * - 深度テストは有効（DepthEnable = TRUE）
 * - ただし、常に「描画OK」（DepthFunc = D3D11_COMPARISON_ALWAYS）
 * - 深度バッファにも書き込む（DepthWriteMask = ALL）
 */
void Renderer::SetDepthAllwaysWrite()
{
    D3D11_DEPTH_STENCIL_DESC depthStencilDesc{};
    depthStencilDesc.DepthEnable = TRUE;
    depthStencilDesc.DepthFunc = D3D11_COMPARISON_ALWAYS; // 常に深度テスト成功
    depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
    depthStencilDesc.StencilEnable = FALSE; // ステンシルテストは無効

    ComPtr<ID3D11DepthStencilState> pDepthStencilState;
    HRESULT hr = m_Device->CreateDepthStencilState(&depthStencilDesc, pDepthStencilState.GetAddressOf());
    if (SUCCEEDED(hr))
    {
        m_DeviceContext->OMSetDepthStencilState(pDepthStencilState.Get(), 0);
    }
}
