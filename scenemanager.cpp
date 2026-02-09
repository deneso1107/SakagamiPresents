#include	"system/IScene.h"
#include	"scenemanager.h"
#include	"CarDriveScene.h"
#include	"Title.h"
#include"Ending.h"
#include <algorithm>
#include <iostream>
#include <WICTextureLoader.h>
// 静的メンバの初期化
std::unordered_map<std::string, std::unique_ptr<IScene>> SceneManager::m_scenes;
std::string SceneManager::m_currentSceneName = "";
std::string SceneManager::m_nextSceneName = "";
bool SceneManager::m_sceneChangeRequested = false;

SceneManager::TransitionState SceneManager::m_transitionState = TransitionState::None;
float SceneManager::m_slideOffset = 0.0f;
float SceneManager::m_transitionSpeed = 2.0f;
bool SceneManager::m_sceneLoaded = false;
float SceneManager::m_loadingRotation = 0.0f;
float SceneManager::m_fadeAlpha = 0.0f;


std::thread  SceneManager::m_loadingThread;
std::atomic<bool>  SceneManager::m_asyncLoading;
std::atomic<bool>  SceneManager::m_asyncFinished=true;
CShader SceneManager::m_transitionShader;
CShader SceneManager::m_blackfadeShader;
ScreenFixedBillboard* SceneManager::m_BillboardLoad;
ScreenFixedBillboard* SceneManager::m_BillboardCowIcon;
ID3D11Buffer* SceneManager::m_transitionVertexBuffer = nullptr;
ID3D11Buffer* SceneManager::m_transitionConstantBuffer = nullptr;
ID3D11ShaderResourceView* SceneManager::m_transitionTexture = nullptr;
ID3D11DepthStencilState* SceneManager::m_transitionDepthState = nullptr;
ID3D11BlendState* SceneManager::m_transitionBlendState = nullptr;
ID3D11SamplerState* SceneManager::m_transitionSamplerState = nullptr;

int m_gameScore = 0;

void SceneManager::Init()
{
    // シーンの登録
    RegisterScene<CarDriveScene>("CarDriveScene");
    RegisterScene<Title>("Title");
    RegisterScene<Ending>("Ending");

    // トランジションリソースの初期化
    InitTransitionResources();

    // 初期シーンの設定
    if (!m_scenes.empty())
    {
        m_currentSceneName = m_scenes.begin()->first;
        m_scenes[m_currentSceneName]->init();
    }
}

void SceneManager::InitTransitionResources()
{
    // トランジション用シェーダーの作成
    m_transitionShader.Create2D("shader/TransitionVS.hlsl", "shader/TransitionPS.hlsl");
    m_blackfadeShader.Create2D("shader/BlackFadeVS.hlsl", "shader/BlackFadePS.hlsl");
    std::wcout << L"Transition Shader created successfully" << std::endl;

    // フルスクリーン矩形の頂点データ
    TransitionVertex vertices[4] = {
        { -1.0f,  1.0f, 0.0f, 0.0f, 0.0f }, // 左上
        {  1.0f,  1.0f, 0.0f, 1.0f, 0.0f }, // 右上
        { -1.0f, -1.0f, 0.0f, 0.0f, 1.0f }, // 左下
        {  1.0f, -1.0f, 0.0f, 1.0f, 1.0f }  // 右下
    };

    // 頂点バッファの作成
    D3D11_BUFFER_DESC bufferDesc = {};
    bufferDesc.ByteWidth = sizeof(vertices);
    bufferDesc.Usage = D3D11_USAGE_DEFAULT;
    bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

    D3D11_SUBRESOURCE_DATA initData = {};
    initData.pSysMem = vertices;

    HRESULT hr = Renderer::GetDevice()->CreateBuffer(&bufferDesc, &initData, &m_transitionVertexBuffer);
    if (FAILED(hr)) {
        printf("ERROR: Failed to create transition vertex buffer!\n");
    }

    // 定数バッファの作成
    D3D11_BUFFER_DESC cbDesc = {};
    cbDesc.ByteWidth = sizeof(TransitionConstantBuffer);
    cbDesc.Usage = D3D11_USAGE_DYNAMIC;
    cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    hr = Renderer::GetDevice()->CreateBuffer(&cbDesc, nullptr, &m_transitionConstantBuffer);
    if (FAILED(hr)) {
        printf("ERROR: Failed to create transition constant buffer!\n");
    }

    //深度ステンシルステートの作成（キャッシュ）
    D3D11_DEPTH_STENCIL_DESC depthDesc = {};
    depthDesc.DepthEnable = FALSE;
    depthDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
    depthDesc.DepthFunc = D3D11_COMPARISON_ALWAYS;
    depthDesc.StencilEnable = FALSE;

    hr = Renderer::GetDevice()->CreateDepthStencilState(&depthDesc, &m_transitionDepthState);
    if (FAILED(hr)) {
        printf("ERROR: Failed to create transition depth state!\n");
    }

    //ブレンドステートの作成（キャッシュ）
    D3D11_BLEND_DESC blendDesc = {};
    blendDesc.RenderTarget[0].BlendEnable = TRUE;
    blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
    blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
    blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
    blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
    blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

    hr = Renderer::GetDevice()->CreateBlendState(&blendDesc, &m_transitionBlendState);
    if (FAILED(hr)) {
        printf("ERROR: Failed to create transition blend state!\n");
    }

    //サンプラーステートの作成（キャッシュ）
    D3D11_SAMPLER_DESC samplerDesc = {};
    samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
    samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
    samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
    samplerDesc.MaxAnisotropy = 1;
    samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    samplerDesc.MinLOD = 0;
    samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

    hr = Renderer::GetDevice()->CreateSamplerState(&samplerDesc, &m_transitionSamplerState);
    if (FAILED(hr)) {
        printf("ERROR: Failed to create transition sampler state!\n");
    }

    // ロケット牛の画像を読み込み
    LoadTransitionTexture(L"assets/texture/cow_rocket.png");
    m_BillboardLoad = new ScreenFixedBillboard(Vector2(0.85f, 0.9f), 0.15f, 0.15f, L"assets/texture/Image.png");
    m_BillboardCowIcon = new ScreenFixedBillboard(Vector2(0.95f, 0.90f), 0.05f, 0.05f, L"assets/texture/cow_icon.png");
    //LoadTransitionTexture(L"assets/texture/cow_rocket.png", m_loadingTextTexture);
    //LoadTransitionTexture(L"assets/texture/cow_rocket.png", m_cowIconTexture);
}

void SceneManager::LoadTransitionTexture(const wchar_t* filepath)
{
    // WICTextureLoaderを使用（ScreenFixedBillboardと同じ方法）
    HRESULT hr = DirectX::CreateWICTextureFromFile(
        Renderer::GetDevice(),
        filepath,
        nullptr,
        &m_transitionTexture
    );

    if (FAILED(hr)) {
        std::wcout << L"ERROR: Failed to load transition texture: " << filepath << std::endl;
        std::wcout << L"HRESULT: 0x" << std::hex << hr << std::dec << std::endl;
    }
    else {
        std::wcout << L"Transition texture loaded successfully: " << filepath << std::endl;
    }
}

void SceneManager::Update(float deltaTime)
{
    // トランジション処理の更新
    UpdateTransition(deltaTime);

    // シーン切り替え処理
    ProcessSceneChange();
    // 現在のシーンの更新
    if (m_asyncFinished)
    {
        if (!m_currentSceneName.empty() && m_scenes.find(m_currentSceneName) != m_scenes.end()) {
            m_scenes[m_currentSceneName]->update(deltaTime);
        }
    }
    m_BillboardLoad->Update();
    m_BillboardCowIcon->Update();
}

void SceneManager::Draw(float deltaTime)
{


    // 現在のシーンの描画
    if (m_asyncFinished)
    {
        if (!m_currentSceneName.empty() && m_scenes.find(m_currentSceneName) != m_scenes.end()) {
            m_scenes[m_currentSceneName]->draw(deltaTime);
        }
    }

    // トランジション描画（最前面）
    if (m_transitionState != TransitionState::None) {
        // 1. まず黒背景フェード
        if (m_fadeAlpha > 0.0f)
        {
            DrawBlackFade();
        }
        DrawTransitionOverlay();

        // ローディング中の表示
        if (m_transitionState == TransitionState::Loading) 
        {
            DrawLoadingIndicator();
        }
    }

}

void SceneManager::UpdateTransition(float deltaTime)
{
    if (m_transitionState == TransitionState::None) return;

    // deltaTimeを秒単位に変換（マイクロ秒の場合）
    float dt = deltaTime;

    switch (m_transitionState) {
    case TransitionState::SlideIn:
        // 右から中央へ (1.5 → 0.0)
        m_slideOffset -= m_transitionSpeed * dt;

        // 背景フェードイン (0.0 → 1.0)
        m_fadeAlpha += m_transitionSpeed * dt;
        if (m_fadeAlpha > 1.0f) m_fadeAlpha = 1.0f;

        if (m_slideOffset <= 0.0f) {
            m_slideOffset = 0.0f;
            // 中央到達、ロード開始
            m_transitionState = TransitionState::Loading;
            m_sceneLoaded = false;
            m_loadingRotation = 0.0f;
        }
        break;

    case TransitionState::Loading:

        m_loadingRotation += dt;
        m_fadeAlpha = 1.0f;

        m_loadingRotation += PI*2 * dt;
		m_BillboardCowIcon->SetAngle(m_loadingRotation * (180.0f / 3.14159f)); // ラジアンを度に変換


        LoadNextSceneAsync();

        // 非同期ロード完了チェック
        if (m_asyncFinished)
        {
            //GPU生成はメインスレッド
            m_currentSceneName = m_nextSceneName;
            m_scenes[m_currentSceneName]->init();

            m_sceneLoaded = true;
            m_transitionState = TransitionState::SlideOut;
            m_asyncLoading = false;
        }
        break;

    case TransitionState::SlideOut:
        // 中央から左へ (0.0 → -1.5)
        m_fadeAlpha = 0.0f;
        m_slideOffset -= m_transitionSpeed * dt;
        if (m_slideOffset <= -1.5f) {
            m_slideOffset = -1.5f;
            m_transitionState = TransitionState::None;
        }
        break;
    }
}

void SceneManager::LoadNextSceneAsync()
{
    if (m_asyncLoading) return;

    m_asyncLoading = true;
    m_asyncFinished = false;

    // 旧シーンの破棄（メインスレッド）
    if (!m_currentSceneName.empty())
        m_scenes[m_currentSceneName]->dispose();

    std::string next = m_nextSceneName;

    //ワーカースレッド起動
        m_loadingThread = std::thread([next]() {
        m_scenes[next]->loadAsync();  // ← CPUロードだけ
        m_asyncFinished = true;
        });

    m_loadingThread.detach();
}

void SceneManager::ProcessSceneChange()
{
    // トランジション中の処理は UpdateTransition で行う
    if (!m_sceneChangeRequested) return;

    // トランジションなしの場合、即座に切り替え
    if (m_transitionState == TransitionState::None) {
        m_sceneChangeRequested = false;
    }
}

void SceneManager::ChangeScene(const std::string& sceneName, bool useTransition)
{
    if (m_scenes.find(sceneName) == m_scenes.end()) {
        throw std::runtime_error("Scene not found: " + sceneName);
    }

    if (m_sceneChangeRequested || m_transitionState != TransitionState::None) {
        return; // 既にシーン変更中
    }
    SoundManager::GetInstance().PlaySE("Click");

    m_nextSceneName = sceneName;
    m_sceneChangeRequested = true;

    if (useTransition) {
        m_transitionState = TransitionState::SlideIn;
        m_slideOffset = 1.5f; // 画面右端外からスタート
        m_loadingRotation = 0.0f;
        m_fadeAlpha = 0.0f;   // フェード初期化
    }
    else {
        // トランジションなしの場合は即座にロード
        LoadNextSceneAsync();
        m_sceneChangeRequested = false;
    }
}

void SceneManager::DrawBlackFade()
{
    if (!m_transitionVertexBuffer || !m_transitionConstantBuffer) {
        return;
    }

    ID3D11DeviceContext* context = Renderer::GetDeviceContext();

    // --- Save many states ---
    ID3D11DepthStencilState* prevDepth = nullptr;
    UINT prevStencilRef = 0;
    context->OMGetDepthStencilState(&prevDepth, &prevStencilRef);

    ID3D11BlendState* prevBlend = nullptr;
    FLOAT prevBlendFactor[4] = {};
    UINT prevSampleMask = 0;
    context->OMGetBlendState(&prevBlend, prevBlendFactor, &prevSampleMask);

    ID3D11RasterizerState* prevRaster = nullptr;
    context->RSGetState(&prevRaster);

    ID3D11SamplerState* prevPSSampler = nullptr;
    context->PSGetSamplers(0, 1, &prevPSSampler);

    ID3D11ShaderResourceView* prevPS_SRV = nullptr;
    context->PSGetShaderResources(0, 1, &prevPS_SRV);

    ID3D11Buffer* prevVSCB = nullptr;
    context->VSGetConstantBuffers(0, 1, &prevVSCB);

    ID3D11Buffer* prevPSCB = nullptr;
    context->PSGetConstantBuffers(0, 1, &prevPSCB);

    ID3D11VertexShader* prevVS = nullptr;
    ID3D11ClassInstance* vsInstances[1] = { nullptr };
    context->VSGetShader(&prevVS, nullptr, nullptr);

    ID3D11PixelShader* prevPS = nullptr;
    ID3D11ClassInstance* psInstances[1] = { nullptr };
    context->PSGetShader(&prevPS, nullptr, nullptr);

    ID3D11InputLayout* prevLayout = nullptr;
    context->IAGetInputLayout(&prevLayout);

    D3D11_PRIMITIVE_TOPOLOGY prevTopology;
    context->IAGetPrimitiveTopology(&prevTopology);

    // Save IA vertex buffer (slot 0)
    ID3D11Buffer* prevVB = nullptr;
    UINT prevStride = 0;
    UINT prevOffset = 0;
    context->IAGetVertexBuffers(0, 1, &prevVB, &prevStride, &prevOffset);

    // Save index buffer (if any)
    ID3D11Buffer* prevIB = nullptr;
    DXGI_FORMAT prevIBFormat = DXGI_FORMAT_UNKNOWN;
    UINT prevIBOffset = 0;
    context->IAGetIndexBuffer(&prevIB, &prevIBFormat, &prevIBOffset);

    // -------------------------
    // Set states for black fade
    // -------------------------
    if (m_transitionDepthState)
        context->OMSetDepthStencilState(m_transitionDepthState, 0);
    if (m_transitionBlendState)
        context->OMSetBlendState(m_transitionBlendState, nullptr, 0xffffffff);
    if (m_transitionSamplerState)
        context->PSSetSamplers(0, 1, &m_transitionSamplerState);

    // Update constant buffer (use padding for alpha as your struct currently does)
    {
        D3D11_MAPPED_SUBRESOURCE mapped;
        if (SUCCEEDED(context->Map(m_transitionConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped))) {
            TransitionConstantBuffer* cb = static_cast<TransitionConstantBuffer*>(mapped.pData);
            cb->slideOffset = 0.0f;
            cb->imageYPosition = 0.0f;
            cb->imageScale = 1.0f;
            cb->fadeAlpha = m_fadeAlpha;

            printf("%f", m_fadeAlpha);
            cb->slideOffset = 0.0f;
            cb->imageScale = 1.0f;
            cb->imageYPosition = 0.0f;
            context->Unmap(m_transitionConstantBuffer, 0);
        }
    }

    // Reset matrices (if your renderer uses them)
    Matrix4x4 identity = DirectX::XMMatrixIdentity();
    Renderer::SetWorldMatrix(&identity);
    Renderer::SetViewMatrix(&identity);
    Renderer::SetProjectionMatrix(&identity);

    // Set shader and buffers
    m_blackfadeShader.SetGPU();
    context->VSSetConstantBuffers(0, 1, &m_transitionConstantBuffer);
    context->PSSetConstantBuffers(0, 1, &m_transitionConstantBuffer);

    // Unbind any PS SRV to draw solid black quad (or optionally bind a black texture)
    ID3D11ShaderResourceView* nullSRV[1] = { nullptr };
    context->PSSetShaderResources(0, 1, nullSRV);

    // Set IA - our fullscreen quad VB
    UINT stride = sizeof(TransitionVertex);
    UINT offset = 0;
    context->IASetVertexBuffers(0, 1, &m_transitionVertexBuffer, &stride, &offset);
    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
    context->IASetInputLayout(m_blackfadeShader.GetInputLayout());

    // Draw full-screen quad
    context->Draw(4, 0);

    // -------------------------
    // Restore saved states
    // -------------------------
    context->OMSetBlendState(prevBlend, prevBlendFactor, prevSampleMask);
    context->OMSetDepthStencilState(prevDepth, prevStencilRef);
    context->RSSetState(prevRaster);

    // Restore PS sampler / SRV
    context->PSSetSamplers(0, 1, &prevPSSampler);
    context->PSSetShaderResources(0, 1, &prevPS_SRV);

    // Restore constant buffers
    ID3D11Buffer* restoreVSCB = prevVSCB;
    context->VSSetConstantBuffers(0, 1, &restoreVSCB);
    ID3D11Buffer* restorePSCB = prevPSCB;
    context->PSSetConstantBuffers(0, 1, &restorePSCB);

    // Restore shaders
    context->VSSetShader(prevVS, nullptr, 0);
    context->PSSetShader(prevPS, nullptr, 0);

    // Restore IA state
    context->IASetInputLayout(prevLayout);
    context->IASetPrimitiveTopology(prevTopology);
    context->IASetVertexBuffers(0, 1, &prevVB, &prevStride, &prevOffset);
    context->IASetIndexBuffer(prevIB, prevIBFormat, prevIBOffset);

    // Release saved COM pointers (only if non-null)
    if (prevDepth) prevDepth->Release();
    if (prevBlend) prevBlend->Release();
    if (prevRaster) prevRaster->Release();
    if (prevPSSampler) prevPSSampler->Release();
    if (prevPS_SRV) prevPS_SRV->Release();
    if (prevVSCB) prevVSCB->Release();
    if (prevPSCB) prevPSCB->Release();
    if (prevVS) prevVS->Release();
    if (prevPS) prevPS->Release();
    if (prevLayout) prevLayout->Release();
    if (prevVB) prevVB->Release();
    if (prevIB) prevIB->Release();
}

void SceneManager::DrawTransitionOverlay()
{

    if (!m_transitionVertexBuffer || !m_transitionConstantBuffer)
        return;

    ID3D11DeviceContext* context = Renderer::GetDeviceContext();

    // ============================
    // ステートの完全保存
    // ============================

    // --- DepthStencil ---
    ID3D11DepthStencilState* prevDepthState = nullptr;
    UINT prevStencilRef = 0;
    context->OMGetDepthStencilState(&prevDepthState, &prevStencilRef);

    // --- Blend ---
    ID3D11BlendState* prevBlendState = nullptr;
    FLOAT prevBlendFactor[4] = {};
    UINT prevSampleMask = 0;
    context->OMGetBlendState(&prevBlendState, prevBlendFactor, &prevSampleMask);

    // --- VS ConstantBuffer ---
    ID3D11Buffer* prevVSConstantBuffer = nullptr;
    context->VSGetConstantBuffers(0, 1, &prevVSConstantBuffer);

    // --- PS SRV ---
    ID3D11ShaderResourceView* prevPS_SRV = nullptr;
    context->PSGetShaderResources(0, 1, &prevPS_SRV);

    // --- PS Sampler ---
    ID3D11SamplerState* prevPSSampler = nullptr;
    context->PSGetSamplers(0, 1, &prevPSSampler);

    // --- シェーダ ---
    ID3D11VertexShader* prevVS = nullptr;
    ID3D11PixelShader* prevPS = nullptr;
    context->VSGetShader(&prevVS, nullptr, nullptr);
    context->PSGetShader(&prevPS, nullptr, nullptr);

    // --- InputLayout ---
    ID3D11InputLayout* prevInputLayout = nullptr;
    context->IAGetInputLayout(&prevInputLayout);

    // --- トポロジ ---
    D3D11_PRIMITIVE_TOPOLOGY prevTopology;
    context->IAGetPrimitiveTopology(&prevTopology);

    // ============================
    //トランジション用ステート設定
    // ============================

    // Depth
    if (m_transitionDepthState)
        context->OMSetDepthStencilState(m_transitionDepthState, 0);

    // Blend
    if (m_transitionBlendState)
    {
        FLOAT blendFactor[4] = { 1,1,1,1 };
        context->OMSetBlendState(m_transitionBlendState, blendFactor, 0xffffffff);
    }

    // ConstantBuffer 更新
    {
        D3D11_MAPPED_SUBRESOURCE mapped;
        if (SUCCEEDED(context->Map(m_transitionConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped)))
        {
            auto* cb = (TransitionConstantBuffer*)mapped.pData;
            cb->slideOffset = m_slideOffset;

            if (m_transitionState == TransitionState::Loading)
            {
                cb->imageYPosition = sinf(m_loadingRotation * 3.0f) * 0.05f;
                cb->imageScale = 1.0f + sinf(m_loadingRotation * 2.0f) * 0.02f;
            }
            else
            {
                cb->imageYPosition = 0.0f;
                cb->imageScale = 1.0f;
            }

            cb->fadeAlpha = 0.0f;
            context->Unmap(m_transitionConstantBuffer, 0);
        }
    }

    // 行列リセット
    Matrix4x4 identity = DirectX::XMMatrixIdentity();
    Renderer::SetWorldMatrix(&identity);
    Renderer::SetViewMatrix(&identity);
    Renderer::SetProjectionMatrix(&identity);

    // シェーダ
    m_transitionShader.SetGPU();

    // CB（VS）
    context->VSSetConstantBuffers(0, 1, &m_transitionConstantBuffer);

    // SRV / Sampler
    if (m_transitionTexture)
        context->PSSetShaderResources(0, 1, &m_transitionTexture);

    if (m_transitionSamplerState)
        context->PSSetSamplers(0, 1, &m_transitionSamplerState);

    // 頂点バッファ
    UINT stride = sizeof(TransitionVertex);
    UINT offset = 0;
    context->IASetVertexBuffers(0, 1, &m_transitionVertexBuffer, &stride, &offset);
    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
    context->IASetInputLayout(m_transitionShader.GetInputLayout());

    // 描画
    context->Draw(4, 0);

    // ============================
    //ステート復元（最重要）
    // ============================

    // Depth
    if (prevDepthState)
    {
        context->OMSetDepthStencilState(prevDepthState, prevStencilRef);
        prevDepthState->Release();
    }

    // Blend
    if (prevBlendState)
    {
        context->OMSetBlendState(prevBlendState, prevBlendFactor, prevSampleMask);
        prevBlendState->Release();
    }

    // VS ConstantBuffer
    context->VSSetConstantBuffers(0, 1, &prevVSConstantBuffer);
    if (prevVSConstantBuffer)
        prevVSConstantBuffer->Release();

    // SRV
    context->PSSetShaderResources(0, 1, &prevPS_SRV);
    if (prevPS_SRV)
        prevPS_SRV->Release();

    // Sampler
    context->PSSetSamplers(0, 1, &prevPSSampler);
    if (prevPSSampler)
        prevPSSampler->Release();

    // シェーダ
    if (prevVS) { context->VSSetShader(prevVS, nullptr, 0); prevVS->Release(); }
    if (prevPS) { context->PSSetShader(prevPS, nullptr, 0); prevPS->Release(); }

    // InputLayout
    if (prevInputLayout)
    {
        context->IASetInputLayout(prevInputLayout);
        prevInputLayout->Release();
    }

    // トポロジ
    context->IASetPrimitiveTopology(prevTopology);
}

void SceneManager::DrawLoadingIndicator()
{
	m_BillboardLoad->Draw();
    m_BillboardCowIcon->Draw();
}

void SceneManager::DisposeTransitionResources()
{
    if (m_transitionVertexBuffer) {
        m_transitionVertexBuffer->Release();
        m_transitionVertexBuffer = nullptr;
    }
    if (m_transitionConstantBuffer) {
        m_transitionConstantBuffer->Release();
        m_transitionConstantBuffer = nullptr;
    }
    if (m_transitionTexture) {
        m_transitionTexture->Release();
        m_transitionTexture = nullptr;
    }
    if (m_transitionDepthState) {
        m_transitionDepthState->Release();
        m_transitionDepthState = nullptr;
    }
    if (m_transitionBlendState) {
        m_transitionBlendState->Release();
        m_transitionBlendState = nullptr;
    }
    if (m_transitionSamplerState) {
        m_transitionSamplerState->Release();
        m_transitionSamplerState = nullptr;
    }
}

void SceneManager::Dispose()
{
    // トランジションリソースの解放
    DisposeTransitionResources();

    // シーンの破棄
    for (auto& scene : m_scenes) {
        if (scene.second) {
            scene.second->dispose();
        }
    }
    m_scenes.clear();

    m_currentSceneName.clear();
    m_nextSceneName.clear();
    m_sceneChangeRequested = false;
    m_transitionState = TransitionState::None;
    m_slideOffset = 0.0f;
    m_sceneLoaded = false;
    m_loadingRotation = 0.0f;
}

void SceneManager::SetCurrentScene(const std::string& sceneName)
{
    if (m_scenes.find(sceneName) == m_scenes.end()) {
        throw std::runtime_error("Scene not found: " + sceneName);
    }

    // 現在のシーンの終了処理
    if (!m_currentSceneName.empty() && m_scenes.find(m_currentSceneName) != m_scenes.end()) {
        m_scenes[m_currentSceneName]->dispose();
    }

    // 新しいシーンの設定
    m_currentSceneName = sceneName;

    // コンテキストをフラッシュ
    ID3D11DeviceContext* context = Renderer::GetDeviceContext();
    context->Flush();

    // 新しいシーンの初期化(非同期処理の導入にあたり、UpdateTransitionにInitを移行)
    m_scenes[m_currentSceneName]->init();
}

std::vector<std::string> SceneManager::GetRegisteredSceneNames()
{
    std::vector<std::string> names;
    for (const auto& scene : m_scenes) {
        names.push_back(scene.first);
    }
    return names;
}

bool SceneManager::IsSceneRegistered(const std::string& sceneName)
{
    return m_scenes.find(sceneName) != m_scenes.end();
}