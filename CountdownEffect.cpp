#include "CountdownEffect.h"
#include <algorithm>
#include <cmath>
#include "SoundManager.h"

CountdownEffect::CountdownEffect()
    : m_numberBillboard(nullptr)
    , m_goBillboard(nullptr)
    , m_currentState(CountdownState::None)
    , m_stateTimer(0.0f)
    , m_numberPosition(0.5f, 0.5f)
    , m_goPosition(0.2f, 0.5f)
    , m_numberWidth(0.15f)
    , m_numberHeight(0.15f)
    , m_goWidth(0.12f)
    , m_goHeight(0.12f)
    , m_bgWidth(0.2f)
    , m_bgHeight(0.2f)
    , m_currentNumberScale(1.0f)
    , m_currentNumberAlpha(1.0f)
    , m_currentBgAlpha(1.0f)
    , m_currentGoScale(1.0f)
    , m_currentGoAlpha(1.0f)
    , m_isActive(false)
{
}

CountdownEffect::~CountdownEffect()
{
    Release();
}

void CountdownEffect::Initialize(
    const Vector2& numberPos,
    const Vector2& goPos,
    float numberSize,
    float goSize,
    float bgSize)
{
    m_numberPosition = numberPos;
    m_goPosition = goPos;
    m_numberWidth = numberSize;
    m_numberHeight = numberSize;
    m_goWidth = goSize;
    m_goHeight = goSize;
    m_bgWidth = bgSize;
    m_bgHeight = bgSize;

    // 数字用ビルボード作成（最初は3を表示）
    m_numberBillboard = new ScreenFixedBillboard(
        m_numberPosition,
        m_numberWidth,
        m_numberHeight,
        L"assets/texture/StartCount/Count3.png"  // 3のテクスチャ
    );

    // GO用ビルボード作成
    m_goBillboard = new ScreenFixedBillboard(
        m_goPosition,
        m_goWidth,
        m_goHeight,
        L"assets/texture/StartCount/GO.png"
    );

    // 背景ビルボード作成（右上ピース）
    m_bgTopRight.billboard = new ScreenFixedBillboard(
        m_numberPosition,
        m_bgWidth,
        m_bgHeight,
        L"assets/texture/StartCount/CountDown_BG_Right.png"  // 背景テクスチャ
    );
    // UV座標を右上半分に設定 (0.5, 0.0) ~ (1.0, 0.5)
    m_bgTopRight.billboard->SetUVRange(0.5f, 0.0f, 1.0f, 0.5f);
    m_bgTopRight.velocity = m_params.topRightVelocity;
    m_bgTopRight.rotationSpeed = m_params.rotationSpeed;

    // 背景ビルボード作成（左下ピース）
    m_bgBottomLeft.billboard = new ScreenFixedBillboard(
        m_numberPosition,
        m_bgWidth,
        m_bgHeight,
        L"assets/texture/StartCount/CountDown_BG_Left.png"
    );
    // UV座標を左下半分に設定 (0.0, 0.5) ~ (0.5, 1.0)
    m_bgBottomLeft.billboard->SetUVRange(0.0f, 0.5f, 0.5f, 1.0f);
    m_bgBottomLeft.velocity = m_params.bottomLeftVelocity;
    m_bgBottomLeft.rotationSpeed = -m_params.rotationSpeed;  // 逆回転
}

void CountdownEffect::Start()
{
    m_isActive = true;
    TransitionToState(CountdownState::Show3);
}

void CountdownEffect::Update(float deltaTime)
{
    if (!m_isActive) return;

    m_stateTimer += deltaTime;

    switch (m_currentState) {
    case CountdownState::Show3:
    case CountdownState::Show2:
    case CountdownState::Show1:
    {
        float duration = m_params.numberDuration;
        float t = m_stateTimer / duration;

        // 数字と背景のアニメーション更新
        UpdateNumber(t);
        UpdateBackground(t);

        // 次の状態へ遷移
        if (m_stateTimer >= duration) {
            if (m_currentState == CountdownState::Show3) {
                TransitionToState(CountdownState::Show2);
            }
            else if (m_currentState == CountdownState::Show2) {
                TransitionToState(CountdownState::Show1);
            }
            else {
                TransitionToState(CountdownState::ShowGo);
            }
        }
        break;
    }

    case CountdownState::ShowGo:
    {
        float t = m_stateTimer / m_params.goDuration;
        UpdateGo(t);

        if (m_stateTimer >= m_params.goDuration) {
            TransitionToState(CountdownState::Finished);
        }
        break;
    }

    case CountdownState::Finished:
        m_isActive = false;
        break;

    default:
        break;
    }

    // ビルボード更新
    if (m_numberBillboard) m_numberBillboard->Update();
    if (m_goBillboard) m_goBillboard->Update();
    if (m_bgTopRight.billboard) m_bgTopRight.billboard->Update();
    if (m_bgBottomLeft.billboard) m_bgBottomLeft.billboard->Update();
}

void CountdownEffect::UpdateNumber(float t)
{
    // 数字のスケールアニメーション
    // 0.0 ~ 0.15: 出現（0.8 -> 1.2）
    // 0.15 ~ 0.35: バウンス（1.2 -> 1.0）
    // 0.35 ~ 1.0: 維持

    if (t < m_params.numberAppearTime) {
        float appearT = t / m_params.numberAppearTime;
        appearT = EaseOutBack(appearT);
        m_currentNumberScale = m_params.numberScaleStart +
            (m_params.numberScaleMax - m_params.numberScaleStart) * appearT;
        m_currentNumberAlpha = appearT;
    }
    else if (t < m_params.numberAppearTime + m_params.numberBounceTime) {
        float bounceT = (t - m_params.numberAppearTime) / m_params.numberBounceTime;
        bounceT = EaseOutQuad(bounceT);
        m_currentNumberScale = m_params.numberScaleMax -
            (m_params.numberScaleMax - m_params.numberScaleEnd) * bounceT;
        m_currentNumberAlpha = 1.0f;
    }
    else {
        m_currentNumberScale = m_params.numberScaleEnd;
        m_currentNumberAlpha = 1.0f;
    }

    // スケールを適用
    float scaledWidth = m_numberWidth * m_currentNumberScale;
    float scaledHeight = m_numberHeight * m_currentNumberScale;
    m_numberBillboard->SetSize(scaledWidth, scaledHeight);
}

void CountdownEffect::UpdateBackground(float t)
{
    // 背景のフェードイン（最初の0.15秒）
    if (t < m_params.bgFadeInTime) {
        float fadeT = t / m_params.bgFadeInTime;
        m_currentBgAlpha = fadeT;

        // スケールも徐々に拡大
        float scale = m_params.bgInitialScale +
            (1.0f - m_params.bgInitialScale) * EaseOutQuad(fadeT);

        float scaledWidth = m_bgWidth * scale;
        float scaledHeight = m_bgHeight * scale;
        m_bgTopRight.billboard->SetSize(scaledWidth, scaledHeight);
        m_bgBottomLeft.billboard->SetSize(scaledWidth, scaledHeight);
    }
    // 背景が分割して飛んでいく（0.7秒から0.3秒間）
    else if (t >= m_params.backgroundSplitDelay) {
        float splitT = (t - m_params.backgroundSplitDelay) / m_params.bgSplitDuration;
        splitT = std::min(splitT, 1.0f);
        splitT = EaseInQuad(splitT);

        // 右上ピースの移動
        Vector2 topRightOffset = m_bgTopRight.velocity * splitT;
        Vector2 topRightPos = m_numberPosition + topRightOffset;
        m_bgTopRight.billboard->SetScreenPosition(topRightPos);

        // 回転
        float topRightRotation = m_bgTopRight.rotationSpeed * splitT;
        m_bgTopRight.billboard->SetAngle(topRightRotation);

        // 左下ピースの移動
        Vector2 bottomLeftOffset = m_bgBottomLeft.velocity * splitT;
        Vector2 bottomLeftPos = m_numberPosition + bottomLeftOffset;
        m_bgBottomLeft.billboard->SetScreenPosition(bottomLeftPos);

        // 回転
        float bottomLeftRotation = m_bgBottomLeft.rotationSpeed * splitT;
        m_bgBottomLeft.billboard->SetAngle(bottomLeftRotation);

        // フェードアウト
        m_currentBgAlpha = 1.0f - splitT;
    }
    else {
        // 背景は完全に表示されている状態
        m_currentBgAlpha = 1.0f;
        float scaledWidth = m_bgWidth;
        float scaledHeight = m_bgHeight;
        m_bgTopRight.billboard->SetSize(scaledWidth, scaledHeight);
        m_bgBottomLeft.billboard->SetSize(scaledWidth, scaledHeight);
    }
}

void CountdownEffect::UpdateGo(float t)
{
    // GOのアニメーション
  // フェーズ1 (0.0 ~ goBounceTime): スライドイン + スケールバウンス
  // フェーズ2 (goBounceTime ~ goBounceTime+0.1): 一時停止
  // フェーズ3 (残り時間): スライドアウト（左へ）

    float phase1End = m_params.goBounceTime;
    float phase2End = phase1End + 0.5f;  // 0.1秒間停止

    if (t < phase1End) {
        // フェーズ1: スライドイン
        float slideT = t / phase1End;
        slideT = EaseOutQuad(slideT);

        Vector2 startPos = m_goPosition - Vector2(m_params.goSlideDistance, 0);
        m_currentGoPos = startPos + Vector2(m_params.goSlideDistance * slideT, 0);

        // スケールバウンス
        float scaleT = EaseOutBack(slideT);
        m_currentGoScale = 1.0f + (m_params.goScaleMax - 1.0f) * (1.0f - scaleT);

        m_currentGoAlpha = slideT;
    }
    else if (t < phase2End) {
        // フェーズ2: 一時停止（位置を維持）
        m_currentGoPos = m_goPosition;
        m_currentGoScale = 1.0f;
        m_currentGoAlpha = 1.0f;
    }
    else {
        // フェーズ3: スライドアウト（左へ）
        float slideOutT = (t - phase2End) / m_params.goSlideOutTime;
        slideOutT = std::min(slideOutT, 1.0f);
        slideOutT = EaseInQuad(slideOutT);  // 加速しながら消える

        // 左方向にスライド
        Vector2 slideOffset = Vector2(-m_params.goSlideOutDistance * slideOutT, 0);
        m_currentGoPos = m_goPosition + slideOffset;

        // スケールを少し大きくしながら消える（勢いを表現）
        m_currentGoScale = 1.0f + (0.3f * slideOutT);

        // フェードアウト
        m_currentGoAlpha = 1.0f - slideOutT;
    }

    // GOビルボードに適用
    m_goBillboard->SetScreenPosition(m_currentGoPos);
    float scaledWidth = m_goWidth * m_currentGoScale;
    float scaledHeight = m_goHeight * m_currentGoScale;
    m_goBillboard->SetSize(scaledWidth, scaledHeight);
}

void CountdownEffect::TransitionToState(CountdownState newState)
{
    m_currentState = newState;
    m_stateTimer = 0.0f;

    // 状態に応じてテクスチャを変更
    switch (newState) {
    case CountdownState::Show3:
        // 既に3のテクスチャがロードされている
        SoundManager::GetInstance().PlaySE("countdown");
        break;

    case CountdownState::Show2:
        // 2のテクスチャに変更（実際には新しいビルボードを作成）
        delete m_numberBillboard;
        m_numberBillboard = new ScreenFixedBillboard(
            m_numberPosition,
            m_numberWidth,
            m_numberHeight,
            L"assets/texture/StartCount/Count2.png"
        );
        SoundManager::GetInstance().PlaySE("countdown");
        break;

    case CountdownState::Show1:
        // 1のテクスチャに変更
        delete m_numberBillboard;
        m_numberBillboard = new ScreenFixedBillboard(
            m_numberPosition,
            m_numberWidth,
            m_numberHeight,
            L"assets/texture/StartCount/Count1.png"
        );
        SoundManager::GetInstance().PlaySE("countdown");
        break;

    case CountdownState::ShowGo:
        // 数字ビルボードを非表示にする
        // （描画時にチェックする）
        SoundManager::GetInstance().PlaySE("countdownfinal");
        //Goの時だけ音変えてもいいかも
        break;

    default:
        break;
    }

    // 背景ビルボードを初期位置にリセット
    if (newState == CountdownState::Show2 || newState == CountdownState::Show1) {
        m_bgTopRight.billboard->SetScreenPosition(m_numberPosition);
        m_bgTopRight.billboard->SetAngle(0.0f);
        m_bgBottomLeft.billboard->SetScreenPosition(m_numberPosition);
        m_bgBottomLeft.billboard->SetAngle(0.0f);
    }
}

void CountdownEffect::Draw()
{
    if (!m_isActive) return;

    // 重要：現在のマトリックスを保存
    Matrix4x4 savedWorld, savedView, savedProj;
    savedWorld=Renderer::GetWorldMatrix(/*&savedWorld*/);
    savedView=Renderer::GetViewMatrix();
    savedProj=Renderer::GetProjectionMatrix();

    // ブレンドステートを保存して、アルファブレンドを設定
    ID3D11BlendState* savedBlendState = nullptr;
    float savedBlendFactor[4];
    UINT savedSampleMask = 0;
    Renderer::GetDeviceContext()->OMGetBlendState(&savedBlendState, savedBlendFactor, &savedSampleMask);

    // アルファブレンドを有効化
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

    // 深度ステンシルステート: Zテスト無効、Z書き込み無効
    ID3D11DepthStencilState* savedDepthState = nullptr;
    UINT savedStencilRef = 0;
    Renderer::GetDeviceContext()->OMGetDepthStencilState(&savedDepthState, &savedStencilRef);

    // UIとして最前面に描画（Zテスト完全無効）
    D3D11_DEPTH_STENCIL_DESC depthDesc = {};
    depthDesc.DepthEnable = FALSE;  // Zテスト無効（常に最前面）
    depthDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
    depthDesc.DepthFunc = D3D11_COMPARISON_ALWAYS;
    depthDesc.StencilEnable = FALSE;

    ID3D11DepthStencilState* uiDepthState = nullptr;
    Renderer::GetDevice()->CreateDepthStencilState(&depthDesc, &uiDepthState);
    Renderer::GetDeviceContext()->OMSetDepthStencilState(uiDepthState, 0);

    // 描画順: 背景 -> 数字 -> GO

    // 背景の描画（数字表示中のみ）
    if (m_currentState == CountdownState::Show3 ||
        m_currentState == CountdownState::Show2 ||
        m_currentState == CountdownState::Show1) {

        if (m_currentBgAlpha > 0.01f) {
            if (m_bgTopRight.billboard) {
                m_bgTopRight.billboard->Draw();
            }
            if (m_bgBottomLeft.billboard) {
                m_bgBottomLeft.billboard->Draw();
            }
        }
    }

    // 数字の描画
    if (m_currentState != CountdownState::ShowGo &&
        m_currentState != CountdownState::Finished) {
        if (m_numberBillboard && m_currentNumberAlpha > 0.01f) {
            m_numberBillboard->Draw();
        }
    }

    // GOの描画
    if (m_currentState == CountdownState::ShowGo) {
        if (m_goBillboard && m_currentGoAlpha > 0.01f) {
            m_goBillboard->Draw();
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

    //マトリックスを復元 
    Renderer::SetWorldMatrix(&savedWorld);
    Renderer::SetViewMatrix(&savedView);
    Renderer::SetProjectionMatrix(&savedProj);
}

void CountdownEffect::Release()
{
    if (m_numberBillboard) {
        delete m_numberBillboard;
        m_numberBillboard = nullptr;
    }

    if (m_goBillboard) {
        delete m_goBillboard;
        m_goBillboard = nullptr;
    }

    if (m_bgTopRight.billboard) {
        delete m_bgTopRight.billboard;
        m_bgTopRight.billboard = nullptr;
    }

    if (m_bgBottomLeft.billboard) {
        delete m_bgBottomLeft.billboard;
        m_bgBottomLeft.billboard = nullptr;
    }
}

// イージング関数の実装
float CountdownEffect::EaseOutBack(float t)
{
    const float c1 = 1.70158f;
    const float c3 = c1 + 1.0f;
    return 1.0f + c3 * pow(t - 1.0f, 3.0f) + c1 * pow(t - 1.0f, 2.0f);
}

float CountdownEffect::EaseInQuad(float t)
{
    return t * t;
}

float CountdownEffect::EaseOutQuad(float t)
{
    return 1.0f - (1.0f - t) * (1.0f - t);
}

float CountdownEffect::EaseInOutQuad(float t)
{
    return t < 0.5f ? 2.0f * t * t : 1.0f - pow(-2.0f * t + 2.0f, 2.0f) / 2.0f;
}

const wchar_t* CountdownEffect::GetNumberTexturePath(int number)
{
    switch (number) {
    case 3: return L"assets/texture/StartCount/Count3.png";
    case 2: return L"assets/texture/StartCount/Count2.png";
    case 1: return L"assets/texture/StartCount/Count1.png";
    default: return nullptr;
    }
}