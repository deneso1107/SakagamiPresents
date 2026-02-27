#pragma once
#include "ScreenFixedBillboard.h"

class ArrowButtonAnimator
{
private:
    float m_timer = 0.0f;
    float m_floatSpeed = 1.2f;   // モデルと同じ速さ
    float m_floatAmount = 0.02f;  // 上下の幅（画面比率）
    float m_baseY = 0.5f;   // 基準Y座標

    // 入力時のスケールダウン
    float m_scale = 1.0f;
    float m_targetScale = 1.0f;
    float m_pressScale = 0.85f;  // 押された時のスケール

    // 左右で位相をずらすと自然な感じに
    float m_phaseOffset = 0.0f;

    float m_baseWidth = 0.1f;  // 基準サイズ
    float m_baseHeight = 0.1f;

public:
    void Init(float baseY, float baseWidth, float baseHeight, float phaseOffset = 0.0f) {
        m_baseY = baseY;
        m_baseWidth = baseWidth;
        m_baseHeight = baseHeight;
        m_phaseOffset = phaseOffset;
    }


    void Update(float deltaTime) {
        m_timer += deltaTime;

        // スケールをLerpで戻す
        m_scale += (m_targetScale - m_scale) * (10.0f * deltaTime);
    }

    // 入力時に呼ぶ
    void OnPress() {
        m_targetScale = m_pressScale;
    }

    // 離した時（毎フレームリセット）
    void OnRelease() {
        m_targetScale = 1.0f;
    }

    void Apply(ScreenFixedBillboard* billboard, float x) {
        if (!billboard) return;

        float floatOffset = sinf(m_timer * m_floatSpeed + m_phaseOffset) * m_floatAmount;
        billboard->SetScreenPosition(Vector2(x, m_baseY + floatOffset));
        // 基準サイズ × スケールで上書き
        billboard->SetSize(m_baseWidth * m_scale, m_baseHeight * m_scale);
    }
};