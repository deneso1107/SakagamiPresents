#include "IntroCamera.h"

IntroCamera& IntroCamera::Instance()
{
    static IntroCamera instance;
    return instance;
}


void IntroCamera::Init() {
    // 基底クラスの初期化を呼び出す（もしあれば）
    Camera::Init();

    // IntroCamera固有の初期化
    m_currentPhase = Phase::HighAngle;
    m_fov = 45.0f;
    m_cameraShakeIntensity = 0.0f;
    m_shakeOffset = Vector3(0.0f, 0.0f, 0.0f);

    // 初期カメラ位置（適当な位置、Updateで上書きされる）
    m_position = Vector3(0.0f, 0.0f, -30.0f);
    m_lookat = Vector3(0.0f, 0.0f, 0.0f);

    // フェーズ遷移の閾値を設定
    m_phaseTransitionProgress[0] = 0.0f;   // HighAngle開始
    m_phaseTransitionProgress[1] = 0.3f;   // Descent開始
    m_phaseTransitionProgress[2] = 0.7f;   // GroundApproach開始
    m_phaseTransitionProgress[3] = 0.95f;  // Landing開始

    m_currentCamera = &SpringCamera::Instance();
    printf("IntroCamera Initialized\n");
}

void IntroCamera::Update(float deltaTime) {
    if (!m_targetPlayer) return;

    // フェーズの自動更新
    UpdatePhase();

    // 各フェーズに応じたカメラ更新
    switch (m_currentPhase) {
    case Phase::HighAngle:
        UpdateHighAngleCamera(deltaTime);
        break;
    case Phase::Descent:
        UpdateDescentCamera(deltaTime);
        break;
    case Phase::GroundApproach:
        UpdateGroundApproachCamera(deltaTime);
        break;
    case Phase::Landing:
        UpdateLandingCamera(deltaTime);
        break;
    case Phase::Transition:
        UpdateTransitionCamera(deltaTime);
        break;
    }

    // カメラシェイクの減衰
    m_cameraShakeIntensity *= 0.9f;
    if (m_cameraShakeIntensity < 0.01f)
    {
        m_cameraShakeIntensity = 0.0f;
        m_shakeOffset = Vector3(0.0f, 0.0f, 0.0f);
    }
    //m_currentCamera->Update(deltaTime);
}

void IntroCamera::UpdatePhase() {
    float progress = GetPlayerProgress();

    // Playerが螺旋降下中の場合
    if (m_targetPlayer->GetStateManager().IsSpiralDescending()) {
        if (progress < m_phaseTransitionProgress[1]) {
            m_currentPhase = Phase::HighAngle;
        }
        else if (progress < m_phaseTransitionProgress[2]) {
            m_currentPhase = Phase::Descent;
        }
        else if (progress < m_phaseTransitionProgress[3]) {
            m_currentPhase = Phase::GroundApproach;
        }
        else {
            m_currentPhase = Phase::Landing;
        }
    }
    // カウントダウン状態になったら遷移フェーズへ
    else if (m_targetPlayer->GetStateManager().IsCountdown()) {
        m_currentPhase = Phase::Transition;
    }
}

float IntroCamera::GetPlayerProgress() {
    // Playerのm_spiralTimeとm_spiralDurationから進行度を計算
    // ※Playerクラスに公開getterを追加する必要があります
    float spiralTime = m_targetPlayer->GetSpiralTime();
    float spiralDuration = m_targetPlayer->GetSpiralDuration();

    if (spiralDuration <= 0.0f) return 0.0f;
    return std::min(spiralTime / spiralDuration, 1.0f);
}

void IntroCamera::UpdateHighAngleCamera(float deltaTime) {
    Vector3 playerPos = m_targetPlayer->GetPosition();

    // 高い位置から斜め下を見下ろす
    m_position = playerPos;
    m_position.y -= 10.0f;  // プレイヤーより30m上
    m_position.z -= 60.0f;  // 少し後ろ
    m_position.x += 10.0f;  // 少し横

    // プレイヤーの少し先を見る
    m_lookat = playerPos;
    m_lookat.z += 5.0f;

    // 広めのFOVでスケール感を出す
    m_fov = 50.0f;
}

void IntroCamera::UpdateDescentCamera(float deltaTime) {
    Vector3 playerPos = m_targetPlayer->GetPosition();
    float progress = GetPlayerProgress();

    // 徐々にカメラを下げていく
    float heightRatio = Lerp(1.0f, 0.3f, (progress - 0.3f) / 0.4f);

    m_position = playerPos;
    m_position.y -= 1.0f * heightRatio;
    m_position.z -= 60.0f + (5.0f * (1.0f - heightRatio));
    m_position.x += 5.0f * heightRatio;

    m_lookat = playerPos;
    m_lookat.z += 10.0f;

    // FOVを徐々に広げて速度感
    m_fov = Lerp(50.0f, 60.0f, (progress - 0.3f) / 0.4f);
}

void IntroCamera::UpdateGroundApproachCamera(float deltaTime) {
    Vector3 playerPos = m_targetPlayer->GetPosition();
    float progress = GetPlayerProgress();

    // 地面近くから見上げる（あなたの要望通り）
    m_position = playerPos;
    m_position.y = 2.0f;  // 地面から2m
    m_position.z -= 8.0f;  // プレイヤーの前方

    // 少し上を向いて、プレイヤーと前方のコースを見る
    m_lookat = playerPos;
    m_lookat.y += 1.0f;   // 少し上
    m_lookat.z += 15.0f;  // 前方のコース

    // 臨場感のためFOVを広く
    float velocityY = abs(m_targetPlayer->GetVelocity().y);
    m_fov = 65.0f + (velocityY * 0.5f);  // 落下速度で変化
    printf("e");
}

void IntroCamera::UpdateLandingCamera(float deltaTime) {
    // 着地の瞬間のカメラ
    Vector3 playerPos = m_targetPlayer->GetPosition();


    // 着地検出（progressが1.0になった瞬間）
    static bool landingTriggered = false;
    if (!landingTriggered) {
        // カメラシェイク発動
        m_cameraShakeIntensity = 0.5f;
        landingTriggered = true;
        printf("=== Camera: Landing Impact! ===\n");
    }

    // 地面視点を維持
    m_shakeOffset = CalculateCameraShake(m_cameraShakeIntensity);
    m_position = playerPos;
    m_position.y = 2.0f;
    m_position.z -= 8.0f;
    m_position += m_shakeOffset;

    m_lookat = playerPos;
    m_lookat.y += 1.0f;
    m_lookat.z += 15.0f;

    m_fov = 60.0f;

    // フラグリセット
    if (m_targetPlayer->GetStateManager().IsCountdown()) {
        landingTriggered = false;
    }
}

void IntroCamera::UpdateTransitionCamera(float deltaTime)
{
    if (!m_targetPlayer) return;

    SpringCamera& springCam = SpringCamera::Instance();
    springCam.Update(deltaTime);

    // ★★★ SpringCameraのスプリング状態を取得 ★★★
    const Spring& posSpring = springCam.GetPositionSpring();
    const Spring& lookSpring = springCam.GetLookAtSpring();

    Vector3 springTargetPos = posSpring.target;   // 目標位置
    Vector3 springActualPos = posSpring.position; // 実際の位置（スプリング適用済み）

    // 遷移進行度
    m_transitionProgress += deltaTime * 0.5f;
    m_transitionProgress = std::min(m_transitionProgress, 1.0f);

    float t = m_transitionProgress;
    float easedProgress = t * t * (3.0f - 2.0f * t);

    // ★★★ SpringCameraの実際の位置（スプリング適用済み）に向かって補間 ★★★
    m_position = Lerp3(m_position, springActualPos, easedProgress);
    m_lookat = Lerp3(m_lookat, lookSpring.position, easedProgress);

    float springFOV = springCam.GetCurrentFOV();
    m_fov = Lerp(m_fov, springFOV, easedProgress);

    // 完了判定
    float positionDiff = (m_position - springActualPos).Length();
    float lookatDiff = (m_lookat - lookSpring.position).Length();

    if (m_transitionProgress >= 0.98f &&
        positionDiff < 1.0f &&
        lookatDiff < 0.5f &&
        !m_isIntroFinished)
    {
        m_isIntroFinished = true;
        printf("=== IntroCamera: Transition Complete! ===\n");
    }
}

float IntroCamera::Vector3Length(const Vector3& v) {
    return sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
}

Vector3 IntroCamera::CalculateCameraShake(float intensity) {
    // ランダムなシェイク
    float x = ((rand() % 200 - 100) / 100.0f) * intensity;
    float y = ((rand() % 200 - 100) / 100.0f) * intensity;
    float z = ((rand() % 200 - 100) / 100.0f) * intensity;
    return Vector3(x, y, z);
}

Vector3 IntroCamera::Lerp3(const Vector3& start, const Vector3& end, float t) const
{
    t = std::max(0.0f, std::min(1.0f, t));
    return Vector3(
        start.x + (end.x - start.x) * t,
        start.y + (end.y - start.y) * t,
        start.z + (end.z - start.z) * t
    );
}

float IntroCamera::Lerp(const float& a, const float& b, float t) const
{
    return a + (b - a) * std::max(0.0f, std::min(1.0f, t));
}