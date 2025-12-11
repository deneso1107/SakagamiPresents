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
float SceneManager::m_fadeAlpha = 0.0f; // ★追加

CShader SceneManager::m_transitionShader;
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

    // ★深度ステンシルステートの作成（キャッシュ）
    D3D11_DEPTH_STENCIL_DESC depthDesc = {};
    depthDesc.DepthEnable = FALSE;
    depthDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
    depthDesc.DepthFunc = D3D11_COMPARISON_ALWAYS;
    depthDesc.StencilEnable = FALSE;

    hr = Renderer::GetDevice()->CreateDepthStencilState(&depthDesc, &m_transitionDepthState);
    if (FAILED(hr)) {
        printf("ERROR: Failed to create transition depth state!\n");
    }

    // ★ブレンドステートの作成（キャッシュ）
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

    // ★サンプラーステートの作成（キャッシュ）
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
    if (!m_currentSceneName.empty() && m_scenes.find(m_currentSceneName) != m_scenes.end()) {
        m_scenes[m_currentSceneName]->update(deltaTime);
    }
}

void SceneManager::Draw(float deltaTime)
{


    // 現在のシーンの描画
    if (!m_currentSceneName.empty() && m_scenes.find(m_currentSceneName) != m_scenes.end()) {
        m_scenes[m_currentSceneName]->draw(deltaTime);
    }

    // トランジション描画（最前面）
    if (m_transitionState != TransitionState::None) {
        // ★1. まず黒背景フェード
        if (m_fadeAlpha > 0.0f) {
            //DrawBlackFade();
        }
        DrawTransitionOverlay();

        // ローディング中の表示
        if (m_transitionState == TransitionState::Loading) {
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

        // ★背景フェードイン (0.0 → 1.0)
        m_fadeAlpha += m_transitionSpeed * dt;
        if (m_fadeAlpha > 1.0f) m_fadeAlpha = 1.0f;

        if (m_slideOffset <= 0.0f) {
            m_slideOffset = 0.0f;
            m_fadeAlpha = 1.0f; // 完全に黒
            // 中央到達、ロード開始
            m_transitionState = TransitionState::Loading;
            m_sceneLoaded = false;
            m_loadingRotation = 0.0f;
            LoadNextSceneAsync();
        }
        break;

    case TransitionState::Loading:
        // ローディングアニメーション（上下に揺れる）
        m_loadingRotation += dt;

        // ★背景は完全に黒のまま
        m_fadeAlpha = 1.0f;

        // ロード完了チェック
        if (m_sceneLoaded) {
            // スライドアウト開始
            m_transitionState = TransitionState::SlideOut;
            m_sceneChangeRequested = false;
        }
        break;

    case TransitionState::SlideOut:
        // 中央から左へ (0.0 → -1.5)
        m_slideOffset -= m_transitionSpeed * dt;

        // ★背景フェードアウト (1.0 → 0.0)
        m_fadeAlpha -= m_transitionSpeed * dt;
        if (m_fadeAlpha < 0.0f) m_fadeAlpha = 0.0f;

        if (m_slideOffset <= -1.5f) {
            m_slideOffset = -1.5f;
            m_fadeAlpha = 0.0f; // 完全に透明
            m_transitionState = TransitionState::None;
        }
        break;
    }
}

void SceneManager::LoadNextSceneAsync()
{
    // 同期版：シンプルに実装（ローディング画面が表示されるので問題なし）

    // 旧シーンの破棄
    if (!m_currentSceneName.empty() &&
        m_scenes.find(m_currentSceneName) != m_scenes.end()) {
        m_scenes[m_currentSceneName]->dispose();
    }

    // コンテキストをフラッシュ
    ID3D11DeviceContext* context = Renderer::GetDeviceContext();
    context->Flush();

    // 新シーンの初期化（ここに時間がかかる）
    m_currentSceneName = m_nextSceneName;
    m_scenes[m_currentSceneName]->init();

    m_sceneLoaded = true; // ロード完了

    std::cout << "Scene loaded: " << m_currentSceneName << std::endl;
}

void SceneManager::ProcessSceneChange()
{
    // トランジション中の処理は UpdateTransition で行う
    if (!m_sceneChangeRequested) return;

    // トランジションなしの場合、即座に切り替え
    if (m_transitionState == TransitionState::None) {
        SetCurrentScene(m_nextSceneName);
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

void SceneManager::DrawTransitionOverlay()
{

    if (!m_transitionVertexBuffer || !m_transitionConstantBuffer)
        return;

    ID3D11DeviceContext* context = Renderer::GetDeviceContext();

    //
    // ============================
    // ★ ステートの完全保存
    // ============================
    //

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


    //
    // ============================
    // ★ トランジション用ステート設定
    // ============================
    //

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

            cb->padding = 0.0f;
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


    //
    // ============================
    // ★ ステート復元（最重要）
    // ============================
    //

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
    // オプション: ローディング中の追加表示
    // 例: "NOW LOADING..." テキストや追加エフェクト

    // ここにテキスト描画やエフェクトの処理を追加できます
    // DrawText("NOW LOADING...", 50, SCREEN_HEIGHT - 100, D3DCOLOR_XRGB(255, 255, 255));
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

    // 新しいシーンの初期化
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