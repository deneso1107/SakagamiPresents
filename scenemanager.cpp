#include	"system/IScene.h"
#include	"scenemanager.h"
#include	"CarDriveScene.h"
#include	"Title.h"
#include"Ending.h"
#include <algorithm>
#include <iostream>
// 静的メンバの初期化
std::unordered_map<std::string, std::unique_ptr<IScene>> SceneManager::m_scenes;
std::string SceneManager::m_currentSceneName = "";
std::string SceneManager::m_nextSceneName = "";
bool SceneManager::m_sceneChangeRequested = false;
//std::vector<std::unique_ptr<Object>> SceneManager::m_objects;
bool SceneManager::m_fadeInProgress = false;
float SceneManager::m_fadeAlpha = 0.0f;
float SceneManager::m_fadeSpeed = 2.0f;

int m_gameScore = 0;

CShader SceneManager::m_fadeShader;
ID3D11Buffer* SceneManager::m_fadeVertexBuffer = nullptr;
ID3D11Buffer* SceneManager::m_fadeConstantBuffer = nullptr;
void SceneManager::Init()
{
    // 初期シーンの登録（一番最後に登録したSceneが最初に出てくる）
    //RegisterScene<Ending>("Ending");
    RegisterScene<Title>("Title");
    RegisterScene<CarDriveScene>("CarDriveScene");

    // フェードリソースの初期化
    InitFadeResources();

    // 初期シーンの設定(最初にInitやってくれる)
    if (!m_scenes.empty())
    {
        m_currentSceneName = m_scenes.begin()->first;
        m_scenes[m_currentSceneName]->init();
    }
}

void SceneManager::InitFadeResources()
{
    // フェード用シェーダーの作成
    m_fadeShader.Create2D("shader/FadeVS.hlsl", "shader/FadePS.hlsl");
    std::wcout << L"Shader created successfully" << std::endl;
    // フルスクリーン矩形の頂点データ
    FadeVertex vertices[4] = {
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

    HRESULT hr = Renderer::GetDevice()->CreateBuffer(&bufferDesc, &initData, &m_fadeVertexBuffer);
    if (FAILED(hr)) {
        printf("ERROR: Failed to create fade vertex buffer!\n");
    }

    // 定数バッファの作成
    D3D11_BUFFER_DESC cbDesc = {};
    cbDesc.ByteWidth = sizeof(FadeConstantBuffer);
    cbDesc.Usage = D3D11_USAGE_DYNAMIC;
    cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    hr = Renderer::GetDevice()->CreateBuffer(&cbDesc, nullptr, &m_fadeConstantBuffer);
    if (FAILED(hr)) {
        printf("ERROR: Failed to create fade constant buffer!\n");
    }
}

void SceneManager::Update(float deltaTime) 
{
    // フェード処理の更新
    UpdateFade(deltaTime);

    // シーン切り替え処理
    ProcessSceneChange();

    // 現在のシーンの更新
    if (!m_currentSceneName.empty() && m_scenes.find(m_currentSceneName) != m_scenes.end()) {
        m_scenes[m_currentSceneName]->update(deltaTime);
    }
}

void SceneManager::Draw(float deltaTime) {
    // 現在のシーンの描画
    if (!m_currentSceneName.empty() && m_scenes.find(m_currentSceneName) != m_scenes.end()) {
        m_scenes[m_currentSceneName]->draw(deltaTime);
    }

    // フェード描画（最前面）
    if (m_fadeInProgress && m_fadeAlpha > 0.0f) {
        DrawFadeOverlay();
    }
}

void SceneManager::DisposeFadeResources() {
    if (m_fadeVertexBuffer) {
        m_fadeVertexBuffer->Release();
        m_fadeVertexBuffer = nullptr;
    }
    if (m_fadeConstantBuffer) {
        m_fadeConstantBuffer->Release();
        m_fadeConstantBuffer = nullptr;
    }
}

void SceneManager::Dispose() {
    // フェードリソースの解放
    DisposeFadeResources();

    // 既存の終了処理...
    for (auto& scene : m_scenes) {
        if (scene.second) {
            scene.second->dispose();
        }
    }
    m_scenes.clear();

    m_currentSceneName.clear();
    m_nextSceneName.clear();
    m_sceneChangeRequested = false;
    m_fadeInProgress = false;
    m_fadeAlpha = 0.0f;
}

void SceneManager::SetCurrentScene(const std::string& sceneName) {
    if (m_scenes.find(sceneName) == m_scenes.end()) {
        throw std::runtime_error("Scene not found: " + sceneName);
    }

    // 現在のシーンの終了処理
    if (!m_currentSceneName.empty() && m_scenes.find(m_currentSceneName) != m_scenes.end()) {
        m_scenes[m_currentSceneName]->dispose();
    }

    // 新しいシーンの設定
    m_currentSceneName = sceneName;

    // コンテキストをフラッシュするだけ（RTVはnullにしない）
    ID3D11DeviceContext* context = Renderer::GetDeviceContext();
    context->Flush();

    // 新しいシーンの初期化
    m_scenes[m_currentSceneName]->init();
}

void SceneManager::ChangeScene(const std::string& sceneName, bool useFade) {
    if (m_scenes.find(sceneName) == m_scenes.end()) {
        throw std::runtime_error("Scene not found: " + sceneName);
    }

    if (m_sceneChangeRequested) {
        return; // 既にシーン変更中
    }

    m_nextSceneName = sceneName;
    m_sceneChangeRequested = true;

    //if (useFade) {
        m_fadeInProgress = true;
        m_fadeAlpha = 0.0f;
   // }
}

void SceneManager::ProcessSceneChange() {
    if (!m_sceneChangeRequested) return;

    // フェードを使用する場合
    if (m_fadeInProgress) {
        if (m_fadeAlpha < 1.0f) {
            return; // フェードイン完了待ち
        }

        // フェードイン完了、シーン切り替え実行
         SetCurrentScene(m_nextSceneName);

        // フェードアウト開始
        m_fadeAlpha = 1.0f;
        m_sceneChangeRequested = false;
    }
    else {
        // フェードなしの場合、即座に切り替え
        SetCurrentScene(m_nextSceneName);
        m_sceneChangeRequested = false;
    }
}

void SceneManager::UpdateFade(float deltaTime) {
    if (!m_fadeInProgress) return;

    if (m_sceneChangeRequested) {
        // フェードイン
        m_fadeAlpha += m_fadeSpeed * deltaTime;
        if (m_fadeAlpha >= 1.0f) {
            m_fadeAlpha = 1.0f;
        }
    }
    else {
        // フェードアウト
        m_fadeAlpha -= m_fadeSpeed * deltaTime;
        if (m_fadeAlpha <= 0.0f) {
            m_fadeAlpha = 0.0f;
            m_fadeInProgress = false;
        }
    }
}

void SceneManager::DrawFadeOverlay()
{
    if (!m_fadeVertexBuffer || !m_fadeConstantBuffer) {
        return;
    }

    ID3D11DeviceContext* context = Renderer::GetDeviceContext();

    // 深度テストを無効化
    ID3D11DepthStencilState* prevDepthState;
    UINT prevStencilRef;
    context->OMGetDepthStencilState(&prevDepthState, &prevStencilRef);

    D3D11_DEPTH_STENCIL_DESC depthDesc = {};
    depthDesc.DepthEnable = FALSE;
    depthDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
    depthDesc.DepthFunc = D3D11_COMPARISON_ALWAYS;
    depthDesc.StencilEnable = FALSE;

    ID3D11DepthStencilState* noDepthState = nullptr;
    Renderer::GetDevice()->CreateDepthStencilState(&depthDesc, &noDepthState);
    if (noDepthState) {
        context->OMSetDepthStencilState(noDepthState, 0);
    }

    // ブレンドステートを設定（アルファブレンド）
    ID3D11BlendState* prevBlendState;
    FLOAT prevBlendFactor[4];
    UINT prevSampleMask;
    context->OMGetBlendState(&prevBlendState, prevBlendFactor, &prevSampleMask);

    D3D11_BLEND_DESC blendDesc = {};
    blendDesc.RenderTarget[0].BlendEnable = TRUE;
    blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
    blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
    blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
    blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
    blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

    ID3D11BlendState* alphaBlendState = nullptr;
    Renderer::GetDevice()->CreateBlendState(&blendDesc, &alphaBlendState);
    if (alphaBlendState) {
        FLOAT blendFactor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
        context->OMSetBlendState(alphaBlendState, blendFactor, 0xffffffff);
    }

    // 定数バッファを更新
    D3D11_MAPPED_SUBRESOURCE mapped;
    HRESULT hr = context->Map(m_fadeConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
    if (SUCCEEDED(hr)) {
        FadeConstantBuffer* cb = static_cast<FadeConstantBuffer*>(mapped.pData);
        cb->fadeAlpha = m_fadeAlpha;
        cb->padding[0] = cb->padding[1] = cb->padding[2] = 0.0f;
        context->Unmap(m_fadeConstantBuffer, 0);
    }

    // シェーダーを設定
    m_fadeShader.SetGPU();

    // 定数バッファをピクセルシェーダーに設定
    context->PSSetConstantBuffers(0, 1, &m_fadeConstantBuffer);

    // 頂点バッファを設定
    UINT stride = sizeof(FadeVertex);
    UINT offset = 0;
    context->IASetVertexBuffers(0, 1, &m_fadeVertexBuffer, &stride, &offset);
    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

    // 描画
    context->Draw(4, 0);

    // ステートを復元
    if (prevDepthState) {
        context->OMSetDepthStencilState(prevDepthState, prevStencilRef);
        prevDepthState->Release();
    }
    if (noDepthState) {
        noDepthState->Release();
    }
    if (prevBlendState) {
        context->OMSetBlendState(prevBlendState, prevBlendFactor, prevSampleMask);
        prevBlendState->Release();
    }
    if (alphaBlendState) {
        alphaBlendState->Release();
    }
}

std::vector<std::string> SceneManager::GetRegisteredSceneNames() {
    std::vector<std::string> names;
    for (const auto& scene : m_scenes) {
        names.push_back(scene.first);
    }
    return names;
}

bool SceneManager::IsSceneRegistered(const std::string& sceneName) {
    return m_scenes.find(sceneName) != m_scenes.end();
}


//std::unordered_map<std::string, std::unique_ptr<IScene>> SceneManager::m_scenes;
//std::string SceneManager::m_currentSceneName = "";
//
//// 登録されているシーンを全て破棄する
//void SceneManager::Dispose() 
//{
//	// 登録されているすべてシーンの終了処理
//	for (auto& s : m_scenes) 
//	{
//		s.second->dispose();
//	}
//
//	m_scenes.clear();
//	m_currentSceneName.clear();
//}
//
//void SceneManager::SetCurrentScene(std::string current) //うんち！
//{
//	// 現在のシーンを設定
//	if (m_scenes.find(current) != m_scenes.end()) 
//	{
//		m_currentSceneName = current;
//	}
//	else 
//	{
//		throw std::runtime_error("Scene not found: " + current);
//	}
//}
//	
//void SceneManager::Init()
//{
//	m_scenes["CarDriveScene"] = std::make_unique<CarDriveScene>();
//	m_scenes["CarDriveScene"]->init();
//	m_currentSceneName = "CarDriveScene";
//}
//
//void SceneManager::Draw(uint64_t deltatime)
//{
//
//	// 現在のシーンを描画
//	m_scenes[m_currentSceneName]->draw(deltatime);
//}
//
//void SceneManager::Update(uint64_t deltatime)
//{
//	// 現在のシーンを更新
//	m_scenes[m_currentSceneName]->update(deltatime);
//}