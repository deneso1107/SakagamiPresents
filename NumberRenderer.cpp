#include "NumberRenderer.h"
#include "system/renderer.h"
#include <algorithm>
#include <WICTextureLoader.h>
#include <cmath>

bool NumberRenderer::s_isInitialized = false;

NumberRenderer::NumberRenderer()
    : m_basePosition(0.5f, 0.1f)
    , m_lastBasePosition(0.5f, 0.1f)
    , m_currentPosition(0.5f, 0.1f)
    , m_digitWidth(0.05f)
    , m_digitHeight(0.08f)
    , m_digitSpacing(0.01f)
    , m_rightAlign(true)
    , m_digitTexture(nullptr)
    , m_lastDisplayNumber(-999999)
    , m_isAnimating(false)
    , m_animationTime(0.0f)
    , m_animationDuration(0.5f)
    , m_currentScale(1.0f)
    , m_targetScale(1.0f)
    , m_currentAlpha(1.0f)
    , m_animationType(AnimationType::None)
    , s_displayNumber(0)
{
}

NumberRenderer::~NumberRenderer()
{
    Dispose();
}

void NumberRenderer::Init(const Vector2& basePos, float digitWidth, float digitHeight,
    float spacing, bool rightAlign)
{
    m_basePosition = basePos;
    m_lastBasePosition = basePos;
    m_currentPosition = m_basePosition;
    m_digitWidth = digitWidth;
    m_digitHeight = digitHeight;
    m_digitSpacing = spacing;
    m_rightAlign = rightAlign;

    LoadDigitTexture();
    s_isInitialized = true;
    Vector2 backpos = basePos;
    backpos.y -= 0.05f;
    backpos.x -= 0.05f;
    m_BackGroundScoreBillBoard= std::make_unique<ScreenFixedBillboard>(backpos, digitWidth*4, digitHeight*3, L"assets/texture/text/Score.png");
    m_BackGroundTimeBillBoard= std::make_unique<ScreenFixedBillboard>(backpos, digitWidth*4, digitHeight*3, L"assets/texture/text/Time.png");
    m_BackGroundTimeBillBoard= std::make_unique<ScreenFixedBillboard>(backpos, digitWidth*4, digitHeight*3, L"assets/texture/text/Time.png");
}

void NumberRenderer::Dispose()
{
    m_digitBillboards.clear();
    if (m_digitTexture) {
        m_digitTexture->Release();
        m_digitTexture = nullptr;
    }
}

void NumberRenderer::LoadDigitTexture()
{
    const wchar_t* texturePath = L"assets/texture/number2.png";
    HRESULT hr = DirectX::CreateWICTextureFromFile(
        Renderer::GetDevice(),
        texturePath,
        nullptr,
        &m_digitTexture
    );

    if (FAILED(hr)) {
        OutputDebugStringA("Failed to load digit texture\n");
    }
}

void NumberRenderer::StartAnimation(AnimationType type, float duration)
{

    m_isAnimating = true;
    m_animationTime = 0.0f;
    m_animationDuration = duration;
    m_animationType = type;

    // アニメーション開始時の初期値設定
    switch (type)
    {
    case AnimationType::ScaleBounce:
    case AnimationType::ScaleRotate:
    case AnimationType::FadeScale:
        m_currentScale = 0.0f;  // 0から開始
        m_currentAlpha = (type == AnimationType::FadeScale) ? 0.0f : 1.0f;
        break;
    case AnimationType::Punch:
        m_currentScale = 1.0f;  // 通常サイズから開始
        m_currentAlpha = 1.0f;
        break;
    default:
        break;
    }

    m_targetScale = 1.0f;
    ApplyAnimationToDigits();
}

void NumberRenderer::StopAnimation()
{
    m_isAnimating = false;
    m_currentScale = 1.0f;
    m_currentAlpha = 1.0f;
}

void NumberRenderer::TestAnimation()
{
    // 全ての桁を2倍のサイズにしてみる
    for (auto& billboard : m_digitBillboards) {
        if (billboard) {
            billboard->SetScreenPosition(Vector2(m_digitWidth * 2.0f, m_digitHeight * 2.0f));
        }
    }
}

void NumberRenderer::Update(float deltaTime)
{
    if (!s_isInitialized) return;

    // アニメーション更新
    if (m_isAnimating) {
        UpdateAnimation(deltaTime);
    }

    // 数値が変更された場合,または場所が更新されたときのみ更新
    if ((s_displayNumber != m_lastDisplayNumber)|| (m_basePosition != m_lastBasePosition)) {
        UpdateDigitBillboardsOptimized();
        m_lastDisplayNumber = s_displayNumber;
		m_lastBasePosition = m_basePosition;
        printf("%f\n", m_basePosition.x);
    }

    for (auto& billboard : m_digitBillboards) {
        if (billboard) {
            billboard->Update();
        }
    }
}

void NumberRenderer::UpdateAnimation(float deltaTime)
{
    m_animationTime += deltaTime;
    float t = std::min(m_animationTime / m_animationDuration, 1.0f);

    switch (m_animationType)
    {
    case AnimationType::ScaleBounce:
        m_currentScale = EaseOutElastic(t) * m_targetScale;
        break;

    case AnimationType::ScaleRotate:
        m_currentScale = EaseOutBack(t) * m_targetScale;
        // 回転は描画時に適用
        break;

    case AnimationType::Punch:
        // パンチ：1.0 -> 1.3 -> 1.0
        if (t < 0.5f) {
            m_currentScale = 1.0f + (0.3f * (t / 0.5f));
        }
        else {
            m_currentScale = 1.3f - (0.3f * ((t - 0.5f) / 0.5f));
        }
        break;

    case AnimationType::FadeScale:
        m_currentScale = EaseInOutCubic(t) * m_targetScale;
        m_currentAlpha = t;
        break;

    default:
        break;
    }
    ApplyAnimationToDigits();

    // アニメーション終了チェック
    if (t >= 1.0f) {
        m_isAnimating = false;
        m_currentScale = 1.0f;
        m_currentAlpha = 1.0f;
    }
}

void NumberRenderer::ApplyAnimationToDigits()
{
    if (!m_isAnimating) return;

    std::vector<int> digits = NumberToDigits(s_displayNumber);

    for (size_t i = 0; i < digits.size() && i < m_digitBillboards.size(); ++i) {
        if (!m_digitBillboards[i]) continue;

        // スケールを適用した位置とサイズを計算
        Vector2 digitPos;
        float scaledWidth = m_digitWidth * m_currentScale;
        float scaledHeight = m_digitHeight * m_currentScale;
        float scaledSpacing = m_digitSpacing * m_currentScale;

        if (m_rightAlign) {
            // 右寄せの場合、右端を基準にスケール
            digitPos.x = m_basePosition.x - (digits.size() - 1 - i) * (scaledWidth + scaledSpacing);
        }
        else {
            // 左寄せの場合、左端を基準にスケール
            digitPos.x = m_basePosition.x + i * (scaledWidth + scaledSpacing);
        }
        digitPos.y = m_basePosition.y;

        // ビルボードのサイズと位置を更新
        m_digitBillboards[i]->SetScreenPosition(digitPos);
            m_digitBillboards[i]->SetSize(scaledWidth, scaledHeight);

        // 透明度を設定（もしScreenFixedBillboardにSetAlphaメソッドがあれば）
        // m_digitBillboards[i]->SetAlpha(m_currentAlpha);
    }
}

void NumberRenderer::UpdateDigitBillboardsOptimized()
{
    std::vector<int> digits = NumberToDigits(s_displayNumber);

    if (m_digitBillboards.size() != digits.size()) {
        ResizeDigitBillboards(digits.size());
        m_lastDigits.resize(digits.size(), -1);
    }

    for (size_t i = 0; i < digits.size(); ++i) {
        if (i >= m_lastDigits.size() || m_lastDigits[i] != digits[i]) {
            Vector2 digitPos;

            //アニメーション中はスケールを考慮
            float effectiveWidth = m_digitWidth * m_currentScale;
            float effectiveSpacing = m_digitSpacing * m_currentScale;

            if (m_rightAlign) {
                digitPos.x = m_basePosition.x - (digits.size() - 1 - i) * (effectiveWidth + effectiveSpacing);
            }
            else {
                digitPos.x = m_basePosition.x + i * (effectiveWidth + effectiveSpacing);
            }
            digitPos.y = m_basePosition.y;

            UpdateSingleDigit(i, digits[i], digitPos);

            if (i < m_lastDigits.size()) {
                m_lastDigits[i] = digits[i];
            }
        }
    }
}

void NumberRenderer::ResizeDigitBillboards(size_t newSize)
{
    if (newSize > m_digitBillboards.size()) {
        size_t oldSize = m_digitBillboards.size();
        m_digitBillboards.resize(newSize);

        for (size_t i = oldSize; i < newSize; ++i) {
            m_digitBillboards[i] = std::make_unique<ScreenFixedBillboard>(
                Vector2(0.5f, 0.1f), m_digitWidth, m_digitHeight, L"assets/texture/number2.png");
        }
    }
    else if (newSize < m_digitBillboards.size()) {
        m_digitBillboards.resize(newSize);
    }
}

void NumberRenderer::UpdateSingleDigit(size_t index, int digit, const Vector2& position)
{
    if (index >= m_digitBillboards.size() || !m_digitBillboards[index]) {
        return;
    }

    m_digitBillboards[index]->SetScreenPosition(position);

    // アニメーション中はサイズも更新
    if (m_isAnimating) {
        float scaledWidth = m_digitWidth * m_currentScale;
        float scaledHeight = m_digitHeight * m_currentScale;
        m_digitBillboards[index]->SetSize(scaledWidth, scaledHeight);
    }

    Vector2 uv = CalculateDigitUV(digit);
    float u1 = uv.x;
    float v1 = 0.0f;
    float u2 = uv.x + DIGIT_UV_WIDTH;
    float v2 = 1.0f;

    m_digitBillboards[index]->SetUVRange(u1, v1, u2, v2);
}

Vector2 NumberRenderer::CalculateDigitUV(int digit)
{
    float u = (digit % DIGITS_PER_ROW) * DIGIT_UV_WIDTH;
    return Vector2(u, 0.0f);
}

void NumberRenderer::Draw(bool useback,bool time)//第二引数でどちらを使うか選択
{
    if (!s_isInitialized) return;

    if (useback)//バックグラウンドが必要なら描画
    {
        if (time)//
        {
            m_BackGroundTimeBillBoard->Draw();
        }
        else
        {
            m_BackGroundScoreBillBoard->Draw();
        }
    }
    for (auto& billboard : m_digitBillboards) {
        if (billboard) {
            billboard->Draw();
        }
    }
}

std::vector<int> NumberRenderer::NumberToDigits(int number)
{
    std::vector<int> digits;

    if (number == 0) {
        digits.push_back(0);
        return digits;
    }

    int absNumber = abs(number);

    while (absNumber > 0) {
        digits.push_back(absNumber % 10);
        absNumber /= 10;
    }

    std::reverse(digits.begin(), digits.end());
    return digits;
}

// イージング関数
float NumberRenderer::EaseOutElastic(float t)
{
    const float c4 = (2.0f * 3.14159f) / 3.0f;

    if (t == 0.0f) return 0.0f;
    if (t == 1.0f) return 1.0f;

    return pow(2.0f, -10.0f * t) * sin((t * 10.0f - 0.75f) * c4) + 1.0f;
}

float NumberRenderer::EaseOutBack(float t)
{
    const float c1 = 1.70158f;
    const float c3 = c1 + 1.0f;

    return 1.0f + c3 * pow(t - 1.0f, 3.0f) + c1 * pow(t - 1.0f, 2.0f);
}

float NumberRenderer::EaseInOutCubic(float t)
{
    return t < 0.5f ? 4.0f * t * t * t : 1.0f - pow(-2.0f * t + 2.0f, 3.0f) / 2.0f;
}