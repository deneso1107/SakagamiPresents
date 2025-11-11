#include "PostProcessManager.h"
#include "system/renderer.h"
#include "Application.h"

//void PostProcessManager::Initialize()
//{
//    CreateTextures();
//    CreateShaders();
//    CreateStates();
//
//    OutputDebugStringA("PostProcessManager 初期化完了\n");
//}
//
//void PostProcessManager::Finalize()
//{
//    m_sceneTexture_internal.Reset();
//    m_sceneRTV.Reset();
//    m_sceneDSV.Reset();
//    m_sceneSRV.Reset();
//    m_intermediateTexture.Reset();
//    m_intermediateRTV.Reset();
//    m_intermediateSRV.Reset();
//    m_sampler.Reset();
//    m_depthStateDisabled.Reset();
//    m_rasterizerState.Reset();
//
//    OutputDebugStringA("PostProcessManager 終了処理完了\n");
//}
//
//void PostProcessManager::CreateTextures()
//{
//    int width = Application::GetWidth();
//    int height = Application::GetHeight();
//    ID3D11Device* device = Renderer::GetDevice();
//
//    // シーン描画用テクスチャ
//    D3D11_TEXTURE2D_DESC textureDesc = {};
//    textureDesc.Width = width;
//    textureDesc.Height = height;
//    textureDesc.MipLevels = 1;
//    textureDesc.ArraySize = 1;
//    textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
//    textureDesc.Usage = D3D11_USAGE_DEFAULT;
//    textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
//    textureDesc.SampleDesc.Count = 1;
//
//    device->CreateTexture2D(&textureDesc, nullptr, m_sceneTexture_internal.GetAddressOf());
//    device->CreateRenderTargetView(m_sceneTexture_internal.Get(), nullptr, m_sceneRTV.GetAddressOf());
//    device->CreateShaderResourceView(m_sceneTexture_internal.Get(), nullptr, m_sceneSRV.GetAddressOf());
//
//    // 深度バッファ
//    D3D11_TEXTURE2D_DESC depthDesc = {};
//    depthDesc.Width = width;
//    depthDesc.Height = height;
//    depthDesc.MipLevels = 1;
//    depthDesc.ArraySize = 1;
//    depthDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
//    depthDesc.Usage = D3D11_USAGE_DEFAULT;
//    depthDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
//    depthDesc.SampleDesc.Count = 1;
//
//    device->CreateTexture2D(&depthDesc, nullptr, m_sceneTexture_internal.GetAddressOf());
//    device->CreateDepthStencilView(m_sceneTexture_internal.Get(), nullptr, m_sceneDSV.GetAddressOf());
//
//    // 中間テクスチャ
//    device->CreateTexture2D(&textureDesc, nullptr, m_intermediateTexture.GetAddressOf());
//    device->CreateRenderTargetView(m_intermediateTexture.Get(), nullptr, m_intermediateRTV.GetAddressOf());
//    device->CreateShaderResourceView(m_intermediateTexture.Get(), nullptr, m_intermediateSRV.GetAddressOf());
//
//    // サンプラー作成
//    D3D11_SAMPLER_DESC samplerDesc = {};
//    samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
//    samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
//    samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
//    samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
//    samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
//    samplerDesc.MinLOD = 0;
//    samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
//
//    device->CreateSamplerState(&samplerDesc, m_sampler.GetAddressOf());
//}
//
//void PostProcessManager::CreateShaders()
//{
//    m_motionBlurShader.Create(
//        "shader/motion_blurVS.hlsl",
//        "shader/motion_blurPS.hlsl");
//
//    m_chromaticShader.Create(
//        "shader/chromatic_aberrationVS.hlsl",
//        "shader/chromatic_aberrationPS.hlsl");
//
//    m_shockwaveShader.Create(
//        "shader/shockwaveVS.hlsl",
//        "shader/shockwavePS.hlsl");
//}
//
//void PostProcessManager::CreateStates()
//{
//    ID3D11Device* device = Renderer::GetDevice();
//
//    // 深度ステート（無効化）
//    D3D11_DEPTH_STENCIL_DESC depthStencilDesc = {};
//    depthStencilDesc.DepthEnable = FALSE;
//    depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
//
//    device->CreateDepthStencilState(&depthStencilDesc, m_depthStateDisabled.GetAddressOf());
//
//    // ラスタライザステート
//    D3D11_RASTERIZER_DESC rasterDesc = {};
//    rasterDesc.FillMode = D3D11_FILL_SOLID;
//    rasterDesc.CullMode = D3D11_CULL_NONE;
//    rasterDesc.FrontCounterClockwise = FALSE;
//    rasterDesc.DepthClipEnable = TRUE;
//
//    device->CreateRasterizerState(&rasterDesc, m_rasterizerState.GetAddressOf());
//}
//
//void PostProcessManager::EnableMotionBlur(float strength)
//{
//    m_motionBlur.enabled = true;
//    m_motionBlur.strength = strength;
//}
//
//void PostProcessManager::EnableChromaticAberration(float strength, float duration)
//{
//    m_chromatic.enabled = true;
//    m_chromatic.strength = strength;
//    m_chromatic.duration = duration;
//    m_chromatic.elapsedTime = 0.0f;
//}
//
//void PostProcessManager::EnableShockwave(float intensity)
//{
//    m_shockwave.enabled = true;
//    m_shockwave.intensity = intensity;
//}
//
//void PostProcessManager::DisableMotionBlur()
//{
//    m_motionBlur.enabled = false;
//}
//
//void PostProcessManager::DisableChromaticAberration()
//{
//    m_chromatic.enabled = false;
//}
//
//void PostProcessManager::DisableShockwave()
//{
//    m_shockwave.enabled = false;
//}
//
//void PostProcessManager::Update(float deltaTime)
//{
//    if (m_chromatic.enabled)
//    {
//        m_chromatic.elapsedTime += deltaTime;
//        m_chromatic.time += deltaTime;
//
//        if (m_chromatic.elapsedTime >= m_chromatic.duration)
//        {
//            DisableChromaticAberration();
//        }
//    }
//}
//
//void PostProcessManager::Render(ID3D11RenderTargetView* backBuffer)
//{
//    if (!m_motionBlur.enabled && !m_chromatic.enabled && !m_shockwave.enabled)
//        return;
//
//    ID3D11DeviceContext* context = Renderer::GetDeviceContext();
//
//    // ステート保存
//    ID3D11DepthStencilState* originalDepthState = nullptr;
//    UINT originalStencilRef = 0;
//    context->OMGetDepthStencilState(&originalDepthState, &originalStencilRef);
//
//    ID3D11RasterizerState* originalRasterState = nullptr;
//    context->RSGetState(&originalRasterState);
//
//    // ステート設定
//    SetupRenderState(context);
//
//    ID3D11ShaderResourceView* currentInput = m_sceneSRV.Get();
//    ID3D11RenderTargetView* currentOutput = nullptr;
//
//    // ★ パス1: Motion Blur
//    if (m_motionBlur.enabled && m_motionBlur.strength > 0.0f)
//    {
//        currentOutput = (m_chromatic.enabled || m_shockwave.enabled) ? m_intermediateRTV.Get() : backBuffer;
//        ApplyMotionBlur(context, currentInput, currentOutput);
//
//        currentInput = m_intermediateSRV.Get();
//    }
//
//    // ★ パス2: Chromatic Aberration
//    if (m_chromatic.enabled && m_chromatic.strength > 0.0f)
//    {
//        currentOutput = m_shockwave.enabled ? m_intermediateRTV.Get() : backBuffer;
//        ApplyChromaticAberration(context, currentInput, currentOutput);
//
//        currentInput = m_intermediateSRV.Get();
//    }
//
//    // ★ パス3: Shockwave
//    if (m_shockwave.enabled && m_shockwave.intensity > 0.0f)
//    {
//        ApplyShockwave(context, currentInput, backBuffer);
//    }
//
//    // ステート復元
//    RestoreRenderState(context, originalDepthState, originalStencilRef, originalRasterState);
//}
//
//void PostProcessManager::ApplyMotionBlur(ID3D11DeviceContext* context,
//    ID3D11ShaderResourceView* input, ID3D11RenderTargetView* output)
//{
//    context->OMSetRenderTargets(1, &output, nullptr);
//
//    float clearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
//    context->ClearRenderTargetView(output, clearColor);
//
//    struct MotionBlurBuffer
//    {
//        float strength;
//        float centerX;
//        float centerY;
//        float padding;
//    };
//
//    MotionBlurBuffer cb;
//    cb.strength = m_motionBlur.strength;
//    cb.centerX = m_motionBlur.centerX;
//    cb.centerY = m_motionBlur.centerY;
//
//    ComPtr<ID3D11Buffer> constantBuffer;
//    D3D11_BUFFER_DESC bufferDesc = {};
//    bufferDesc.ByteWidth = sizeof(MotionBlurBuffer);
//    bufferDesc.Usage = D3D11_USAGE_DEFAULT;
//    bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
//
//    D3D11_SUBRESOURCE_DATA initData = {};
//    initData.pSysMem = &cb;
//
//    Renderer::GetDevice()->CreateBuffer(&bufferDesc, &initData, constantBuffer.GetAddressOf());
//
//    m_motionBlurShader.SetGPU();
//    context->PSSetConstantBuffers(0, 1, constantBuffer.GetAddressOf());
//    context->PSSetShaderResources(0, 1, &input);
//    context->PSSetSamplers(0, 1, m_sampler.GetAddressOf());
//
//    context->IASetInputLayout(nullptr);
//    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
//    context->Draw(4, 0);
//}
//
//void PostProcessManager::ApplyChromaticAberration(ID3D11DeviceContext* context,
//    ID3D11ShaderResourceView* input, ID3D11RenderTargetView* output)
//{
//    context->OMSetRenderTargets(1, &output, nullptr);
//
//    float clearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
//    context->ClearRenderTargetView(output, clearColor);
//
//    struct ChromaticBuffer
//    {
//        float strength;
//        float time;
//        float padding1;
//        float padding2;
//    };
//
//    ChromaticBuffer cb;
//    cb.strength = m_chromatic.strength;
//    cb.time = m_chromatic.time;
//
//    ComPtr<ID3D11Buffer> constantBuffer;
//    D3D11_BUFFER_DESC bufferDesc = {};
//    bufferDesc.ByteWidth = sizeof(ChromaticBuffer);
//    bufferDesc.Usage = D3D11_USAGE_DEFAULT;
//    bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
//
//    D3D11_SUBRESOURCE_DATA initData = {};
//    initData.pSysMem = &cb;
//
//    Renderer::GetDevice()->CreateBuffer(&bufferDesc, &initData, constantBuffer.GetAddressOf());
//
//    m_chromaticShader.SetGPU();
//    context->PSSetConstantBuffers(0, 1, constantBuffer.GetAddressOf());
//    context->PSSetShaderResources(0, 1, &input);
//    context->PSSetSamplers(0, 1, m_sampler.GetAddressOf());
//
//    context->Draw(4, 0);
//}
//
//void PostProcessManager::ApplyShockwave(ID3D11DeviceContext* context,
//    ID3D11ShaderResourceView* input, ID3D11RenderTargetView* output)
//{
//    context->OMSetRenderTargets(1, &output, nullptr);
//
//    float clearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
//    context->ClearRenderTargetView(output, clearColor);
//
//    struct ShockwaveBuffer
//    {
//        float intensity;
//        float progress;
//        float padding1;
//        float padding2;
//    };
//
//    ShockwaveBuffer cb;
//    cb.intensity = m_shockwave.intensity;
//    cb.progress = m_shockwave.progress;
//
//    ComPtr<ID3D11Buffer> constantBuffer;
//    D3D11_BUFFER_DESC bufferDesc = {};
//    bufferDesc.ByteWidth = sizeof(ShockwaveBuffer);
//    bufferDesc.Usage = D3D11_USAGE_DEFAULT;
//    bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
//
//    D3D11_SUBRESOURCE_DATA initData = {};
//    initData.pSysMem = &cb;
//
//    Renderer::GetDevice()->CreateBuffer(&bufferDesc, &initData, constantBuffer.GetAddressOf());
//
//    m_shockwaveShader.SetGPU();
//    context->PSSetConstantBuffers(0, 1, constantBuffer.GetAddressOf());
//    context->PSSetShaderResources(0, 1, &input);
//    context->PSSetSamplers(0, 1, m_sampler.GetAddressOf());
//
//    context->Draw(4, 0);
//}
//
//void PostProcessManager::SetupRenderState(ID3D11DeviceContext* context)
//{
//    context->OMSetDepthStencilState(m_depthStateDisabled.Get(), 0);
//    context->RSSetState(m_rasterizerState.Get());
//}
//
//void PostProcessManager::RestoreRenderState(ID3D11DeviceContext* context,
//    ID3D11DepthStencilState* depthState, UINT stencilRef, ID3D11RasterizerState* rasterState)
//{
//    if (depthState)
//    {
//        context->OMSetDepthStencilState(depthState, stencilRef);
//        depthState->Release();
//    }
//
//    if (rasterState)
//    {
//        context->RSSetState(rasterState);
//        rasterState->Release();
//    }
//}