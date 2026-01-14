#pragma once
#include "ScreenFixedBillboard.h"
#include <vector>

// カウントダウンの状態
enum class CountdownState {
    None,
    Show3,
    Show2,
    Show1,
    ShowGo,
    Finished
};

// アニメーションパラメータ
struct CountdownAnimParams {
    // タイミング設定
    float numberDuration = 1.0f;          // 各数字の表示時間
    float goDuration = 0.5f;              // GOの表示時間
    float backgroundSplitDelay = 0.7f;    // 背景分割開始タイミング

    // 数字アニメーション
    float numberAppearTime = 0.15f;       // 出現時間
    float numberScaleStart = 0.8f;        // 初期スケール
    float numberScaleMax = 1.2f;          // 最大スケール
    float numberScaleEnd = 1.0f;          // 最終スケール
    float numberBounceTime = 0.2f;        // バウンス時間

    // 背景アニメーション
    float bgFadeInTime = 0.15f;           // 背景フェードイン
    float bgInitialScale = 0.5f;          // 背景初期スケール
    float bgSplitDuration = 0.3f;         // 分割飛散時間

    // 飛散パラメータ（スクリーン座標系）
    Vector2 topRightVelocity = Vector2(0.4f, -0.4f);     // 右上方向
    Vector2 bottomLeftVelocity = Vector2(-0.4f, 0.4f);   // 左下方向
    float rotationSpeed = 360.0f;         // 回転速度（度/秒）

    // GO演出
    float goSlideDistance = 0.3f;         // スライド距離
    float goScaleMax = 1.3f;              // GOの最大スケール
    float goBounceTime = 0.15f;           // GOバウンス時間
    float goSlideOutDistance = 0.5f;      // スライドアウト距離（左方向）
    float goSlideOutTime = 0.25f;         // スライドアウト時間
};

// 背景ピースの情報
struct BackgroundPiece {
    ScreenFixedBillboard* billboard;
    Vector2 velocity;           // 移動速度
    float rotationSpeed;        // 回転速度
    Vector2 initialPos;         // 初期位置
    float initialRotation;      // 初期回転

    BackgroundPiece() : billboard(nullptr), velocity(0, 0),
        rotationSpeed(0), initialPos(0, 0), initialRotation(0) {
    }
};

class CountdownEffect {
private:
    // ビルボード
    ScreenFixedBillboard* m_numberBillboard;     // 3,2,1用
    ScreenFixedBillboard* m_goBillboard;         // GO用
    BackgroundPiece m_bgTopRight;                // 右上背景
    BackgroundPiece m_bgBottomLeft;              // 左下背景

    // 状態管理
    CountdownState m_currentState;
    float m_stateTimer;
    CountdownAnimParams m_params;

    // 位置情報
    Vector2 m_numberPosition;    // 数字表示位置（中央の白枠）
    Vector2 m_goPosition;        // GO表示位置（左の白枠）

    // サイズ情報
    float m_numberWidth;
    float m_numberHeight;
    float m_goWidth;
    float m_goHeight;
    float m_bgWidth;
    float m_bgHeight;

    // アニメーション用変数
    float m_currentNumberScale;
    float m_currentNumberAlpha;
    float m_currentBgAlpha;
    Vector2 m_currentGoPos;
    float m_currentGoScale;
    float m_currentGoAlpha;

    bool m_isActive;

public:
    CountdownEffect();
    ~CountdownEffect();

    // 初期化
    void Initialize(
        const Vector2& numberPos,      // 数字の表示位置
        const Vector2& goPos,          // GOの表示位置
        float numberSize = 0.15f,      // 数字のサイズ（スクリーン比率）
        float goSize = 0.12f,          // GOのサイズ
        float bgSize = 0.2f            // 背景のサイズ
    );

    // 開始
    void Start();

    // 更新
    void Update(float deltaTime);

    // 描画
    void Draw();

    // 状態取得
    bool IsFinished() const { return m_currentState == CountdownState::Finished; }
    bool IsActive() const { return m_isActive; }
    CountdownState GetCurrentState() const { return m_currentState; }

    // パラメータ設定
    void SetAnimParams(const CountdownAnimParams& params) { m_params = params; }

    // 解放
    void Release();

private:
    // 状態遷移
    void TransitionToState(CountdownState newState);

    // 各状態の更新
    void UpdateNumber(float t);           // 数字アニメーション
    void UpdateBackground(float t);       // 背景アニメーション
    void UpdateGo(float t);               // GOアニメーション

    // ユーティリティ
    float EaseOutBack(float t);           // イージング関数
    float EaseInQuad(float t);
    float EaseOutQuad(float t);
    float EaseInOutQuad(float t);

    // テクスチャパス取得
    const wchar_t* GetNumberTexturePath(int number);
};
