#pragma once
#include "Camera.h"
#include "Player.h"
#include <algorithm>

// ゴール演出専用カメラ
// ゆっくり引いていくドラマチックな演出
class GoalCamera : public Camera {
public:
    // フェーズ定義
    enum class Phase {
        FollowClose,        // ゴール直後：プレイヤーに密着
        PullBackSlow,       // ゆっくり引き始める
        PullBackFast,       // 加速して引いていく
        WideView,           // 最終的に広い視野で全体を見せる
        Finished
    };

private:
    // シングルトン
    GoalCamera() = default;
    ~GoalCamera() = default;

public:
    // コピー/ムーブ禁止
    GoalCamera(const GoalCamera&) = delete;
    GoalCamera& operator=(const GoalCamera&) = delete;
    GoalCamera(GoalCamera&&) = delete;
    GoalCamera& operator=(GoalCamera&&) = delete;

    static GoalCamera& Instance();

    void Init() override;
    void Update(float deltaTime) override;

    // プレイヤー設定
    void SetTargetPlayer(Player* player) { m_targetPlayer = player; }

    // ゴール演出開始
    void StartGoalSequence();

    // 演出完了確認
    bool IsFinished() const { return m_currentPhase == Phase::Finished; }
    Phase GetCurrentPhase() const { return m_currentPhase; }

private:
    Player* m_targetPlayer = nullptr;
    Phase m_currentPhase = Phase::FollowClose;

    // タイマー
    float m_phaseTimer = 0.0f;
    float m_totalTime = 0.0f;

    // フェーズ切り替えタイミング（秒）
    float m_followCloseDuration = 0.5f;      // 密着フェーズ
    float m_pullBackSlowDuration = 1.0f;     // ゆっくり引く
    float m_pullBackFastDuration = 1.5f;     // 加速して引く
    float m_wideViewDuration = 1.0f;         // 広い視野

    // カメラパラメータ
    Vector3 m_initialPosition;     // 開始位置
    Vector3 m_initialLookAt;       // 開始注視点
    float m_initialFOV = 45.0f;

    Vector3 m_targetOffset;        // プレイヤーからのオフセット
    float m_fov = 4.5f;
    float m_currentDistance = 10.0f;   // プレイヤーからの距離
    float m_currentHeight = 5.0f;      // プレイヤーからの高さ
    float m_pullBackSpeed = 1.0f;      // 引く速度

    // フェーズ更新関数
    void UpdatePhase(float deltaTime);
    void UpdateFollowClose(float deltaTime);
    void UpdatePullBackSlow(float deltaTime);
    void UpdatePullBackFast(float deltaTime);
    void UpdateWideView(float deltaTime);

    // ユーティリティ
    Vector3 Lerp3(const Vector3& start, const Vector3& end, float t) const;
    float Lerp(float a, float b, float t) const;
    float EaseOutQuad(float t) const;
    float EaseInOutQuad(float t) const;
};

