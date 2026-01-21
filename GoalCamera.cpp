#include "GoalCamera.h"
#include <cmath>

GoalCamera& GoalCamera::Instance()
{
    static GoalCamera instance;
    return instance;
}

void GoalCamera::Init()
{
    // 基底クラスの初期化
    Camera::Init();

    // GoalCamera固有の初期化
    m_currentPhase = Phase::FollowClose;
    m_phaseTimer = 0.0f;
    m_totalTime = 0.0f;

    // 初期カメラ設定
    m_fov = 45.0f;
    m_currentDistance = 10.0f;
    m_currentHeight = 5.0f;
    m_pullBackSpeed = 1.0f;

    printf("GoalCamera Initialized\n");
}

void GoalCamera::StartGoalSequence()
{
    if (!m_targetPlayer) return;

    // ★★★ 現在のプレイヤー状態を保存 ★★★
    Vector3 playerPos = m_targetPlayer->GetPosition();
    Vector3 playerDir = m_targetPlayer->GetForwardVector();

    // ★★★ カメラ位置を固定（ゴール時点の位置から変えない） ★★★
    m_initialPosition = playerPos - playerDir * 12.0f;  // プレイヤーの後ろ12m
    m_initialPosition.y = playerPos.y + 5.0f;           // 5m上

    // 少し横にずらす（より良いアングル）
    Vector3 rightDir = Vector3(-playerDir.z, 0, playerDir.x);
    rightDir.Normalize();
    m_initialPosition += rightDir * 3.0f;  // 右に3m

    // ★★★ この位置に固定！ ★★★
    m_position = m_initialPosition;

    // 初期注視点
    m_initialLookAt = playerPos;
    m_initialLookAt.y += 2.0f;
    m_lookat = m_initialLookAt;

    m_initialFOV = 45.0f;
    m_fov = m_initialFOV;

    // タイマーリセット
    m_currentPhase = Phase::FollowClose;
    m_phaseTimer = 0.0f;
    m_totalTime = 0.0f;

    printf("=== GoalCamera: Fixed Position Camera Started! ===\n");
    printf("Camera Position: (%.2f, %.2f, %.2f)\n", m_position.x, m_position.y, m_position.z);
}

void GoalCamera::Update(float deltaTime)
{
    if (!m_targetPlayer) return;

    m_phaseTimer += deltaTime;
    m_totalTime += deltaTime;

    // フェーズ更新
    UpdatePhase(deltaTime);

    // 各フェーズに応じたカメラ更新
    switch (m_currentPhase) {
    case Phase::FollowClose:
        UpdateFollowClose(deltaTime);
        break;
    case Phase::PullBackSlow:
        UpdatePullBackSlow(deltaTime);
        break;
    case Phase::PullBackFast:
        UpdatePullBackFast(deltaTime);
        break;
    case Phase::WideView:
        UpdateWideView(deltaTime);
        break;
    case Phase::Finished:
        // 何もしない
        break;
    }
}

void GoalCamera::UpdatePhase(float deltaTime)
{
    // フェーズ自動遷移
    switch (m_currentPhase) {
    case Phase::FollowClose:
        if (m_phaseTimer >= m_followCloseDuration) {
            m_currentPhase = Phase::PullBackSlow;
            m_phaseTimer = 0.0f;
            printf("=== GoalCamera: Phase -> PullBackSlow ===\n");
        }
        break;

    case Phase::PullBackSlow:
        if (m_phaseTimer >= m_pullBackSlowDuration) {
            m_currentPhase = Phase::PullBackFast;
            m_phaseTimer = 0.0f;
            printf("=== GoalCamera: Phase -> PullBackFast ===\n");
        }
        break;

    case Phase::PullBackFast:
        if (m_phaseTimer >= m_pullBackFastDuration) {
            m_currentPhase = Phase::WideView;
            m_phaseTimer = 0.0f;
            printf("=== GoalCamera: Phase -> WideView ===\n");
        }
        break;

    case Phase::WideView:
        if (m_phaseTimer >= m_wideViewDuration) {
            m_currentPhase = Phase::Finished;
            m_phaseTimer = 0.0f;
            printf("=== GoalCamera: Sequence Finished! ===\n");
        }
        break;

    default:
        break;
    }
}

void GoalCamera::UpdateFollowClose(float deltaTime)
{
    // ★★★ フェーズ1: カメラ位置は固定、プレイヤーを追う ★★★

    // ★位置は完全に固定★
    m_position = m_initialPosition;

    // プレイヤーの現在位置を見る
    Vector3 playerPos = m_targetPlayer->GetPosition();
    Vector3 targetLookAt = playerPos;
    targetLookAt.y += 2.0f;  // プレイヤーの中心

    // 視線をスムーズに追従
    float followSpeed = 8.0f;
    float t = std::min(1.0f, deltaTime * followSpeed);
    m_lookat = Lerp3(m_lookat, targetLookAt, t);

    // FOVは標準
    m_fov = 45.0f;
}

void GoalCamera::UpdatePullBackSlow(float deltaTime)
{
    // ★★★ フェーズ2: 位置固定、FOVをゆっくり広げる ★★★
    float t = m_phaseTimer / m_pullBackSlowDuration;
    t = EaseOutQuad(t);

    // ★位置は完全に固定★
    m_position = m_initialPosition;

    // プレイヤーを追い続ける
    Vector3 playerPos = m_targetPlayer->GetPosition();
    Vector3 targetLookAt = playerPos;
    targetLookAt.y += Lerp(2.0f, 5.0f, t);  // 徐々に高い位置を見る

    // 視線をスムーズに更新
    float smoothSpeed = 5.0f;
    float smoothT = std::min(1.0f, deltaTime * smoothSpeed);
    m_lookat = Lerp3(m_lookat, targetLookAt, smoothT);

    // FOVをゆっくり広げる
    m_fov = Lerp(45.0f, 60.0f, t);
}

void GoalCamera::UpdatePullBackFast(float deltaTime)
{
    // ★★★ フェーズ3: 位置固定、FOVを急速に広げる ★★★
    float t = m_phaseTimer / m_pullBackFastDuration;
    t = EaseInOutQuad(t);

    // ★位置は完全に固定★
    m_position = m_initialPosition;

    // プレイヤーを追い続ける
    Vector3 playerPos = m_targetPlayer->GetPosition();
    Vector3 targetLookAt = playerPos;
    targetLookAt.y += Lerp(5.0f, 10.0f, t);  // さらに高い位置を見る

    // 視線をスムーズに更新
    float smoothSpeed = 4.0f;
    float smoothT = std::min(1.0f, deltaTime * smoothSpeed);
    m_lookat = Lerp3(m_lookat, targetLookAt, smoothT);

    // FOVを急速に広げる
    m_fov = Lerp(60.0f, 80.0f, t);
}

void GoalCamera::UpdateWideView(float deltaTime)
{
    // ★★★ フェーズ4: 位置固定、最大FOVで全体を見せる ★★★

    // ★位置は完全に固定★
    m_position = m_initialPosition;

    // プレイヤーを追い続ける
    Vector3 playerPos = m_targetPlayer->GetPosition();
    Vector3 targetLookAt = playerPos;
    targetLookAt.y += 15.0f;  // かなり高い位置を見る

    // 視線をゆっくり更新
    float smoothSpeed = 2.0f;
    float smoothT = std::min(1.0f, deltaTime * smoothSpeed);
    m_lookat = Lerp3(m_lookat, targetLookAt, smoothT);

    // 最大FOV
    float t = m_phaseTimer / m_wideViewDuration;
    m_fov = Lerp(80.0f, 85.0f, t);
}

// ユーティリティ関数
Vector3 GoalCamera::Lerp3(const Vector3& start, const Vector3& end, float t) const
{
    t = std::max(0.0f, std::min(1.0f, t));
    return Vector3(
        start.x + (end.x - start.x) * t,
        start.y + (end.y - start.y) * t,
        start.z + (end.z - start.z) * t
    );
}

float GoalCamera::Lerp(float a, float b, float t) const
{
    return a + (b - a) * std::max(0.0f, std::min(1.0f, t));
}

float GoalCamera::EaseOutQuad(float t) const
{
    return 1.0f - (1.0f - t) * (1.0f - t);
}

float GoalCamera::EaseInOutQuad(float t) const
{
    return t < 0.5f ? 2.0f * t * t : 1.0f - pow(-2.0f * t + 2.0f, 2.0f) / 2.0f;
}