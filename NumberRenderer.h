// NumberRenderer.h
#pragma once
#include "ScreenFixedBillboard.h"
#include <vector>
#include <string>

class NumberRenderer
{
private:
    int s_displayNumber;
    static bool s_isInitialized;
    int m_lastDisplayNumber;
    std::vector<int> m_lastDigits;

    Vector2 m_basePosition;
    float m_digitWidth;
    float m_digitHeight;
    float m_digitSpacing;
    bool m_rightAlign;

    ID3D11ShaderResourceView* m_digitTexture;
    static constexpr int DIGITS_PER_ROW = 10;
    static constexpr float DIGIT_UV_WIDTH = 1.0f / DIGITS_PER_ROW;


    std::vector<std::unique_ptr<ScreenFixedBillboard>> m_digitBillboards;
    std::unique_ptr<ScreenFixedBillboard>  m_BackGroundScoreBillBoard;
    std::unique_ptr<ScreenFixedBillboard>  m_BackGroundTimeBillBoard;

    // ★アニメーション用メンバ変数★
    bool m_isAnimating;              // アニメーション中かどうか
    float m_animationTime;           // アニメーション経過時間
    float m_animationDuration;       // アニメーション総時間
    float m_currentScale;            // 現在のスケール
    float m_targetScale;             // 目標スケール（通常は1.0f）
    float m_currentAlpha;            // 現在の透明度


public:
    // アニメーション設定
    enum class AnimationType
    {
        None,
        ScaleBounce,      // スケールアップ + バウンス
        ScaleRotate,      // 回転しながら登場
        Punch,            // パンチ演出
        FadeScale         // フェード + スケール
    };
    AnimationType m_animationType;
    NumberRenderer();
    ~NumberRenderer();

    void Init(const Vector2& basePos, float digitWidth, float digitHeight,
        float spacing = 0.05f, bool rightAlign = true);
    void Dispose();

    void SetNumber(int number) { s_displayNumber = number; }
    int GetNumber() { return s_displayNumber; }
    void AddToNumber(int value) { s_displayNumber += value; }

    void SetPosition(const Vector2& pos) { m_basePosition = pos; }
    void SetDigitSize(float width, float height) { m_digitWidth = width; m_digitHeight = height; }
    void SetSpacing(float spacing) { m_digitSpacing = spacing; }
    void SetAlignment(bool rightAlign) { m_rightAlign = rightAlign; }
	void TestAnimation();
    // ★アニメーション制御★
    void StartAnimation(AnimationType type = AnimationType::ScaleBounce, float duration = 0.5f);
    void StopAnimation();
    bool IsAnimating() const { return m_isAnimating; }

    void Update(float deltaTime);
    void Draw(bool,bool time=true);

private:
    void LoadDigitTexture();
    void UpdateDigitBillboardsOptimized();
    void ResizeDigitBillboards(size_t newSize);
    void UpdateSingleDigit(size_t index, int digit, const Vector2& position);
    Vector2 CalculateDigitUV(int digit);
    std::vector<int> NumberToDigits(int number);

    // ★アニメーション更新★
    void UpdateAnimation(float deltaTime);
    void ApplyAnimationToDigits();

    // ★イージング関数★
    float EaseOutElastic(float t);
    float EaseOutBack(float t);
    float EaseInOutCubic(float t);
};