#include "ChaseCamera.h"
CheeseCamera::CheeseCamera()
    : m_targetPlayer(nullptr)
    , m_currentOffset(0.0f, 0.0f, 0.0f)
    , m_targetOffset(0.0f, 0.0f, 0.0f)
    , m_currentLookAt(0.0f, 0.0f, 0.0f)
    , m_targetLookAt(0.0f, 0.0f, 0.0f)
    , m_currentDynamicDistance(0.0f)
    , m_currentDynamicFOV(0.0f)
{
}

void CheeseCamera::Init()
{
    Camera::Init();

    if (m_targetPlayer) {
        Vector3 playerPos = m_targetPlayer->GetPosition();
        Vector3 playerRot = m_targetPlayer->GetRotation();

        Vector3 backward = Vector3(-sinf(playerRot.y), 0.0f, -cosf(playerRot.y));
        m_currentOffset = backward * m_distance;
        m_currentOffset.y = m_height;

        m_position = playerPos + m_currentOffset;
        m_currentLookAt = playerPos + Vector3(0.0f, 2.0f, 0.0f);
        m_lookat = m_currentLookAt;

        // カメラパラメータの初期設定
        SetDistance(75.0f);
        SetHeight(6.0f);
        SetFollowSpeed(0.1f);
        SetLookAtSpeed(0.15f);

        // 加速演出パラメータの設定
        SetBoostDistance(55.0f);      // 加速時はさらに引く
        SetBoostFOV(70.0f);           // 加速時はFOVを広げる
        SetBoostSpeedThreshold(50.0f);// この速度から演出開始
        SetCameraTransitionSpeed(0.08f); // 演出の遷移速度
    }
}

void CheeseCamera::Update(float deltaTime)
{
    if (!m_targetPlayer) {
        return;
    }

    // 速度に基づく動的パラメータの更新
    UpdateDynamicCameraParameters();

    // 目標位置と注視点を計算
    Vector3 targetPos = CalculateTargetPosition();
    Vector3 targetLookAt = CalculateTargetLookAt();

    // スムーズに追従
    m_position = Lerp3(m_position, targetPos, m_followSpeed);
    m_currentLookAt = Lerp3(m_currentLookAt, targetLookAt, m_lookAtSpeed);
    m_lookat = m_currentLookAt;

    //揺らす処理
    if (m_isShaking && m_shakeDuration > 0.0f)
    {
        // ランダムなオフセットを追加
        float randomX = ((rand() % 200 - 100) / 100.0f) * m_shakeIntensity;
        float randomY = ((rand() % 200 - 100) / 100.0f) * m_shakeIntensity;
        float randomZ = ((rand() % 200 - 100) / 100.0f) * m_shakeIntensity;

        m_position = m_originalPosition + Vector3(randomX, randomY, randomZ);

        m_shakeDuration -= deltaTime;

        // シェイク終了
        if (m_shakeDuration <= 0.0f)
        {
            m_position = m_originalPosition;
            m_isShaking = false;
        }
    }
    else if (!m_isShaking)
    {
        m_originalPosition = m_position;  // 常に更新
    }
}

void CheeseCamera::Draw()
{
    // ビュー変換行列作成
    Vector3 up = Vector3(0.0f, 1.0f, 0.0f);
    m_viewmtx = DirectX::XMMatrixLookAtLH(
        m_position,
        m_lookat,
        up);
    Renderer::SetViewMatrix(&m_viewmtx);

    // 動的FOVの計算(通常FOV + 速度による追加FOV)
    float currentFOV = m_normalFOV + m_currentDynamicFOV;
    float fieldOfView = DirectX::XMConvertToRadians(currentFOV);

    float aspectRatio = static_cast<float>(Application::GetWidth()) /
        static_cast<float>(Application::GetHeight());
    float nearPlane = 0.1f;
    float farPlane = 3000.0f;

    m_projmtx = DirectX::XMMatrixPerspectiveFovLH(
        fieldOfView,
        aspectRatio,
        nearPlane,
        farPlane);
    Renderer::SetProjectionMatrix(&m_projmtx);
}

float CheeseCamera::CalculateSpeedRatio() const
{
    Vector3 playerVel = m_targetPlayer->GetVelocity();
    float speed = sqrt(playerVel.x * playerVel.x + playerVel.z * playerVel.z);

    // 閾値以下なら0.0、最大速度以上なら1.0
    if (speed <= m_boostSpeedThreshold) {
        return 0.0f;
    }
    if (speed >= m_maxBoostSpeed) {
        return 1.0f;
    }

    // 閾値と最大速度の間で0.0〜1.0に正規化
    float ratio = (speed - m_boostSpeedThreshold) /
        (m_maxBoostSpeed - m_boostSpeedThreshold);
    return std::max(0.0f, std::min(1.0f, ratio));
}

void CheeseCamera::UpdateDynamicCameraParameters()
{
    // 現在の速度比率を取得
    float speedRatio = CalculateSpeedRatio();

    // 目標の追加距離とFOVを計算
    float targetDistance = (m_boostDistance - m_normalDistance) * speedRatio;
    float targetFOV = (m_boostFOV - m_normalFOV) * speedRatio;

    // スムーズに遷移
    m_currentDynamicDistance = Lerp(m_currentDynamicDistance, targetDistance,
        m_cameraTransitionSpeed);
    m_currentDynamicFOV = Lerp(m_currentDynamicFOV, targetFOV,
        m_cameraTransitionSpeed);
}

Vector3 CheeseCamera::CalculateTargetPosition() const
{
    Vector3 playerPos = m_targetPlayer->GetPosition();
    Vector3 playerRot = m_targetPlayer->GetRotation();
    Vector3 playerVel = m_targetPlayer->GetVelocity();

    // プレイヤーの後ろ方向を計算
    Vector3 backward = Vector3(-sinf(playerRot.y), 0.0f, -cosf(playerRot.y));

    // 速度に応じた先読み距離
    float speed = sqrt(playerVel.x * playerVel.x + playerVel.z * playerVel.z);
    float anticipation = std::min(speed * 0.3f, 3.0f);

    // 基本距離 + 動的距離 + 先読み距離
    float totalDistance = m_normalDistance + m_currentDynamicDistance + anticipation;

    Vector3 targetOffset = backward * totalDistance;
    targetOffset.y = m_height;

    return playerPos + targetOffset;
}

Vector3 CheeseCamera::CalculateTargetLookAt() const
{
    Vector3 playerPos = m_targetPlayer->GetPosition();
    Vector3 playerVel = m_targetPlayer->GetVelocity();

    Vector3 lookAtPos = playerPos + Vector3(0.0f, 2.0f, 0.0f);

    // 速度に応じて注視点を前方に移動
    float speed = sqrt(playerVel.x * playerVel.x + playerVel.z * playerVel.z);
    if (speed > 0.1f) {
        Vector3 velocityDir = playerVel * (1.0f / speed);

        // 加速時はさらに前方を注視
        float lookAheadDistance = std::min(speed * 0.5f, 5.0f);
        float speedRatio = CalculateSpeedRatio();
        lookAheadDistance += speedRatio * 3.0f; // 加速時は最大3.0単位追加

        lookAtPos += velocityDir * lookAheadDistance;
    }

    return lookAtPos;
}

void CheeseCamera::Shake(float intensity, float duration)
{
    m_shakeIntensity = intensity;
    m_shakeDuration = duration;
    m_isShaking = true;
    m_originalPosition = m_position;  // 現在位置を保存
}

Vector3 CheeseCamera::Lerp3(const Vector3& start, const Vector3& end, float t) const
{
    t = std::max(0.0f, std::min(1.0f, t));
    return Vector3(
        start.x + (end.x - start.x) * t,
        start.y + (end.y - start.y) * t,
        start.z + (end.z - start.z) * t
    );
}

float CheeseCamera::Lerp(float start, float end, float t) const
{
    t = std::max(0.0f, std::min(1.0f, t));
    return start + (end - start) * t;
}