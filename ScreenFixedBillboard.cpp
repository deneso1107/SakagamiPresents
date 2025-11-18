#include "ScreenFixedBillboard.h"
#include <WICTextureLoader.h>
#include "system/CShader.h"
#include <iostream>

static CShader g_Shader{};
ScreenFixedBillboard::ScreenFixedBillboard(const Vector2& screenPos, float width, float height, const wchar_t* texturePath)
    : m_screenPosition(screenPos)  // 画面中央
    , m_width(width)
    , m_height(height)
    , m_angle(0.0f)  // 角度を初期化
    , m_uvOffset(0.0f, 0.0f)
    , m_uvSize(1.0f, 1.0f)
    , m_texture(nullptr)
    , m_vertexBuffer(nullptr)
    , m_indexBuffer(nullptr)
{

    // テクスチャの読み込み
    HRESULT hr = DirectX::CreateWICTextureFromFile(
        Renderer::GetDevice(),
        texturePath,
        nullptr,
        &m_texture
    );

    if (FAILED(hr)) {
        //CreateDefaultTexture();
    }
    // シェーダーの初期化
    g_Shader.Create2D(
        "shader/Simple2DTextureVS.hlsl",
        "shader/Simple2DTexturePS.hlsl");


    // マテリアル設定
    MATERIAL materialData = {};
    materialData.Diffuse = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
    materialData.Ambient = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
    materialData.Specular = Vector4(0.0f, 0.0f, 0.0f, 0.0f);
    materialData.Emission = Vector4(0.0f, 0.0f, 0.0f, 0.0f);
    materialData.Shiness = 0.0f;
    materialData.TextureEnable = 1.0f;

    m_Material.Create(materialData);
    CreateBuffers();
}

// 動画用コンストラクタ（VideoPlayerを渡す）
ScreenFixedBillboard::ScreenFixedBillboard(const Vector2& screenPos, float width, float height, VideoPlayer* videoPlayer, bool takeOwnership)
    : m_screenPosition(screenPos)
    , m_width(width)
    , m_height(height)
    , m_angle(0.0f)
    , m_uvOffset(0.0f, 0.0f)
    , m_uvSize(1.0f, 1.0f)
    , m_texture(nullptr)
    , m_vertexBuffer(nullptr)
    , m_indexBuffer(nullptr)
    , m_videoPlayer(videoPlayer)
    , m_isOwningVideoPlayer(takeOwnership)
{
    // VideoPlayerからSRVを取得
    if (m_videoPlayer) {
        m_texture = m_videoPlayer->GetTextureSRV();
        if (m_texture) {
            m_texture->AddRef();
        }
    }

    // シェーダーの初期化
    g_Shader.Create2D(
        "shader/Simple2DTextureVS.hlsl",
        "shader/Simple2DTexturePS.hlsl");


    // マテリアル設定
    MATERIAL materialData = {};
    materialData.Diffuse = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
    materialData.Ambient = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
    materialData.Specular = Vector4(0.0f, 0.0f, 0.0f, 0.0f);
    materialData.Emission = Vector4(0.0f, 0.0f, 0.0f, 0.0f);
    materialData.Shiness = 0.0f;
    materialData.TextureEnable = 1.0f;

    m_Material.Create(materialData);
    CreateBuffers();
}

void ScreenFixedBillboard::Init(const Vector2& screenPos, float width, float height, const wchar_t* texturePath)
{
}

void ScreenFixedBillboard::CreateBuffers()
{
    UpdateVertexBuffer();

    // インデックスデータ
    WORD indices[] = { 0, 1, 2, 0, 2, 3 };

    D3D11_BUFFER_DESC bd = {};
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(WORD) * 6;
    bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    bd.CPUAccessFlags = 0;

    D3D11_SUBRESOURCE_DATA InitData = {};
    InitData.pSysMem = indices;
    Renderer::GetDevice()->CreateBuffer(&bd, &InitData, &m_indexBuffer);
}

void ScreenFixedBillboard::UpdateVertexBuffer()
{
    // スクリーン座標を正規化デバイス座標（NDC）に変換
    // スクリーン座標 (0,0) = 左上、(1,1) = 右下
    // NDC座標 (-1,1) = 左上、(1,-1) = 右下
 // スクリーン座標を正規化デバイス座標（NDC）に変換
    float centerX = (m_screenPosition.x * 2.0f) - 1.0f;
    float centerY = 1.0f - (m_screenPosition.y * 2.0f);

    float halfWidth = m_width;
    float halfHeight = m_height;

    // 回転前の4つの頂点座標
    Vector2 vertices2D[4] = {
        Vector2(centerX - halfWidth, centerY - halfHeight), // 左下
        Vector2(centerX - halfWidth, centerY + halfHeight), // 左上
        Vector2(centerX + halfWidth, centerY + halfHeight), // 右上
        Vector2(centerX + halfWidth, centerY - halfHeight)  // 右下
    };

    // 中心点
    Vector2 center(centerX, centerY);

    // 各頂点を回転
    if (m_angle != 0.0f) {
        for (int i = 0; i < 4; i++) {
            vertices2D[i] = RotatePoint(vertices2D[i], center, m_angle);
        }
    }

    // テクスチャ座標の計算を変更
    float u1 = m_uvOffset.x;
    float v1 = m_uvOffset.y;
    float u2 = m_uvOffset.x + m_uvSize.x;
    float v2 = m_uvOffset.y + m_uvSize.y;

    // 3D頂点データを作成
    ScreenBillboardVertex vertices[4] = {
        // 左下
         { Vector3(vertices2D[0].x, vertices2D[0].y, 0.0f), Vector2(u1, v2) },
         // 左上  
         { Vector3(vertices2D[1].x, vertices2D[1].y, 0.0f), Vector2(u1, v1) },
         // 右上
         { Vector3(vertices2D[2].x, vertices2D[2].y, 0.0f), Vector2(u2, v1) },
         // 右下
         { Vector3(vertices2D[3].x, vertices2D[3].y, 0.0f), Vector2(u2, v2) }
    };

    // 頂点バッファを作成または更新
    if (m_vertexBuffer == nullptr) {
        D3D11_BUFFER_DESC bd = {};
        bd.Usage = D3D11_USAGE_DYNAMIC;
        bd.ByteWidth = sizeof(ScreenBillboardVertex) * 4;
        bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

        D3D11_SUBRESOURCE_DATA InitData = {};
        InitData.pSysMem = vertices;
        Renderer::GetDevice()->CreateBuffer(&bd, &InitData, &m_vertexBuffer);
    }
    else {
        D3D11_MAPPED_SUBRESOURCE mappedResource;
        HRESULT hr = Renderer::GetDeviceContext()->Map(m_vertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
        if (SUCCEEDED(hr)) {
            memcpy(mappedResource.pData, vertices, sizeof(vertices));
            Renderer::GetDeviceContext()->Unmap(m_vertexBuffer, 0);
        }
    }
}

void ScreenFixedBillboard::SetUVRange(float u1, float v1, float u2, float v2)
{
    m_uvOffset = Vector2(u1, v1);
    m_uvSize = Vector2(u2 - u1, v2 - v1);
    UpdateVertexBuffer();  // UV変更時に頂点バッファを更新
}

Vector2 ScreenFixedBillboard::RotatePoint(const Vector2& point, const Vector2& center, float angle)
{
    // 度数法をラジアンに変換
    float radians = angle * (3.14159265f / 180.0f);

    float cosAngle = cos(radians);
    float sinAngle = sin(radians);

    // 中心を原点とする座標に変換
    float x = point.x - center.x;
    float y = point.y - center.y;

    // 回転行列を適用
    float rotatedX = x * cosAngle - y * sinAngle;
    float rotatedY = x * sinAngle + y * cosAngle;

    // 中心座標を戻す
    return Vector2(rotatedX + center.x, rotatedY + center.y);
}

void ScreenFixedBillboard::Update()
{
    // 動画の場合、フレームを更新
    if (m_videoPlayer) {
        m_videoPlayer->Update(0.016f);  // 約60FPS
    }
    // 位置やサイズが変更された場合のみ更新
    UpdateVertexBuffer();
}

void ScreenFixedBillboard::Draw()
{
    ID3D11ShaderResourceView* currentSRV = nullptr;
    Renderer::GetDeviceContext()->PSGetShaderResources(0, 1, &currentSRV);

    if (currentSRV) {
        OutputDebugStringA("SRVはバインドされています\n");
        currentSRV->Release();
    }
    else {
        OutputDebugStringA("SRVがバインドされていません\n");
    }
    // ワールド行列とビュー行列を単位行列に設定
    // スクリーン座標系で直接描画するため
    Matrix4x4 identity = DirectX::XMMatrixIdentity();
    Renderer::SetWorldMatrix(&identity);
    Renderer::SetViewMatrix(&identity);
    // シェーダーとマテリアル設定
// 注意：位置変換なしのシンプルなシェーダーが必要
    g_Shader.SetGPU();
    m_Material.SetGPU();

    // 正射影行列を設定（パースペクティブ無効化）
    Matrix4x4 orthoMatrix = DirectX::XMMatrixOrthographicLH(2.0f, 2.0f, 0.0f, 1.0f);
    Renderer::SetProjectionMatrix(&orthoMatrix);


    if (m_texture) {
        Renderer::GetDeviceContext()->PSSetShaderResources(0, 1, &m_texture);

        ID3D11SamplerState* samplerState = nullptr;
        D3D11_SAMPLER_DESC samplerDesc = {};
        samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
        samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
        samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
        samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
        samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
        samplerDesc.MinLOD = 0;
        samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

        HRESULT hr = Renderer::GetDevice()->CreateSamplerState(&samplerDesc, &samplerState);
        if (SUCCEEDED(hr)) {
            Renderer::GetDeviceContext()->PSSetSamplers(0, 1, &samplerState);
            samplerState->Release();
        }
    }

    // 頂点バッファ設定
    UINT stride = sizeof(ScreenBillboardVertex);
    UINT offset = 0;
    Renderer::GetDeviceContext()->IASetVertexBuffers(0, 1, &m_vertexBuffer, &stride, &offset);
    Renderer::GetDeviceContext()->IASetIndexBuffer(m_indexBuffer, DXGI_FORMAT_R16_UINT, 0);
    Renderer::GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    // 描画
    Renderer::GetDeviceContext()->DrawIndexed(6, 0, 0);
}

//動画用
// 動画用ファクトリメソッド
ScreenFixedBillboard* ScreenFixedBillboard::CreateFromVideo(const Vector2& screenPos, float width, float height, const wchar_t* videoPath)
{
    // VideoPlayerを作成
    VideoPlayer* player = new VideoPlayer();
    player->Initialize(Renderer::GetDevice(), Renderer::GetDeviceContext());

    HRESULT hr = player->LoadVideo(videoPath);
    if (FAILED(hr)) {
        delete player;
        return nullptr;
    }

    // Billboardを作成（所有権を渡す）
    return new ScreenFixedBillboard(screenPos, width, height, player, true);
}

void ScreenFixedBillboard::PlayVideo()
{
    if (m_videoPlayer) {
        m_videoPlayer->Play();
    }
}

void ScreenFixedBillboard::PauseVideo()
{
    if (m_videoPlayer) {
        m_videoPlayer->Pause();
    }
}

void ScreenFixedBillboard::StopVideo()
{
    if (m_videoPlayer) {
        m_videoPlayer->Stop();
    }
}

void ScreenFixedBillboard::SetLooping(bool loop)
{
    if (m_videoPlayer) {
        m_videoPlayer->SetLooping(loop);
    }
}

//bool ScreenFixedBillboard::IsPlaying() const
//{
//    return m_videoPlayer ? m_videoPlayer->IsPlaying() : false;
//}

float ScreenFixedBillboard::GetCurrentTime() const
{
    return m_videoPlayer ? m_videoPlayer->GetCurrentTimeSeconds() : 0.0f;
}

float ScreenFixedBillboard::GetDuration() const
{
    return m_videoPlayer ? m_videoPlayer->GetDurationSeconds() : 0.0f;
}
