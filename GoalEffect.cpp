#include "GoalEffect.h"
#include <algorithm>
#include <cmath>

GoalEffect::GoalEffect()
    : m_flashBillboard(nullptr)
    , m_goalTextBillboard(nullptr)
    , m_currentPhase(GoalPhase::None)
    , m_phaseTimer(0.0f)
    , m_totalTimer(0.0f)
    , m_currentFlashAlpha(0.0f)
    , m_currentGoalTextScale(1.0f)
    , m_currentGoalTextAlpha(1.0f)
    , m_isActive(false)
{
}

GoalEffect::~GoalEffect()
{
    Release();
}

void GoalEffect::Initialize()
{
    // フラッシュ用ビルボード（全画面白）
    m_flashBillboard = new ScreenFixedBillboard(
        Vector2(0.5f, 0.5f),    // 画面中央
        1.0f,                    // 画面全体の幅
        1.0f,                    // 画面全体の高さ
        L"assets/texture/white.png"  // 白いテクスチャ
    );

    // "GOAL!" テキスト用ビルボード
    m_goalTextBillboard = new ScreenFixedBillboard(
        Vector2(0.5f, 0.4f),    // 画面中央やや上
        0.3f,                    // テキストサイズ
        0.15f,
        L"assets/texture/Goal.png"  // "GOAL!" テクスチャ
    );
}

void GoalEffect::Start()
{
    m_isActive = true;
    m_totalTimer = 0.0f;
    TransitionToPhase(GoalPhase::Flash);
}

void GoalEffect::Update(float deltaTime)
{
    if (!m_isActive) return;

    m_phaseTimer += deltaTime;
    m_totalTimer += deltaTime;

    switch (m_currentPhase) {
    case GoalPhase::Flash:
    {
        float t = m_phaseTimer / m_params.flashDuration;
        UpdateFlash(t);

        if (m_phaseTimer >= m_params.flashDuration) {
            TransitionToPhase(GoalPhase::GoalText);
        }
        break;
    }

    case GoalPhase::GoalText:
    {
        float t = m_phaseTimer / m_params.goalTextDuration;
        UpdateGoalText(t);

        if (m_phaseTimer >= m_params.goalTextDuration) {
            TransitionToPhase(GoalPhase::FadeOut);
        }
        break;
    }

    case GoalPhase::FadeOut:
    {
        float t = m_phaseTimer / m_params.fadeOutDuration;
        UpdateFadeOut(t);

        if (m_phaseTimer >= m_params.fadeOutDuration) {
            TransitionToPhase(GoalPhase::Finished);
        }
        break;
    }

    case GoalPhase::Finished:
        m_isActive = false;
        break;

    default:
        break;
    }

    // ビルボード更新
    if (m_flashBillboard) m_flashBillboard->Update();
    if (m_goalTextBillboard) m_goalTextBillboard->Update();
}

void GoalEffect::UpdateFlash(float t)
{
    // フラッシュ: 0→1→0 の三角波
    if (t < 0.5f) {
        m_currentFlashAlpha = (t * 2.0f) * m_params.flashMaxAlpha;
    }
    else {
        m_currentFlashAlpha = ((1.0f - t) * 2.0f) * m_params.flashMaxAlpha;
    }
    if (m_flashBillboard) {
        m_flashBillboard->SetAlpha(m_currentFlashAlpha);
    }
}

void GoalEffect::UpdateGoalText(float t)
{
    // GOALテキストのスケールアニメーション
    float bounceRatio = m_params.goalTextBounceTime / m_params.goalTextDuration;

    if (t < bounceRatio) {
        float bounceT = t / bounceRatio;
        bounceT = EaseOutBack(bounceT);
        m_currentGoalTextScale = m_params.goalTextScaleStart +
            (m_params.goalTextScaleMax - m_params.goalTextScaleStart) * bounceT;
        m_currentGoalTextAlpha = bounceT;
    }
    else {
        float stabilizeT = (t - bounceRatio) / (1.0f - bounceRatio);
        stabilizeT = EaseOutQuad(stabilizeT);
        m_currentGoalTextScale = m_params.goalTextScaleMax -
            (m_params.goalTextScaleMax - m_params.goalTextScaleEnd) * stabilizeT;
        m_currentGoalTextAlpha = 1.0f;
    }

    // スケールを適用
    float baseWidth = 0.3f;
    float baseHeight = 0.15f;
    m_goalTextBillboard->SetSize(
        baseWidth * m_currentGoalTextScale,
        baseHeight * m_currentGoalTextScale
    );

    if (m_goalTextBillboard) 
    {
        m_goalTextBillboard->SetAlpha(m_currentGoalTextAlpha);
    }
}

void GoalEffect::UpdateFadeOut(float t)
{
    // GOALテキストをフェードアウト
    m_currentGoalTextAlpha = 1.0f - t;

    // アルファ値を設定
    if (m_goalTextBillboard) {
        m_goalTextBillboard->SetAlpha(m_currentGoalTextAlpha);
    }
}

void GoalEffect::TransitionToPhase(GoalPhase newPhase)
{
    m_currentPhase = newPhase;
    m_phaseTimer = 0.0f;

    // フェーズ開始時の初期化
    switch (newPhase) {
    case GoalPhase::Flash:
        m_currentFlashAlpha = 0.0f;
        break;

    case GoalPhase::GoalText:
        m_currentGoalTextScale = m_params.goalTextScaleStart;
        m_currentGoalTextAlpha = 0.0f;
        break;

    case GoalPhase::FadeOut:
        m_currentGoalTextAlpha = 1.0f;
        break;

    default:
        break;
    }
}

void GoalEffect::Draw()
{
    if (!m_isActive) return;

    // 現在のマトリックスを保存
    Matrix4x4 savedWorld, savedView, savedProj;
    savedWorld = Renderer::GetWorldMatrix();
    savedView = Renderer::GetViewMatrix();
    savedProj = Renderer::GetProjectionMatrix();

    //ブレンドステートを保存して、アルファブレンドを設定
    ID3D11BlendState* savedBlendState = nullptr;
    float savedBlendFactor[4];
    UINT savedSampleMask = 0;
    Renderer::GetDeviceContext()->OMGetBlendState(&savedBlendState, savedBlendFactor, &savedSampleMask);

    D3D11_BLEND_DESC blendDesc = {};
    blendDesc.AlphaToCoverageEnable = FALSE;
    blendDesc.IndependentBlendEnable = FALSE;
    blendDesc.RenderTarget[0].BlendEnable = TRUE;
    blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
    blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
    blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
    blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
    blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

    ID3D11BlendState* uiBlendState = nullptr;
    Renderer::GetDevice()->CreateBlendState(&blendDesc, &uiBlendState);
    float blendFactor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
    Renderer::GetDeviceContext()->OMSetBlendState(uiBlendState, blendFactor, 0xFFFFFFFF);

    // 深度ステンシルステート: Zテスト無効
    ID3D11DepthStencilState* savedDepthState = nullptr;
    UINT savedStencilRef = 0;
    Renderer::GetDeviceContext()->OMGetDepthStencilState(&savedDepthState, &savedStencilRef);

    D3D11_DEPTH_STENCIL_DESC depthDesc = {};
    depthDesc.DepthEnable = FALSE;
    depthDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
    depthDesc.DepthFunc = D3D11_COMPARISON_ALWAYS;
    depthDesc.StencilEnable = FALSE;

    ID3D11DepthStencilState* uiDepthState = nullptr;
    Renderer::GetDevice()->CreateDepthStencilState(&depthDesc, &uiDepthState);
    Renderer::GetDeviceContext()->OMSetDepthStencilState(uiDepthState, 0);

    // フラッシュの描画
    if (m_currentPhase == GoalPhase::Flash && m_currentFlashAlpha > 0.01f) {
        if (m_flashBillboard) {
            m_flashBillboard->Draw();
        }
    }

    // GOALテキストの描画
    if ((m_currentPhase == GoalPhase::GoalText || m_currentPhase == GoalPhase::FadeOut)
        && m_currentGoalTextAlpha > 0.01f) {
        if (m_goalTextBillboard) {
            m_goalTextBillboard->Draw();
        }
    }

    //ステートを復元
    if (uiBlendState) {
        uiBlendState->Release();
    }
    Renderer::GetDeviceContext()->OMSetBlendState(savedBlendState, savedBlendFactor, savedSampleMask);
    if (savedBlendState) {
        savedBlendState->Release();
    }

    if (uiDepthState) {
        uiDepthState->Release();
    }
    Renderer::GetDeviceContext()->OMSetDepthStencilState(savedDepthState, savedStencilRef);
    if (savedDepthState) {
        savedDepthState->Release();
    }

    // マトリックスを復元
    Renderer::SetWorldMatrix(&savedWorld);
    Renderer::SetViewMatrix(&savedView);
    Renderer::SetProjectionMatrix(&savedProj);
}

void GoalEffect::Release()
{
    if (m_flashBillboard) {
        delete m_flashBillboard;
        m_flashBillboard = nullptr;
    }

    if (m_goalTextBillboard) {
        delete m_goalTextBillboard;
        m_goalTextBillboard = nullptr;
    }
}

// イージング関数
float GoalEffect::EaseOutBack(float t) const
{
    const float c1 = 1.70158f;
    const float c3 = c1 + 1.0f;
    return 1.0f + c3 * pow(t - 1.0f, 3.0f) + c1 * pow(t - 1.0f, 2.0f);
}

float GoalEffect::EaseOutQuad(float t) const
{
    return 1.0f - (1.0f - t) * (1.0f - t);
}