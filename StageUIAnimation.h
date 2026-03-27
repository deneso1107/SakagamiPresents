#pragma once
#include <memory>
#include "ScreenFixedBillboard.h"

class StageUIAnimator
{
private:
    // アニメーション状態
    enum class State { IDLE, ANIMATING };
    State m_state = State::IDLE;

    float m_timer = 0.0f;
    float m_duration = 0.4f;   // アニメーション時間（秒）
    float m_slideDir = 1.0f;   // 1.0f=右から, -1.0f=左から

    // 各パラメータの現在値
    Vector2 m_currentPos;
    float   m_currentAlpha = 1.0f;
    float   m_currentAngle = 0.0f;

    // 目標値
    Vector2 m_targetPos = Vector2(0.5f, 0.9f); // 画面中央下
    float   m_slideOffset = 0.3f; // スライド開始オフセット（画面比率）
    float   m_maxAngle = 15.0f; // 最大回転角度（度）

    // Ease関数
    float EaseOutBack(float t) {
        float c1 = 1.70158f;
        float c3 = c1 + 1.0f;
        return 1.0f + c3 * powf(t - 1.0f, 3.0f) + c1 * powf(t - 1.0f, 2.0f);
    }
    float EaseOutCubic(float t) {
        return 1.0f - powf(1.0f - t, 3.0f);
    }

public:
    // 方向を指定してアニメーション開始（1.0f=右入力, -1.0f=左入力）
    void Play(float direction) {
        m_slideDir = direction;
        m_timer = 0.0f;
        m_state = State::ANIMATING;
    }

    void Update(float deltaTime) {
        if (m_state == State::IDLE) return;

        m_timer += deltaTime;
        float t = std:: min(m_timer / m_duration, 1.0f); // 0.0〜1.0

        // スライド：入力方向の逆から飛んでくる
        float easedT = EaseOutCubic(t);
        float startX = m_targetPos.x + (m_slideDir) * m_slideOffset; // 逆方向からスタート
        m_currentPos.x = startX + (m_targetPos.x - startX) * easedT;
        m_currentPos.y = m_targetPos.y;

        // フェードイン
        m_currentAlpha = easedT;

        // 回転：最初に傾いて徐々に戻る
        float angleEase = EaseOutBack(t);
        m_currentAngle = m_maxAngle * (-m_slideDir) * (1.0f - angleEase);

        // アニメーション完了
        if (t >= 1.0f) {
            m_currentPos = m_targetPos;
            m_currentAlpha = 1.0f;
            m_currentAngle = 0.0f;
            m_state = State::IDLE;
        }
    }

    // ビルボードに適用
    void Apply(ScreenFixedBillboard* billboard) {
        if (!billboard) return;
        billboard->SetScreenPosition(m_currentPos);
        billboard->SetAngle(m_currentAngle);
        // アルファは別途対応（後述）
    }

    bool IsPlaying() const { return m_state == State::ANIMATING; }

    void ForceSetPosition(const Vector2& pos) {
        m_targetPos = pos;
        m_currentPos = pos;
        m_currentAngle = 0.0f;
        m_currentAlpha = 1.0f;
        m_state = State::IDLE;
    }
    void SetTargetPosition(const Vector2& pos) {
        m_targetPos = pos;
	}
};
