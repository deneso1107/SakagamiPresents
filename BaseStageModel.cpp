#include "BaseStageModel.h"
void BaseStageModel::Init() {
    if (m_isInitialized) return;

    try {
        m_mesh.Load(GetModelFileName().c_str(), "assets/model/");

        m_meshRenderer.Init(m_mesh);

        // ハイライト用定数バッファ作成
        D3D11_BUFFER_DESC bd = {};
        bd.Usage = D3D11_USAGE_DYNAMIC;
        bd.ByteWidth = sizeof(HighlightBuffer);
        bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        Renderer::GetDevice()->CreateBuffer(&bd, nullptr, &m_highlightCBuffer);

        // シェーダーをステージセレクト用に変更
        m_shader.Create(
            "shader/StageModelVS.hlsl",
            "shader/StageModelPS.hlsl");
        m_isInitialized = true;
		m_Rotation.x= -90.0f;
    }
    catch (...) {
        m_isInitialized = false;
        throw;
    }
}

void BaseStageModel::Update(float deltatime) 
{
    m_floatTimer += deltatime;

    // ふわふわ上下
    float floatOffset = sinf(m_floatTimer * 1.2f) * 1.5f; // 周期,振幅は調整
    m_Position.y = m_baseY + floatOffset;

    // 通常回転 + 入力スピン
    float baseRotSpeed = 0.4f;
    m_rotationY += (baseRotSpeed + m_spinVelocity) * deltatime;

    // スピンは徐々に減衰
    m_spinVelocity = m_spinVelocity * (1.0f - 5.0f * deltatime);
    if (fabsf(m_spinVelocity) < 0.001f) m_spinVelocity = 0.0f;

    // 慣性傾き（LerpはSpringCameraと同じ方式）
    m_tiltZ += (m_targetTiltZ - m_tiltZ) * (8.0f * deltatime);

    // ハイライト値を補間
    float targetHighlight = m_isSelected ? 1.0f : 0.0f;
    m_highlight += (targetHighlight - m_highlight) * (5.0f * deltatime);

    // 回転・傾きをセット
    m_Rotation.y = m_rotationY;
    m_Rotation.z = m_tiltZ * 0.3f; // 傾き量（ラジアン）
}

void BaseStageModel::Draw() {
    if (!m_isInitialized) return;

    // ハイライトバッファを更新してb1にセット
    D3D11_MAPPED_SUBRESOURCE mapped;
    auto* ctx = Renderer::GetDeviceContext();
    ctx->Map(m_highlightCBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);

    HighlightBuffer hb;
    float intensity = m_highlight * 0.8f; // 強さ調整
    hb.highlightColor = { 1.0f, 0.9f, 0.5f, intensity }; // 黄色っぽいハイライト
    memcpy(mapped.pData, &hb, sizeof(hb));
    ctx->Unmap(m_highlightCBuffer, 0);

    // b1スロットにセット（b0はWorldViewProjが使っている想定）
    ctx->VSSetConstantBuffers(6, 1, &m_highlightCBuffer);

    SRT srt;
    srt.pos = m_Position;
    srt.rot = m_Rotation;
    // 個別スケール × カルーセルスケールを合成
    srt.scale = m_baseScale * m_carouselScale;

    Matrix4x4 worldmtx = srt.GetMatrix();
    Renderer::SetWorldMatrix(&worldmtx);
	m_shader.SetGPU();
    m_meshRenderer.Draw();
}

void BaseStageModel::Dispose() {
    if (!m_isInitialized) return;
    //m_mesh.dispose();
    m_isInitialized = false;
}