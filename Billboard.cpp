#include "Billboard.h"
#include <WICTextureLoader.h>
#include"system/renderer.h" 
#include "system/CShader.h"

static CShader g_Shader{};


ComPtr<ID3D11ShaderResourceView> TextureCache:: GetTexture(ID3D11Device* device, const wchar_t* path)
{
    // ★ キャッシュを確認
    auto it = m_Cache.find(path);
    if (it != m_Cache.end(  ))
    {
        return it->second;  // キャッシュから返す
    }

    // 新規読み込み
    ComPtr<ID3D11ShaderResourceView> texture;
    HRESULT hr = DirectX::CreateWICTextureFromFile(
        device,
        path,
        nullptr,
        texture.GetAddressOf()
    );

    if (SUCCEEDED(hr))
    {
        m_Cache[path] = texture;

        char buffer[256];
        sprintf_s(buffer, "テクスチャキャッシュ登録: %ws\n", path);
        OutputDebugStringA(buffer);
    }
    else
    {
        OutputDebugStringA("テクスチャ読み込み失敗\n");
    }

    return texture;
}
Billboard::Billboard()
    : m_position(0.0f, 0.0f, 0.0f)
    , m_width(1.0f)
    , m_height(1.0f)
    ,m_IsInitialized(false)
{
}

Billboard::~Billboard() {
    Dispose();
}
void Billboard::Init(const Vector3& position, float width, float height, const wchar_t* texturePath) 
{
    if (m_IsInitialized)
    {
        Dispose();  // ★ 既に初期化されていたら一度クリア
    }
    m_position = position;
    m_width = width;
    m_height = height;

    m_texture = TextureCache::Instance().GetTexture(Renderer::GetDevice(), texturePath);


    //if (FAILED(hr)) {
    //    wprintf(L"Failed to load texture: %s (HRESULT: 0x%x)\n", texturePath, hr);
    //    CreateDefaultTexture();
    //}


    // シェーダーの初期化
    g_Shader.Create(
        "shader/unlitTextureVS.hlsl",
        "shader/unlitTexturePS.hlsl");

    // マテリアルを初期化
  // m_Material.Init();

    // マテリアル設定
    MATERIAL materialData = {};
    materialData.Diffuse = Vector4(1.0f, 1.0f, 1.0f, 1.0f);      // 白色
    materialData.Ambient = Vector4(1.0f, 1.0f, 1.0f, 1.0f);      // 白色
    materialData.Specular = Vector4(0.0f, 0.0f, 0.0f, 0.0f);     // スペキュラなし
    materialData.Emission = Vector4(0.0f, 0.0f, 0.0f, 0.0f);     // エミッションなし
    materialData.Shiness = 0.0f;
    materialData.TextureEnable = 1.0f;  // テクスチャ有効

    if (!m_Material.Create(materialData))  // ★ ここで初期化
    {
        OutputDebugStringA("マテリアル初期化失敗\n");
    }

    CreateBuffers();
    printf("Billboard initialized at position (%.2f, %.2f, %.2f) with size (%.2f, %.2f)\n",
		position.x, position.y, position.z, width, height);
    m_IsInitialized = true;
    m_NeedsUpdate = true;
}
void Billboard::CreateDefaultTexture() {
    // 2x2の白いテクスチャを作成
    UINT32 pixels[4] = { 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF };

    D3D11_TEXTURE2D_DESC desc = {};
    desc.Width = 2;
    desc.Height = 2;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.SampleDesc.Count = 1;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

    D3D11_SUBRESOURCE_DATA initData = {};
    initData.pSysMem = pixels;
    initData.SysMemPitch = sizeof(UINT32) * 2;

    ID3D11Texture2D* texture2D = nullptr;
    HRESULT hr = Renderer::GetDevice()->CreateTexture2D(&desc, &initData, &texture2D);

    if (SUCCEEDED(hr)) {
        D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
        srvDesc.Format = desc.Format;
        srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Texture2D.MipLevels = 1;

        Renderer::GetDevice()->CreateShaderResourceView(texture2D, &srvDesc, &m_texture);
        texture2D->Release();
    }
}
void Billboard::Dispose() {
    m_texture.Reset();
    m_vertexBuffer.Reset();
    m_indexBuffer.Reset();
}

    void Billboard::CreateBuffers()
    {
        ID3D11Device* device = Renderer::GetDevice();

        // インデックスデータ
        WORD indices[] = { 0, 1, 2, 0, 2, 3 };
        D3D11_BUFFER_DESC bd = {};
        bd.Usage = D3D11_USAGE_DEFAULT;
        bd.ByteWidth = sizeof(WORD) * 6;
        bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
        bd.CPUAccessFlags = 0;

        D3D11_SUBRESOURCE_DATA InitData = {};
        InitData.pSysMem = indices;

        device->CreateBuffer(&bd, &InitData, m_indexBuffer.GetAddressOf());

        // 頂点バッファ（クアッド用）
        BillboardVertex vertices[4] = {
            { Vector3(-m_width * 0.5f, -m_height * 0.5f, 0.0f), Vector3(0, 0, -1), Vector4(1, 1, 1, 1), Vector2(0, 1) },
            { Vector3(-m_width * 0.5f,  m_height * 0.5f, 0.0f), Vector3(0, 0, -1), Vector4(1, 1, 1, 1), Vector2(0, 0) },
            { Vector3(m_width * 0.5f,  m_height * 0.5f, 0.0f), Vector3(0, 0, -1), Vector4(1, 1, 1, 1), Vector2(1, 0) },
            { Vector3(m_width * 0.5f, -m_height * 0.5f, 0.0f), Vector3(0, 0, -1), Vector4(1, 1, 1, 1), Vector2(1, 1) }
        };

        D3D11_BUFFER_DESC vbDesc = {};
        vbDesc.Usage = D3D11_USAGE_DEFAULT;
        vbDesc.ByteWidth = sizeof(BillboardVertex) * 4;
        vbDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

        D3D11_SUBRESOURCE_DATA vbData = {};
        vbData.pSysMem = vertices;

        device->CreateBuffer(&vbDesc, &vbData, m_vertexBuffer.GetAddressOf());
        //// 白色のDiffuseに戻す
        //BillboardVertex vertices[4] = {
        //    { Vector3(-m_width * 0.5f, -m_height * 0.5f, 0.0f),
        //      Vector3(0.0f, 0.0f, -1.0f),
        //      Vector4(1.0f, 1.0f, 1.0f, 1.0f), // 白色に戻す
        //      Vector2(0.0f, 1.0f) },

        //    { Vector3(-m_width * 0.5f,  m_height * 0.5f, 0.0f),
        //      Vector3(0.0f, 0.0f, -1.0f),
        //      Vector4(1.0f, 1.0f, 1.0f, 1.0f), // 白色に戻す
        //      Vector2(0.0f, 0.0f) },

        //    { Vector3(m_width * 0.5f,  m_height * 0.5f, 0.0f),
        //      Vector3(0.0f, 0.0f, -1.0f),
        //      Vector4(1.0f, 1.0f, 1.0f, 1.0f), // 白色に戻す
        //      Vector2(1.0f, 0.0f) },

        //    { Vector3(m_width * 0.5f, -m_height * 0.5f, 0.0f),
        //      Vector3(0.0f, 0.0f, -1.0f),
        //      Vector4(1.0f, 1.0f, 1.0f, 1.0f), // 白色に戻す
        //      Vector2(1.0f, 1.0f) }
        //};

        //// 頂点バッファ作成
        //D3D11_BUFFER_DESC bd = {};
        //bd.Usage = D3D11_USAGE_DYNAMIC;
        //bd.ByteWidth = sizeof(BillboardVertex) * 4;
        //bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        //bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

        //D3D11_SUBRESOURCE_DATA InitData = {};
        //InitData.pSysMem = vertices;

        //Renderer::GetDevice()->CreateBuffer(&bd, &InitData, &m_vertexBuffer);

        //// インデックスデータ
        //WORD indices[] = { 0, 1, 2, 0, 2, 3 };

        //bd.Usage = D3D11_USAGE_DEFAULT;
        //bd.ByteWidth = sizeof(WORD) * 6;
        //bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
        //bd.CPUAccessFlags = 0;

        //InitData.pSysMem = indices;
        //Renderer::GetDevice()->CreateBuffer(&bd, &InitData, &m_indexBuffer);
    }

    void Billboard::Update(const Matrix4x4& viewMatrix) 
    {
        UpdateVertexBuffer(viewMatrix);
    }
    void Billboard::UpdateVertexBuffer(const Matrix4x4& viewMatrix)
    {
        // ビュー行列の逆行列を計算
        DirectX::XMVECTOR det;
        Matrix4x4 invView = DirectX::XMMatrixInverse(&det, viewMatrix);

        // カメラのright（右）ベクトルとup（上）ベクトルを抽出
        Vector3 right = Vector3(invView._11, invView._21, invView._31);
        Vector3 up = Vector3(invView._12, invView._22, invView._32);

        // ベクトルを正規化
        right = DirectX::XMVector3Normalize(right);
        up = DirectX::XMVector3Normalize(up);

        // ビルボードのサイズに応じてスケール
        Vector3 halfWidth = right * (m_width * 0.5f);
        Vector3 halfHeight = up * (m_height * 0.5f);

        // 4つの頂点をワールド空間で計算
        BillboardVertex vertices[4] = {
            // 左下
            { m_position - halfWidth - halfHeight,
              Vector3(0.0f, 0.0f, -1.0f),
              Vector4(1.0f, 1.0f, 1.0f, 1.0f),
              Vector2(0.0f, 1.0f) },

              // 左上
              { m_position - halfWidth + halfHeight,
                Vector3(0.0f, 0.0f, -1.0f),
                Vector4(1.0f, 1.0f, 1.0f, 1.0f),
                Vector2(0.0f, 0.0f) },

                // 右上
                { m_position + halfWidth + halfHeight,
                  Vector3(0.0f, 0.0f, -1.0f),
                  Vector4(1.0f, 1.0f, 1.0f, 1.0f),
                  Vector2(1.0f, 0.0f) },

                  // 右下
                  { m_position + halfWidth - halfHeight,
                    Vector3(0.0f, 0.0f, -1.0f),
                    Vector4(1.0f, 1.0f, 1.0f, 1.0f),
                    Vector2(1.0f, 1.0f) }
        };

        // 頂点バッファを更新
        D3D11_MAPPED_SUBRESOURCE mappedResource;
        HRESULT hr = Renderer::GetDeviceContext()->Map(m_vertexBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
        if (SUCCEEDED(hr))
        {
            memcpy(mappedResource.pData, vertices, sizeof(vertices));
            Renderer::GetDeviceContext()->Unmap(m_vertexBuffer.Get(), 0);
        }
        //// 頂点バッファを更新
        //D3D11_MAPPED_SUBRESOURCE mappedResource;
        //HRESULT hr = Renderer::GetDeviceContext()->Map(m_vertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
        //if (SUCCEEDED(hr)) {
        //    memcpy(mappedResource.pData, vertices, sizeof(vertices));
        //    Renderer::GetDeviceContext()->Unmap(m_vertexBuffer, 0);
        //}
    }
// === 視覚的な説明用のコメント ===

/*
カメラの位置と向きの例：

1. カメラが正面から見ている場合：
   カメラ位置: (0, 0, -10)
   注視点: (0, 0, 0)

   right = (1, 0, 0)  // X軸正方向
   up = (0, 1, 0)     // Y軸正方向

   ビルボード中心が(0, 0, 0)、サイズが2x2の場合：
   左下(-1, -1, 0), 左上(-1, 1, 0), 右上(1, 1, 0), 右下(1, -1, 0)

2. カメラが右側から見ている場合：
   カメラ位置: (10, 0, 0)
   注視点: (0, 0, 0)

   right = (0, 0, -1)  // Z軸負方向
   up = (0, 1, 0)      // Y軸正方向

   ビルボード中心が(0, 0, 0)、サイズが2x2の場合：
   左下(0, -1, 1), 左上(0, 1, 1), 右上(0, 1, -1), 右下(0, -1, -1)

   → カメラから見ると同じ正方形に見える！

3. カメラが斜めから見ている場合も同様に、
   rightとupベクトルがカメラの向きに応じて自動的に調整される
*/

void Billboard::Draw()
{
    // SRT情報作成（他のクラスと同じパターン）
    SRT srt;
    srt.pos = m_position;           // 位置
    srt.rot = Vector3(0.0f, 0.0f, 0.0f);  // 回転なし（ビルボードなので）
    srt.scale = Vector3(1.0f, 1.0f, 1.0f); // スケール

    Matrix4x4 worldmtx = srt.GetMatrix();
    Renderer::SetWorldMatrix(&worldmtx);  // GPUにセット

    if (m_blendType == BillboardBlendType::Additive)
    {
     
        Renderer::SetBlendState(BS_ADDITIVE);
        Renderer::SetDepthTestOnly();
    }
    // シェーダーを設定
    g_Shader.SetGPU();

    // マテリアルを設定（重要！）
    m_Material.SetGPU();  // 他のクラスと同じ方法

    // テクスチャを設定これ変更したらでた
    if (m_texture)
    {
        ID3D11ShaderResourceView* srv = m_texture.Get();
        Renderer::GetDeviceContext()->PSSetShaderResources(0, 1, &srv);
    }


    // 頂点バッファ設定
    UINT stride = sizeof(BillboardVertex);
    UINT offset = 0;
    Renderer::GetDeviceContext()->IASetVertexBuffers(0, 1, m_vertexBuffer.GetAddressOf(), &stride, &offset);
    Renderer::GetDeviceContext()->IASetIndexBuffer(m_indexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0);

    // プリミティブトポロジー設定
    Renderer::GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    Renderer::GetDeviceContext()->IASetInputLayout(g_Shader.GetInputLayout());
    // 描画
    Renderer::GetDeviceContext()->DrawIndexed(6, 0, 0);

    Renderer::SetBlendState(BS_ALPHABLEND);
    Renderer::SetDepthEnable(true);
}
