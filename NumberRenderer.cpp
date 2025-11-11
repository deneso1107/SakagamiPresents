#include "NumberRenderer.h"
#include"system/renderer.h"
//#include <DirectXTex.h>
#include <algorithm>
#include <WICTextureLoader.h>

// 静的メンバの初期化
//int NumberRenderer::s_displayNumber = 0;
bool NumberRenderer::s_isInitialized = false;

NumberRenderer::NumberRenderer()
    : m_basePosition(0.5f, 0.1f)
    , m_digitWidth(0.05f)
    , m_digitHeight(0.08f)
    , m_digitSpacing(0.01f)
    , m_rightAlign(true)
    , m_digitTexture(nullptr)
    , m_lastDisplayNumber(-999999) // 初回は必ず更新されるように無効値を設定
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
    m_digitWidth = digitWidth;
    m_digitHeight = digitHeight;
    m_digitSpacing = spacing;
    m_rightAlign = rightAlign;

    LoadDigitTexture();
    s_isInitialized = true;
}

void NumberRenderer::Dispose()
{
    // ビルボードのクリア
    m_digitBillboards.clear();

    // テクスチャの解放
    if (m_digitTexture) {
        m_digitTexture->Release();
        m_digitTexture = nullptr;
    }
}

void NumberRenderer::LoadDigitTexture()
{
    // 数字スプライトシート（0-9が横に並んだ画像）を読み込み
    const wchar_t* texturePath = L"assets/texture/number.png";

    HRESULT hr = DirectX::CreateWICTextureFromFile(
        Renderer::GetDevice(),
        texturePath,
        nullptr,
        &m_digitTexture
    );

    if (FAILED(hr)) {
        // デフォルトテクスチャまたはエラー処理
        OutputDebugStringA("Failed to load digit texture\n");
    }
}

void NumberRenderer::Update()
{
    if (!s_isInitialized) return;

    // 数値が変更された場合のみ更新
    if (s_displayNumber != m_lastDisplayNumber) {
        UpdateDigitBillboardsOptimized();
        m_lastDisplayNumber = s_displayNumber;
    }
}

void NumberRenderer::UpdateDigitBillboardsOptimized()
{
    std::vector<int> digits = NumberToDigits(s_displayNumber);

    // 桁数が変わった場合のみリサイズ
    if (m_digitBillboards.size() != digits.size()) {
        ResizeDigitBillboards(digits.size());
        m_lastDigits.resize(digits.size(), -1); // 全桁を強制更新するために-1で初期化
    }

    // 変更された桁のみを更新（超高速化）
    for (size_t i = 0; i < digits.size(); ++i) {
        if (i >= m_lastDigits.size() || m_lastDigits[i] != digits[i]) {
            Vector2 digitPos;

            if (m_rightAlign) {
                digitPos.x = m_basePosition.x - (digits.size() - 1 - i) * (m_digitWidth + m_digitSpacing);
            }
            else {
                digitPos.x = m_basePosition.x + i * (m_digitWidth + m_digitSpacing);
            }
            digitPos.y = m_basePosition.y;

            UpdateSingleDigit(i, digits[i], digitPos);

            // 更新した桁を記録
            if (i < m_lastDigits.size()) {
                m_lastDigits[i] = digits[i];
            }
        }
    }
}

void NumberRenderer::ResizeDigitBillboards(size_t newSize)
{
    if (newSize > m_digitBillboards.size()) {
        // 桁数が増えた場合：新しいビルボードを追加
        size_t oldSize = m_digitBillboards.size();
        m_digitBillboards.resize(newSize);

        // 新しく追加された部分にビルボードを作成
        for (size_t i = oldSize; i < newSize; ++i) {
            m_digitBillboards[i] = std::make_unique<ScreenFixedBillboard>(
                Vector2(0.5f, 0.1f), m_digitWidth, m_digitHeight, L"assets/texture/number.png");
        }
    }
    else if (newSize < m_digitBillboards.size()) {
        // 桁数が減った場合：余分なビルボードを削除
        m_digitBillboards.resize(newSize);
    }
}

void NumberRenderer::UpdateSingleDigit(size_t index, int digit, const Vector2& position)
{
    if (index >= m_digitBillboards.size() || !m_digitBillboards[index]) {
        return;
    }

    // 位置を更新
    m_digitBillboards[index]->SetScreenPosition(position);

    // UV座標を更新（数字が変わった場合のみ）
    Vector2 uv = CalculateDigitUV(digit);
    float u1 = uv.x;
    float v1 = 0.0f;
    float u2 = uv.x + DIGIT_UV_WIDTH;
    float v2 = 1.0f;

    m_digitBillboards[index]->SetUVRange(u1, v1, u2, v2);
}

Vector2 NumberRenderer::CalculateDigitUV(int digit)
{
    // 0-9の数字に対応するU座標を計算
    float u = (digit % DIGITS_PER_ROW) * DIGIT_UV_WIDTH;
    return Vector2(u, 0.0f);  // Vは常に0（1行の画像なので）
}

void NumberRenderer::Draw()
{
    if (!s_isInitialized) return;

    // 全ての桁ビルボードを描画
    for (auto& billboard : m_digitBillboards) {
        if (billboard) {
            billboard->Draw();
        }
    }
}

std::vector<int> NumberRenderer::NumberToDigits(int number)
{
    std::vector<int> digits;

    // 0の場合の特別処理
    if (number == 0) {
        digits.push_back(0);
        return digits;
    }

    // 負数の場合は絶対値を使用（必要に応じてマイナス記号の処理を追加）
    int absNumber = abs(number);

    // 各桁を取得
    while (absNumber > 0) {
        digits.push_back(absNumber % 10);
        absNumber /= 10;
    }

    // 桁は逆順で取得されるので、正順に戻す
    std::reverse(digits.begin(), digits.end());

    return digits;
}