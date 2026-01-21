#include "ScreenGaugeBillboard.h"
#include <WICTextureLoader.h>
#include "system/CShader.h"
#include <algorithm>
#include <iostream>

static CShader g_Shader{};

ScreenGaugeBillboard::ScreenGaugeBillboard()
    : m_screenPosition(0.5f, 0.5f)
    , m_width(0.15f)
    , m_height(0.025f)
    , m_frameTexture(nullptr)
    , m_fillTexture(nullptr)
    , m_textTexture(nullptr)
    , m_frameVertexBuffer(nullptr)
    , m_fillVertexBuffer(nullptr)
    , m_textVertexBuffer(nullptr)
    , m_indexBuffer(nullptr)
    , m_currentValue(1.0f)
    , m_targetValue(1.0f)
    , m_animationSpeed(2.0f)  // 2秒で完全に変化
    , m_fillMargin(0.05f, 0.05f)
    , m_tintColor(1.0f, 1.0f, 1.0f, 1.0f)
{
}

ScreenGaugeBillboard::~ScreenGaugeBillboard()
{
    if (m_frameTexture) {
        m_frameTexture->Release();
        m_frameTexture = nullptr;
    }
    if (m_fillTexture) {
        m_fillTexture->Release();
        m_fillTexture = nullptr;
    }
    if (m_textTexture) {
        m_textTexture->Release();
        m_textTexture = nullptr;
    }
    if (m_frameVertexBuffer) {
        m_frameVertexBuffer->Release();
        m_frameVertexBuffer = nullptr;
    }
    if (m_fillVertexBuffer) {
        m_fillVertexBuffer->Release();
        m_fillVertexBuffer = nullptr;
    }
    if (m_textVertexBuffer) {
        m_textVertexBuffer->Release();
        m_textVertexBuffer = nullptr;
    }
}

bool ScreenGaugeBillboard::Init(const Vector2& screenPos, float width, float height,
    const wchar_t* frameTexturePath, const wchar_t* fillTexturePath,
    const Vector2& fillMargin)
{
    m_screenPosition = screenPos;
    m_width = width;
    m_height = height;
    m_fillMargin = fillMargin;

    // 外枠テクスチャの読み込み
    HRESULT hr = DirectX::CreateWICTextureFromFile(
        Renderer::GetDevice(),
        frameTexturePath,
        nullptr,
        &m_frameTexture
    );
    if (FAILED(hr)) {
        return false;
    }
    // 内容テクスチャの読み込み
    hr = DirectX::CreateWICTextureFromFile(
        Renderer::GetDevice(),
        fillTexturePath,
        nullptr,
        &m_fillTexture
    );
    if (FAILED(hr)) {
        return false;
    }

        if (FAILED(hr)) {
            return false;
        }
    // 内容テクスチャの読み込み
    hr = DirectX::CreateWICTextureFromFile(
        Renderer::GetDevice(),
        L"assets/texture/Gauge/EnergyBackGround.png",
        nullptr,
        &m_textTexture
    );
    if (FAILED(hr)) {
        return false;
    }
    // シェーダーの初期化
    g_Shader.Create2D(
        "shader/Simple2DTextureVS.hlsl",
        "shader/Simple2DTexturePS.hlsl");
    std::wcout << L"Shader created successfully" << std::endl;

    // マテリアルの初期化
    MATERIAL materialData = {};
    materialData.Diffuse = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
    materialData.Ambient = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
    materialData.Specular = Vector4(0.0f, 0.0f, 0.0f, 0.0f);
    materialData.Emission = Vector4(0.0f, 0.0f, 0.0f, 0.0f);
    materialData.Shiness = 0.0f;
    materialData.TextureEnable = 1.0f;

    if (!m_material.Create(materialData)) {
        return false;
    }
    CreateBuffers();
    return true;
}

void ScreenGaugeBillboard::CreateBuffers()
{
    // インデックスバッファ（共通）
    WORD indices[] = { 0, 1, 2, 0, 2, 3 };

    D3D11_BUFFER_DESC bd = {};
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(WORD) * 6;
    bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    bd.CPUAccessFlags = 0;

    D3D11_SUBRESOURCE_DATA InitData = {};
    InitData.pSysMem = indices;
    Renderer::GetDevice()->CreateBuffer(&bd, &InitData, &m_indexBuffer);

    UpdateFrameVertexBuffer();
    UpdateFillVertexBuffer();
    UpdateTextVertexBuffer();
}

void ScreenGaugeBillboard::UpdateFrameVertexBuffer()
{
    // 外枠は常に全体を表示
    float centerX = (m_screenPosition.x * 2.0f) - 1.0f;
    float centerY = 1.0f - (m_screenPosition.y * 2.0f);

    float halfWidth = m_width;
    float halfHeight = m_height*1.25;

    ScreenBillboardVertex vertices[4] = {
        { Vector3(centerX - halfWidth, centerY - halfHeight, 0.0f), Vector2(0.0f, 1.0f) },
        { Vector3(centerX - halfWidth, centerY + halfHeight, 0.0f), Vector2(0.0f, 0.0f) },
        { Vector3(centerX + halfWidth, centerY + halfHeight, 0.0f), Vector2(1.0f, 0.0f) },
        { Vector3(centerX + halfWidth, centerY - halfHeight, 0.0f), Vector2(1.0f, 1.0f) }
    };

    if (m_frameVertexBuffer == nullptr) {
        D3D11_BUFFER_DESC bd = {};
        bd.Usage = D3D11_USAGE_DYNAMIC;
        bd.ByteWidth = sizeof(ScreenBillboardVertex) * 4;
        bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

        D3D11_SUBRESOURCE_DATA InitData = {};
        InitData.pSysMem = vertices;
        Renderer::GetDevice()->CreateBuffer(&bd, &InitData, &m_frameVertexBuffer);
    }
    else {
        D3D11_MAPPED_SUBRESOURCE mappedResource;
        if (SUCCEEDED(Renderer::GetDeviceContext()->Map(m_frameVertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource))) {
            memcpy(mappedResource.pData, vertices, sizeof(vertices));
            Renderer::GetDeviceContext()->Unmap(m_frameVertexBuffer, 0);
        }
    }
}

void ScreenGaugeBillboard::UpdateTextVertexBuffer()
{
    // 外枠は常に全体を表示(マジックナンバーで位置と場所を調整)
    float centerX = (m_screenPosition.x * 2.0f) - 1.0f;
    float centerY = 1.0f - (m_screenPosition.y * 1.75f);

    float halfWidth = m_width*1.1f;
    float halfHeight = m_height*2.3f;

    ScreenBillboardVertex vertices[4] = {
        { Vector3(centerX - halfWidth, centerY - halfHeight, 0.0f), Vector2(0.0f, 1.0f) },
        { Vector3(centerX - halfWidth, centerY + halfHeight, 0.0f), Vector2(0.0f, 0.0f) },
        { Vector3(centerX + halfWidth, centerY + halfHeight, 0.0f), Vector2(1.0f, 0.0f) },
        { Vector3(centerX + halfWidth, centerY - halfHeight, 0.0f), Vector2(1.0f, 1.0f) }
    };

    if (m_textVertexBuffer == nullptr) {
        D3D11_BUFFER_DESC bd = {};
        bd.Usage = D3D11_USAGE_DYNAMIC;
        bd.ByteWidth = sizeof(ScreenBillboardVertex) * 4;
        bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

        D3D11_SUBRESOURCE_DATA InitData = {};
        InitData.pSysMem = vertices;
        Renderer::GetDevice()->CreateBuffer(&bd, &InitData, &m_textVertexBuffer);
    }
    else {
        D3D11_MAPPED_SUBRESOURCE mappedResource;
        if (SUCCEEDED(Renderer::GetDeviceContext()->Map(m_textVertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource))) {
            memcpy(mappedResource.pData, vertices, sizeof(vertices));
            Renderer::GetDeviceContext()->Unmap(m_textVertexBuffer, 0);
        }
    }
}

void ScreenGaugeBillboard::UpdateFillVertexBuffer()
{
    float centerX = (m_screenPosition.x * 2.0f) - 1.0f;
    float centerY = 1.0f - (m_screenPosition.y * 2.0f);

    // 内容は余白を考慮してサイズを調整
    float fillWidth = m_width - m_fillMargin.x;
    float fillHeight = m_height - m_fillMargin.y;

    // ゲージ値に基づいて幅を調整
    float currentFillWidth = fillWidth * m_currentValue;

    // 修正：座標計算
    float leftX = centerX - fillWidth;
    float rightX = leftX + currentFillWidth*2;  // *2.0fを削除
    float topY = centerY + fillHeight;
    float bottomY = centerY - fillHeight;

    float uvRight = m_currentValue;

    ScreenBillboardVertex vertices[4] = {
        { Vector3(leftX,  bottomY, 0.0f), Vector2(0.0f,   1.0f) },
        { Vector3(leftX,  topY,    0.0f), Vector2(0.0f,   0.0f) },
        { Vector3(rightX, topY,    0.0f), Vector2(uvRight, 0.0f) },
        { Vector3(rightX, bottomY, 0.0f), Vector2(uvRight, 1.0f) }
    };

    if (m_fillVertexBuffer == nullptr) {
        // 初回作成
        D3D11_BUFFER_DESC bd = {};
        bd.Usage = D3D11_USAGE_DYNAMIC;
        bd.ByteWidth = sizeof(ScreenBillboardVertex) * 4;
        bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

        D3D11_SUBRESOURCE_DATA InitData = {};
        InitData.pSysMem = vertices;
        HRESULT hr = Renderer::GetDevice()->CreateBuffer(&bd, &InitData, &m_fillVertexBuffer);

        if (FAILED(hr)) {
            std::wcout << L"Failed to create fill vertex buffer: 0x" << std::hex << hr << std::dec << std::endl;
        }
    }
    else {
        // 既存バッファの更新
        D3D11_MAPPED_SUBRESOURCE mappedResource;
        HRESULT hr = Renderer::GetDeviceContext()->Map(m_fillVertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
        if (SUCCEEDED(hr)) {
            memcpy(mappedResource.pData, vertices, sizeof(vertices));
            Renderer::GetDeviceContext()->Unmap(m_fillVertexBuffer, 0);
        }
        else {
            std::wcout << L"Failed to map fill vertex buffer: 0x" << std::hex << hr << std::dec << std::endl;
        }
    }
}

void ScreenGaugeBillboard::SetValue(float value, bool animate)
{
    // 0.0f から 1.0f の範囲にクランプ
    m_targetValue = std::max(0.0f, std::min(1.0f, value));

    if (!animate) 
    {
        m_currentValue = m_targetValue;
        UpdateFillVertexBuffer();
    }
}

void ScreenGaugeBillboard::Update(float deltaTime)
{
    // アニメーション処理
    if (abs(m_currentValue - m_targetValue) > 0.001f) {
        float step = m_animationSpeed * deltaTime;

        if (m_currentValue < m_targetValue) {
            m_currentValue = std::min(m_targetValue, m_currentValue + step);
        }
        else {
            m_currentValue = std::max(m_targetValue, m_currentValue - step);
        }

        UpdateFillVertexBuffer();

        // デバッグ出力
       /* std::wcout << L"Gauge Update - Current: " << m_currentValue
            << L", Target: " << m_targetValue << std::endl;*/
    }
}

void ScreenGaugeBillboard::SetupRenderState()
{

    // 行列設定（2D用の正投影）
    Matrix4x4 identity = DirectX::XMMatrixIdentity();
    Renderer::SetWorldMatrix(&identity);
    Renderer::SetViewMatrix(&identity);

    // 正投影行列（-1〜1の範囲で描画）
    Renderer::SetProjectionMatrix(&identity); // 正射影行列ではなく単位行列

    // シェーダーとマテリアル設定
    g_Shader.SetGPU();
    m_material.SetGPU();

    Renderer::GetDeviceContext()->PSSetShaderResources(0, 1, &m_fillTexture);
    Renderer::GetDeviceContext()->PSSetShaderResources(0, 1, &m_frameTexture);
    Renderer::GetDeviceContext()->PSSetShaderResources(0, 1, &m_textTexture);
    // サンプラー設定
    ID3D11SamplerState* samplerState = nullptr;
    D3D11_SAMPLER_DESC samplerDesc = {};
    samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;//D3D11_TEXTURE_ADDRESS_CLAMPにしたらバグる
    samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;//D3D11_TEXTURE_ADDRESS_CLAMPにしたらバグる
    samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;//D3D11_TEXTURE_ADDRESS_CLAMPにしたらバグる
    samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    samplerDesc.MinLOD = 0;
    samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

    HRESULT hr = Renderer::GetDevice()->CreateSamplerState(&samplerDesc, &samplerState);
    if (SUCCEEDED(hr)) {
        Renderer::GetDeviceContext()->PSSetSamplers(0, 1, &samplerState);
        samplerState->Release();
    }

    // ラスタライザーステートを2D用に設定
    ID3D11RasterizerState* rasterizerState = nullptr;
    D3D11_RASTERIZER_DESC rasterizerDesc = {};
    rasterizerDesc.FillMode = D3D11_FILL_SOLID;
    rasterizerDesc.CullMode = D3D11_CULL_NONE;  // カリングなし
    rasterizerDesc.FrontCounterClockwise = FALSE;
    rasterizerDesc.DepthBias = 0;
    rasterizerDesc.SlopeScaledDepthBias = 0.0f;
    rasterizerDesc.DepthBiasClamp = 0.0f;
    rasterizerDesc.DepthClipEnable = TRUE;
    rasterizerDesc.ScissorEnable = FALSE;
    rasterizerDesc.MultisampleEnable = FALSE;
    rasterizerDesc.AntialiasedLineEnable = FALSE;

    Renderer::GetDevice()->CreateRasterizerState(&rasterizerDesc, &rasterizerState);
    if (rasterizerState) {
        Renderer::GetDeviceContext()->RSSetState(rasterizerState);
        rasterizerState->Release();
    }
}


void ScreenGaugeBillboard::Draw()
{
    static int drawCount = 0;
    drawCount++;

    // 描画ステートを完全に設定
    SetupRenderState();

    ID3D11DeviceContext* context = Renderer::GetDeviceContext();
    UINT stride = sizeof(ScreenBillboardVertex);
    UINT offset = 0;


    // 文字の描画（必要に応じて）
    if (m_textTexture && m_textVertexBuffer) {
        context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        context->PSSetShaderResources(0, 1, &m_textTexture);
        context->IASetVertexBuffers(0, 1, &m_textVertexBuffer, &stride, &offset);
        context->IASetIndexBuffer(m_indexBuffer, DXGI_FORMAT_R16_UINT, 0);
        context->DrawIndexed(6, 0, 0);
    }

    // 外枠の描画（必要に応じて）
    if (m_frameTexture && m_frameVertexBuffer) {
        context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        context->PSSetShaderResources(0, 1, &m_frameTexture);
        context->IASetVertexBuffers(0, 1, &m_frameVertexBuffer, &stride, &offset);
        context->IASetIndexBuffer(m_indexBuffer, DXGI_FORMAT_R16_UINT, 0);
        context->DrawIndexed(6, 0, 0);
    }

    // 内容（ゲージ）を描画
    if (m_fillTexture && m_fillVertexBuffer) 
    {
        if (drawCount % 100 == 0)
        {
            //std::wcout << L"Drawing fill texture..." << std::endl;
        }
        // プリミティブトポロジーを再度確認・設定
        context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        context->PSSetShaderResources(0, 1, &m_fillTexture);
        context->IASetVertexBuffers(0, 1, &m_fillVertexBuffer, &stride, &offset);
        context->IASetIndexBuffer(m_indexBuffer, DXGI_FORMAT_R16_UINT, 0);
        context->DrawIndexed(6, 0, 0);
    }

    CleanupRenderState();
}

void ScreenGaugeBillboard::CleanupRenderState()
{
    // 必要に応じてレンダーステートのクリーンアップ
}