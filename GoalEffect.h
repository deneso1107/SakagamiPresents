#pragma once
#include "ScreenFixedBillboard.h"
#include "system/commontypes.h"

// ゴール演出の状態（UI専用）
enum class GoalPhase {
    None,
    Flash,          // 画面フラッシュ
    GoalText,       // "GOAL!" テキスト表示
    FadeOut,        // フェードアウト
    Finished
};

// ゴール演出パラメータ
struct GoalEffectParams {
    // タイミング設定
    float flashDuration = 0.15f;        // フラッシュ時間
    float goalTextDuration = 1.5f;      // GOALテキスト表示時間
    float fadeOutDuration = 0.5f;       // フェードアウト時間

    // フラッシュ設定
    float flashMaxAlpha = 0.8f;         // フラッシュの最大不透明度

    // GOALテキスト設定
    float goalTextScaleStart = 0.5f;    // 初期スケール
    float goalTextScaleMax = 1.4f;      // 最大スケール
    float goalTextScaleEnd = 1.0f;      // 最終スケール
    float goalTextBounceTime = 0.3f;    // バウンス時間
};

// UI演出専用クラス（プレイヤーの動きは含まない）
class GoalEffect {
private:
    // ビルボード
    ScreenFixedBillboard* m_flashBillboard;     // 画面フラッシュ用
    ScreenFixedBillboard* m_goalTextBillboard;  // "GOAL!" テキスト用

    // 状態管理
    GoalPhase m_currentPhase;
    float m_phaseTimer;
    float m_totalTimer;
    GoalEffectParams m_params;

    // アニメーション用変数
    float m_currentFlashAlpha;
    float m_currentGoalTextScale;
    float m_currentGoalTextAlpha;

    bool m_isActive;

public:
    GoalEffect();
    ~GoalEffect();

    // 初期化
    void Initialize();

    // ゴール演出開始
    void Start();

    // 更新
    void Update(float deltaTime);

    // 描画（UI要素のみ）
    void Draw();

    // 状態取得
    bool IsFinished() const { return m_currentPhase == GoalPhase::Finished; }
    bool IsActive() const { return m_isActive; }
    GoalPhase GetCurrentPhase() const { return m_currentPhase; }

    // パラメータ設定
    void SetEffectParams(const GoalEffectParams& params) { m_params = params; }

    // 解放
    void Release();

private:
    // 状態遷移
    void TransitionToPhase(GoalPhase newPhase);

    // 各フェーズの更新
    void UpdateFlash(float t);
    void UpdateGoalText(float t);
    void UpdateFadeOut(float t);

    // ユーティリティ
    float EaseOutBack(float t) const;
    float EaseOutQuad(float t) const;
};