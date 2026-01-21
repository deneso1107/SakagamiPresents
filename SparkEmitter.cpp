#include "SparkEmitter.h"
#include "system/renderer.h"
#include <WICTextureLoader.h>
#include <algorithm>
using namespace DirectX;

static CShader g_Shader{};
bool SparkEmitter::Init(ID3D11Device* device)
{
    // 1. テクスチャ読み込み（既存のまま）
    HRESULT hr = CreateWICTextureFromFile(device, L"assets/texture/space.png",
        nullptr, m_texture.ReleaseAndGetAddressOf());

    if (!m_shader.CreateParticle("shader/VS_Particle.hlsl", "shader/PS_Particle.hlsl"))
    {
        OutputDebugStringA("パーティクルシェーダー作成失敗\n");
        return false;
    }

    // 3. マテリアル初期化
    MATERIAL materialData = {};
    materialData.Diffuse = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
    materialData.TextureEnable = 1.0f;
    m_material.Create(materialData);

    // 4. パーティクル専用のバッファ作成（既存のまま）
    CreateBuffers();
    return true;

}

void SparkEmitter::Update(float deltaTime)
{

    for (auto& p : m_particles)
    {
        p.life += deltaTime;

        //現在設定されている動作タイプで更新
        switch (m_BehaviorType)
        {
        case ParticleBehaviorType::Burst:
            UpdateBurst(p, deltaTime);
            break;

        case ParticleBehaviorType::Continuous:
            UpdateContinuous(p, deltaTime);
            break;

        case ParticleBehaviorType::Trail:
            UpdateTrail(p, deltaTime);
            break;
        case ParticleBehaviorType::Sparkle:  // ★追加★
            UpdateSparkle(p, deltaTime);
            break;
        }
    }

    m_particles.erase(
        std::remove_if(m_particles.begin(), m_particles.end(),
            [](const Particle& p) { return p.life >= p.lifespan; }),
        m_particles.end()
    );
}
//バースト型の更新
void SparkEmitter::UpdateBurst(Particle& p, float deltaTime)
{
    p.pos.x += p.velocity.x * deltaTime;
    p.pos.y += p.velocity.y * deltaTime;
    p.pos.z += p.velocity.z * deltaTime;

    if (m_Gravity != 0.0f)
    {
        p.velocity.y += m_Gravity * deltaTime;
    }

    float fadeT = 1.0f - (p.life / p.lifespan);
    p.color.w = fadeT;
}

//連続型の更新（砂煙用）
void SparkEmitter::UpdateContinuous(Particle& p, float deltaTime)
{
    // 重力的な減速
    p.velocity.y -= 0.5f * deltaTime;

    // 空気抵抗
    p.velocity.x *= 0.98f;
    p.velocity.y *= 0.98f;
    p.velocity.z *= 0.98f;

    // 位置更新
    p.pos.x += p.velocity.x * deltaTime;
    p.pos.y += p.velocity.y * deltaTime;
    p.pos.z += p.velocity.z * deltaTime;

    // サイズ拡大
    float t = p.life / p.lifespan;
    p.size = m_ParticleSize + t * m_ParticleSize * 1.5f;

    // フェードアウト（後半で急激に）
    float fadeT = 1.0f - (p.life / p.lifespan);
    fadeT = fadeT * fadeT;
    p.color.w = fadeT;
}

void SparkEmitter::SetSparkleMode(bool isGold, float area)
{
    m_isGoldSparkle = isGold;
    m_sparkleArea = area;
    m_BehaviorType = ParticleBehaviorType::Sparkle;

    // キラキラ用の色設定
    if (isGold) {
        // 金色
        m_StartColor = DirectX::XMFLOAT3(1.0f, 0.9f, 0.3f);  // 明るい金色
        m_EndColor = DirectX::XMFLOAT3(1.0f, 0.7f, 0.0f);    // オレンジ寄りの金色
        m_ParticleSize = 8.0f;  // やや大きめ
    }
    else {
        // 白色
        m_StartColor = DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f);
        m_EndColor = DirectX::XMFLOAT3(0.8f, 0.8f, 1.0f);    // 少し青みがかった白
        m_ParticleSize = 5.0f;  // 控えめ
    }

    m_MinSpeed = 2.0f;
    m_MaxSpeed = 5.0f;
    m_Gravity = -0.5f;  // ゆっくり落ちる
}

//軌跡型の更新
void SparkEmitter::UpdateTrail(Particle& p, float deltaTime)
{
    p.velocity.x *= 0.9f;
    p.velocity.y *= 0.9f;
    p.velocity.z *= 0.9f;

    p.pos.x += p.velocity.x * deltaTime;
    p.pos.y += p.velocity.y * deltaTime;
    p.pos.z += p.velocity.z * deltaTime;

    float fadeT = 1.0f - (p.life / p.lifespan);
    p.color.w = fadeT;
}

void SparkEmitter::UpdateSparkle(Particle& p, float deltaTime)
{
    // ゆらゆらと漂いながら落ちる
    p.velocity.x += (rand() % 100 - 50) * 0.002f;  // 横方向にゆらぐ
    p.velocity.z += (rand() % 100 - 50) * 0.002f;

    // ゆるい重力
    p.velocity.y += m_Gravity * deltaTime;

    // 空気抵抗
    p.velocity.x *= 0.99f;
    p.velocity.z *= 0.99f;

    // 位置更新
    p.pos.x += p.velocity.x * deltaTime;
    p.pos.y += p.velocity.y * deltaTime;
    p.pos.z += p.velocity.z * deltaTime;

    // ★キラキラ感：サイズを脈動させる★
    float pulse = sin(p.life * 10.0f) * 0.3f + 0.7f;  // 0.4～1.0
    p.size = m_ParticleSize * pulse;

    // フェードアウト
    float t = p.life / p.lifespan;
    if (t > 0.7f) {
        // 後半30%で急激にフェード
        float fadeT = (t - 0.7f) / 0.3f;
        p.color.w = 1.0f - fadeT;
    }
    else {
        p.color.w = 1.0f;
    }
}


void SparkEmitter::CreateBuffers()
{
    ID3D11Device* device = Renderer::GetDevice();
    HRESULT hr;

    // インデックスデータ
    WORD indices[] = { 0, 1, 2, 0, 2, 3 };
    D3D11_BUFFER_DESC bd = {};
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(WORD) * 6;
    bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    bd.CPUAccessFlags = 0;

    D3D11_SUBRESOURCE_DATA InitData = {};
    InitData.pSysMem = indices;
    device->CreateBuffer(&bd, &InitData, &m_indexBuffer);

    // 頂点バッファ（クアッド用）
    Vertex quadVertices[4] = {
        {{-0.5f,  0.5f, 0.0f}, {0.0f, 0.0f}},
        {{ 0.5f,  0.5f, 0.0f}, {1.0f, 0.0f}},
        {{-0.5f, -0.5f, 0.0f}, {0.0f, 1.0f}},
        {{ 0.5f, -0.5f, 0.0f}, {1.0f, 1.0f}},
    };

    D3D11_BUFFER_DESC vbDesc = {};
    vbDesc.Usage = D3D11_USAGE_DEFAULT;
    vbDesc.ByteWidth = sizeof(Vertex) * 4;
    vbDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

    D3D11_SUBRESOURCE_DATA vbData = {};
    vbData.pSysMem = quadVertices;
    device->CreateBuffer(&vbDesc, &vbData, &m_vertexBuffer);

    // インスタンスバッファ
    vbDesc.Usage = D3D11_USAGE_DYNAMIC;
    vbDesc.ByteWidth = sizeof(InstanceData) * 10000;
    vbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    device->CreateBuffer(&vbDesc, nullptr, &m_instanceBuffer);

    // 定数バッファ
    D3D11_BUFFER_DESC cbDesc = {};
    cbDesc.Usage = D3D11_USAGE_DEFAULT;
    cbDesc.ByteWidth = sizeof(ConstantBuffer);
    cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    cbDesc.CPUAccessFlags = 0;
    device->CreateBuffer(&cbDesc, nullptr, &m_constantBuffer);

    //ブレンドステート作成
    D3D11_BLEND_DESC blendDesc = {};
    blendDesc.RenderTarget[0].BlendEnable = TRUE;
    blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
    blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
    blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ZERO;
    blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
    blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

    hr = device->CreateBlendState(&blendDesc, m_blendState.GetAddressOf());
    if (FAILED(hr)) {
        OutputDebugStringA("ブレンドステート作成失敗\n");
    }

    D3D11_DEPTH_STENCIL_DESC dsDesc = {};
    dsDesc.DepthEnable = TRUE;                    // 深度テストは有効
    dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;  //深度書き込みOFF
    dsDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;

    hr = device->CreateDepthStencilState(&dsDesc, m_depthStencilState.GetAddressOf());
    if (FAILED(hr)) {
        OutputDebugStringA("深度ステンシルステート作成失敗\n");
    }

    //サンプラーステート作成
    D3D11_SAMPLER_DESC sampDesc = {};
    sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
    sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
    sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
    sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    sampDesc.MinLOD = 0;
    sampDesc.MaxLOD = D3D11_FLOAT32_MAX;

    hr = device->CreateSamplerState(&sampDesc, m_samplerState.GetAddressOf());
    if (FAILED(hr)) {
        OutputDebugStringA("サンプラーステート作成失敗\n");
    }
}
DirectX::XMFLOAT3 SparkEmitter::LerpColor(DirectX::XMFLOAT3 a, DirectX::XMFLOAT3 b, float t)// 線形補間関数
{
    return {
        a.x + (b.x - a.x) * t,
        a.y + (b.y - a.y) * t,
        a.z + (b.z - a.z) * t
    };
}

void SparkEmitter::SetupRenderState()
{
    // 行列設定（2D用の正投影）
    ID3D11DeviceContext* context = Renderer::GetDeviceContext();


    // シェーダーを設定
    m_shader.SetGPU();;

    // マテリアル設定
    m_material.SetGPU();

    // ブレンドステート設定（加算合成）
    context->OMSetBlendState(m_blendState.Get(), nullptr, 0xFFFFFFFF);

    //深度ステンシルステート設定（追加）
    context->OMSetDepthStencilState(m_depthStencilState.Get(), 0);

    // テクスチャとサンプラー設定
    context->PSSetShaderResources(0, 1, m_texture.GetAddressOf());
    context->PSSetSamplers(0, 1, m_samplerState.GetAddressOf());
}

void SparkEmitter::Render(ID3D11DeviceContext* context, const DirectX::XMMATRIX& viewProj)
{
    if (m_particles.empty()) return;

    // --- 1. ステート保存 ---
    Microsoft::WRL::ComPtr<ID3D11BlendState> prevBlendState;
    FLOAT prevBlendFactor[4];
    UINT prevSampleMask;
    context->OMGetBlendState(&prevBlendState, prevBlendFactor, &prevSampleMask);
    Microsoft::WRL::ComPtr<ID3D11DepthStencilState> prevDepthStencilState;
    UINT prevStencilRef;
    context->OMGetDepthStencilState(&prevDepthStencilState, &prevStencilRef);

    Microsoft::WRL::ComPtr<ID3D11Buffer> prevVSCB;
    context->VSGetConstantBuffers(0, 1, &prevVSCB);

    // --- 2. 描画ステート設定 ---
    SetupRenderState();

    // --- 3. 定数バッファ更新 ---
    ConstantBuffer cbData = {};
    cbData.viewProj = XMMatrixTranspose(viewProj);
    context->UpdateSubresource(m_constantBuffer.Get(), 0, nullptr, &cbData, 0, 0);
    context->VSSetConstantBuffers(0, 1, m_constantBuffer.GetAddressOf());

    // --- 4. ビルボード回転行列を作成 ---
    // viewProj ではなく、"view" のみを使って回転を作る！
    DirectX::XMMATRIX view = Renderer::GetViewMatrix();
    DirectX::XMMATRIX invView = DirectX::XMMatrixInverse(nullptr, view);
    invView.r[3] = DirectX::XMVectorSet(0, 0, 0, 1);
    DirectX::XMMATRIX billboardRot = invView;

    // --- 5. インスタンスデータ作成 ---
    std::vector<InstanceData> instances;
   //emitterのワールド行列（位置）を加味する
        DirectX::XMMATRIX emitterWorld = DirectX::XMMatrixTranslation(m_Position.x, m_Position.y, m_Position.z);

    for (const auto& p : m_particles)
    {
        DirectX::XMMATRIX scale = DirectX::XMMatrixScaling(p.size, p.size, p.size);
        DirectX::XMMATRIX trans = DirectX::XMMatrixTranslation(p.pos.x, p.pos.y, p.pos.z);

        //ローカル(粒子) → エミッタ位置(ワールド)
        DirectX::XMMATRIX world = scale * billboardRot * trans * emitterWorld;

        InstanceData inst;
        DirectX::XMStoreFloat4x4(&inst.world, world);
        inst.color = p.color;
        instances.push_back(inst);
    }

    // --- 6. バッファ更新 ---
    D3D11_MAPPED_SUBRESOURCE mapped;
    context->Map(m_instanceBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
    memcpy(mapped.pData, instances.data(), sizeof(InstanceData) * instances.size());
    context->Unmap(m_instanceBuffer.Get(), 0);

    // --- 7. 頂点バッファ設定 ---
    ID3D11Buffer* buffers[2] = { m_vertexBuffer.Get(), m_instanceBuffer.Get() };
    UINT strides[2] = { sizeof(Vertex), sizeof(InstanceData) };
    UINT offsets[2] = { 0, 0 };
    context->IASetVertexBuffers(0, 2, buffers, strides, offsets);
    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
    context->IASetInputLayout(m_shader.GetInputLayout());

    // --- 8. 描画 ---
    context->DrawInstanced(4, static_cast<UINT>(instances.size()), 0, 0);

    // --- 9. ステート復元 ---
    context->OMSetBlendState(prevBlendState.Get(), prevBlendFactor, prevSampleMask);
    context->VSSetConstantBuffers(0, 1, prevVSCB.GetAddressOf());
    context->OMSetDepthStencilState(prevDepthStencilState.Get(), prevStencilRef);

}
void SparkEmitter::Emit(const DirectX::XMFLOAT3& pos, const DirectX::XMFLOAT3& dir,
    ParticleBehaviorType type)
{
    switch (type)
    {
    case ParticleBehaviorType::Burst:
        EmitBurst(pos, dir, 100);  // デフォルトで100個
        break;

    case ParticleBehaviorType::Continuous:
        EmitContinuous(pos, dir);
        break;

    case ParticleBehaviorType::Trail:
        EmitTrail(pos, dir);
        break;
    case ParticleBehaviorType::Sparkle:  // ★追加★
        EmitSparkle(pos, dir);
        break;
    }
}
void SparkEmitter::EmitBurst(const DirectX::XMFLOAT3& pos, const DirectX::XMFLOAT3& dir, int count)
{
    for (int i = 0; i < count && m_particles.size() < m_maxParticles; ++i)
    {
        Particle p;

        p.pos = {
            pos.x + (rand() % 100 - 50) * 0.01f,
            pos.y + (rand() % 100 - 50) * 0.01f,
            pos.z + (rand() % 100 - 50) * 0.01f
        };

        float randomAngle = (rand() % 360) * 3.14159f / 180.0f;
        float randomPitch = (rand() % 360 - 180) * 3.14159f / 180.0f;
        float speed = m_MinSpeed + (rand() % 100) / 100.0f * (m_MaxSpeed - m_MinSpeed);

        p.velocity = {
            cosf(randomAngle) * cosf(randomPitch) * speed,
            sinf(randomPitch) * speed,
            sinf(randomAngle) * cosf(randomPitch) * speed
        };

        float t = static_cast<float>(rand() % 100) / 100.0f;//複数の色幅を持たせる
        p.color = {
            LerpFloat(m_StartColor.x, m_EndColor.x, t),
            LerpFloat(m_StartColor.y, m_EndColor.y, t),
            LerpFloat(m_StartColor.z, m_EndColor.z, t),
            1.0f
        };

        p.life = 0.0f;
		p.lifespan = 0.5f + (rand() % 100) / 100.0f * 0.5f;//寿命0.5〜1.0秒
		p.size = m_ParticleSize;

        m_particles.push_back(p);
    }
}

// 連続型の放出（砂煙用）
void SparkEmitter::EmitContinuous(const DirectX::XMFLOAT3& pos, const DirectX::XMFLOAT3& dir)
{
    if (m_particles.size() >= m_maxParticles) return;

    const int sparksToEmit = 3;

    for (int i = 0; i < sparksToEmit && m_particles.size() < m_maxParticles; ++i)
    {
        Particle p;

        p.pos = {
            pos.x + (rand() % 200 - 100) * 0.01f,
            pos.y + (rand() % 50) * 0.001f,
            pos.z + (rand() % 200 - 100) * 0.01f
        };

        float horizontalSpread = (rand() % 100 - 50) * 0.01f;
        float upwardSpeed = 0.5f + (rand() % 100) * 0.01f;

        p.velocity = {
            dir.x * 0.3f + horizontalSpread,
            upwardSpeed,
            dir.z * 0.3f + horizontalSpread
        };

        float grayVariation = 0.8f + (rand() % 20) * 0.01f;
        p.color = {
            grayVariation,
            grayVariation,
            grayVariation,
            1.0f
        };

        p.life = 0.0f;
        p.lifespan = 0.8f + (rand() % 100) / 100.0f * 0.7f;
        p.size = 30.0f + (rand() % 100) * 0.2f;

        m_particles.push_back(p);
    }
}

//軌跡型の放出
void SparkEmitter::EmitTrail(const DirectX::XMFLOAT3& pos, const DirectX::XMFLOAT3& dir)
{
    if (m_particles.size() >= m_maxParticles) return;

    Particle p;
    p.pos = pos;

    p.velocity = {
        (rand() % 20 - 10) * 0.001f,
        (rand() % 20 - 10) * 0.001f,
        (rand() % 20 - 10) * 0.001f
    };

    p.color = { m_StartColor.x, m_StartColor.y, m_StartColor.z, 1.0f };
    p.life = 0.0f;
    p.lifespan = 0.2f;
    p.size = m_ParticleSize;

    m_particles.push_back(p);
}
void SparkEmitter::EmitSparkle(const DirectX::XMFLOAT3& centerPos, const DirectX::XMFLOAT3& dir)
{
    if (m_particles.size() >= m_maxParticles) return;

    // 毎フレーム2～5個のキラキラを生成
    int count = 2 + (rand() % 4);

    for (int i = 0; i < count && m_particles.size() < m_maxParticles; ++i)
    {
        Particle p;

        // ★広範囲にランダム配置★
        p.pos = {
            centerPos.x + (rand() % 200 - 100) * 0.01f * m_sparkleArea / 50.0f,
            centerPos.y + (rand() % 150) * 0.01f * m_sparkleArea / 50.0f,  // 上方向
            centerPos.z + (rand() % 200 - 100) * 0.01f * m_sparkleArea / 50.0f
        };

        // ★初速：ゆっくり上昇、横にゆらぐ★
        float horizontalSpeed = (rand() % 100 - 50) * 0.01f;
        p.velocity = {
            horizontalSpeed,
            1.0f + (rand() % 100) * 0.02f,  // 上昇速度 1.0～3.0
            horizontalSpeed
        };

        // ★色のバリエーション★
        float t = static_cast<float>(rand() % 100) / 100.0f;
        p.color = {
            LerpFloat(m_StartColor.x, m_EndColor.x, t),
            LerpFloat(m_StartColor.y, m_EndColor.y, t),
            LerpFloat(m_StartColor.z, m_EndColor.z, t),
            1.0f
        };

        p.life = 0.0f;
        p.lifespan = 2.0f + (rand() % 100) / 100.0f * 1.0f;  // 2～3秒
        p.size = m_ParticleSize + (rand() % 100) * 0.03f;

        m_particles.push_back(p);
    }
}

SparkEmitter::~SparkEmitter()
{
    Uninit();
}

void SparkEmitter::Uninit()
{
    // バッファを解放
    if (m_vertexBuffer) {
        m_vertexBuffer.Reset();
    }
    if (m_indexBuffer) {
        m_indexBuffer.Reset();
    }
    if (m_instanceBuffer) {
        m_instanceBuffer.Reset();
    }
    if (m_constantBuffer) {
        m_constantBuffer.Reset();
    }
    if (m_vertexShader) {
        m_vertexShader.Reset();
    }
    if (m_pixelShader) {
        m_pixelShader.Reset();
    }
    if (m_texture) {
        m_texture.Reset();
    }
    if (m_samplerState) {
        m_samplerState.Reset();
    }
    if (m_blendState) {
        m_blendState.Reset();
    }
    if (m_inputLayout) {
        m_inputLayout.Reset();
    }

    // パーティクルデータをクリア
    m_particles.clear();

    // その他内部状態のリセット
    m_maxParticles = 1000;
}