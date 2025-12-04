#include "ChaseCamera.h"

// ===================================
// Spring実装
// ===================================
void Spring::Update(float deltaTime)
{
    // F = -k * x - d * v
    // k: stiffness(バネ定数)
    // d: damping(減衰係数)
    // x: displacement(変位)
    // v: velocity(速度)

    Vector3 displacement = position - target;
    Vector3 springForce = displacement * (-stiffness);
    Vector3 dampingForce = velocity * (-damping);
    Vector3 acceleration = springForce + dampingForce;

    velocity += acceleration * deltaTime;
    position += velocity * deltaTime;
}

void Spring::Snap()
{
    position = target;
    velocity = Vector3(0, 0, 0);
}

// ===================================
// CheeseCamera実装
// ===================================
SpringCamera::SpringCamera()
    : m_targetPlayer(nullptr)
    , m_currentBank(0.0f)
    , m_currentFOV(45.0f)
{
    // 通常パラメータ
    m_normalParams.distance = 75.0f;
    m_normalParams.height = 6.0f;
    m_normalParams.fov = 45.0f;
    m_normalParams.anticipation = 0.3f;
    m_normalParams.lookAheadDist = 0.5f;
    m_normalParams.positionStiffness = 150.0f;
    m_normalParams.positionDamping = 20.0f;
    m_normalParams.lookAtStiffness = 200.0f;
    m_normalParams.lookAtDamping = 25.0f;
	m_normalParams.name = 'N';

    // 加速パラメータ
    m_boostParams.distance = 55.0f;
    m_boostParams.height = 8.0f;
    m_boostParams.fov = 55.0f;
    m_boostParams.anticipation = 0.5f;
    m_boostParams.lookAheadDist = 0.8f;
    m_boostParams.positionStiffness = 100.0f;  // 少し柔らかく
    m_boostParams.positionDamping = 18.0f;
    m_boostParams.lookAtStiffness = 180.0f;
    m_boostParams.lookAtDamping = 22.0f;
	m_boostParams.name = 'B';

    m_currentParams = m_normalParams;
}

SpringCamera& SpringCamera::Instance()
{
    static SpringCamera instance;
    return instance;
}

void SpringCamera::Init()
{
    Camera::Init();

    if (m_targetPlayer) {
        m_boostSpeedThreshold = m_targetPlayer->GetNormalSpeed();
        m_maxBoostSpeed = m_targetPlayer->GetMaxSpeed();

        // スプリングを初期位置にスナップ
        Vector3 playerPos = m_targetPlayer->GetPosition();
        Vector3 playerRot = m_targetPlayer->GetRotation();
        Vector3 backward = Vector3(-sinf(playerRot.y), 0.0f, -cosf(playerRot.y));

        m_positionSpring.target = playerPos + backward * m_normalParams.distance + Vector3(0, m_normalParams.height, 0);
        m_positionSpring.Snap();

        m_lookAtSpring.target = playerPos + Vector3(0, 2, 0);
        m_lookAtSpring.Snap();

        // スプリングパラメータ設定
        m_positionSpring.stiffness = m_normalParams.positionStiffness;
        m_positionSpring.damping = m_normalParams.positionDamping;
        m_lookAtSpring.stiffness = m_normalParams.lookAtStiffness;
        m_lookAtSpring.damping = m_normalParams.lookAtDamping;
    }
}

void SpringCamera::Update(float deltaTime)
{
    if (!m_targetPlayer) return;

    // 現在の状態に応じたパラメータを取得
    CameraParams targetParams = DetermineTargetParams();
    printf("Target Params: %c Distance: %.2f Height: %.2f FOV: %.2f\n",
        (CalculateSpeedRatio() < 0.5f) ? 'N' : 'B',
        targetParams.distance,
        targetParams.height,
		targetParams.fov);

    // パラメータを滑らかに遷移
    m_currentParams.distance = Lerp(m_currentParams.distance, targetParams.distance, m_transitionSpeed);
    m_currentParams.height = Lerp(m_currentParams.height, targetParams.height, m_transitionSpeed);
    m_currentParams.fov = Lerp(m_currentParams.fov, targetParams.fov, m_transitionSpeed);
    m_currentParams.anticipation = Lerp(m_currentParams.anticipation, targetParams.anticipation, m_transitionSpeed);
    m_currentParams.lookAheadDist = Lerp(m_currentParams.lookAheadDist, targetParams.lookAheadDist, m_transitionSpeed);

    // スプリングパラメータも更新
    m_positionSpring.stiffness = Lerp(m_positionSpring.stiffness, targetParams.positionStiffness, m_transitionSpeed);
    m_positionSpring.damping = Lerp(m_positionSpring.damping, targetParams.positionDamping, m_transitionSpeed);
    m_lookAtSpring.stiffness = Lerp(m_lookAtSpring.stiffness, targetParams.lookAtStiffness, m_transitionSpeed);
    m_lookAtSpring.damping = Lerp(m_lookAtSpring.damping, targetParams.lookAtDamping, m_transitionSpeed);

    // 理想的なカメラ位置を計算
    m_positionSpring.target = CalculateIdealCameraPosition();
    m_lookAtSpring.target = CalculateIdealLookAtPosition();

    // スプリング更新(ここで物理シミュレーション)
    m_positionSpring.Update(deltaTime);
    m_lookAtSpring.Update(deltaTime);

    // 基底クラスに反映
    m_position = m_positionSpring.position;
    m_lookat = m_lookAtSpring.position;

    // エフェクト適用
    ApplyBoostShake(m_position);
    ApplyShake(m_position, deltaTime);

    // カメラバンク
    Vector3 steer = m_targetPlayer->GetRotation();
    float steerNorm = steer.y / 3.14f;
    float maxBankDeg = 1.25f;
    float targetBank = steerNorm * maxBankDeg;
    m_currentBank = Lerp(m_currentBank, targetBank, 0.1f);

    // FOV更新
    m_currentFOV = m_currentParams.fov;
}

void SpringCamera::Draw()
{
    // ビュー変換行列作成
    Vector3 up = Vector3(0.0f, 1.0f, 0.0f);
    m_viewmtx = DirectX::XMMatrixLookAtLH(m_position, m_lookat, up);

    // バンク角適用
    float bankRad = DirectX::XMConvertToRadians(m_currentBank);
    Matrix4x4 bankRot = Matrix4x4::CreateRotationY(bankRad);
    m_viewmtx = bankRot * m_viewmtx;

    Renderer::SetViewMatrix(&m_viewmtx);

    // プロジェクション行列
    float fieldOfView = DirectX::XMConvertToRadians(m_currentFOV);
    float aspectRatio = static_cast<float>(Application::GetWidth()) /
        static_cast<float>(Application::GetHeight());

    m_projmtx = DirectX::XMMatrixPerspectiveFovLH(
        fieldOfView, aspectRatio, 0.1f, 3000.0f);

    Renderer::SetProjectionMatrix(&m_projmtx);
}

// ===== プライベート関数実装 =====

float SpringCamera::CalculateSpeedRatio() const
{
    Vector3 playerVel = m_targetPlayer->GetVelocity();
    float speed = sqrt(playerVel.x * playerVel.x + playerVel.z * playerVel.z);

    if (speed <= m_boostSpeedThreshold) return 0.0f;
    if (speed >= m_maxBoostSpeed) return 1.0f;

    return (speed - m_boostSpeedThreshold) / (m_maxBoostSpeed - m_boostSpeedThreshold);
}

SpringCamera::CameraParams SpringCamera::DetermineTargetParams() const
{
    float speedRatio = CalculateSpeedRatio();

    // 通常と加速のパラメータを補間
    CameraParams result;
    result.distance = Lerp(m_normalParams.distance, m_boostParams.distance, speedRatio);
    result.height = Lerp(m_normalParams.height, m_boostParams.height, speedRatio);
    result.fov = Lerp(m_normalParams.fov, m_boostParams.fov, speedRatio);
    result.anticipation = Lerp(m_normalParams.anticipation, m_boostParams.anticipation, speedRatio);
    result.lookAheadDist = Lerp(m_normalParams.lookAheadDist, m_boostParams.lookAheadDist, speedRatio);

    result.positionStiffness = Lerp(m_normalParams.positionStiffness, m_boostParams.positionStiffness, speedRatio);
    result.positionDamping = Lerp(m_normalParams.positionDamping, m_boostParams.positionDamping, speedRatio);
    result.lookAtStiffness = Lerp(m_normalParams.lookAtStiffness, m_boostParams.lookAtStiffness, speedRatio);
    result.lookAtDamping = Lerp(m_normalParams.lookAtDamping, m_boostParams.lookAtDamping, speedRatio);

    return result;
}

Vector3 SpringCamera::CalculateIdealCameraPosition() const
{
    Vector3 playerPos = m_targetPlayer->GetPosition();
    Vector3 playerRot = m_targetPlayer->GetRotation();
    Vector3 playerVel = m_targetPlayer->GetVelocity();

    // プレイヤーの後ろ方向
    Vector3 backward = Vector3(-sinf(playerRot.y), 0.0f, -cosf(playerRot.y));

    // 速度に応じた先読み(Velocity Extrapolation)
    float speed = sqrt(playerVel.x * playerVel.x + playerVel.z * playerVel.z);
    float anticipation = std::min(speed * m_currentParams.anticipation, 5.0f);

    // カメラの理想位置
    Vector3 idealPos = playerPos + backward * (m_currentParams.distance + anticipation);
    idealPos.y += m_currentParams.height;

    return idealPos;
}

Vector3 SpringCamera::CalculateIdealLookAtPosition() const
{
    Vector3 playerPos = m_targetPlayer->GetPosition();
    Vector3 playerVel = m_targetPlayer->GetVelocity();

    Vector3 lookAt = playerPos + Vector3(0.0f, 2.0f, 0.0f);

    // 速度に応じて前方を注視(Lookahead)
    float speed = sqrt(playerVel.x * playerVel.x + playerVel.z * playerVel.z);
    if (speed > 0.1f) {
        Vector3 velocityDir = playerVel * (1.0f / speed);
        float lookAheadDistance = std::min(speed * m_currentParams.lookAheadDist, 8.0f);
        lookAt += velocityDir * lookAheadDistance;
    }

    return lookAt;
}

void SpringCamera::ApplyShake(Vector3& position, float deltaTime)
{
    if (!m_isShaking || m_shakeDuration <= 0.0f) return;

    Vector3 playerVel = m_targetPlayer->GetVelocity();
    float speed = sqrt(playerVel.x * playerVel.x +
        playerVel.y * playerVel.y +
        playerVel.z * playerVel.z);

    Vector3 shakeOffset(0.0f, 0.0f, 0.0f);

    if (speed > 0.1f) {
        Vector3 moveDir = playerVel * (1.0f / speed);
        float absX = fabsf(moveDir.x);
        float absY = fabsf(moveDir.y);
        float absZ = fabsf(moveDir.z);

        const float threshold = 0.5f;
        if (absX < threshold) {
            shakeOffset.x = ((rand() % 200 - 100) / 100.0f) * m_shakeIntensity;
        }
        if (absY < threshold) {
            shakeOffset.y = ((rand() % 200 - 100) / 100.0f) * m_shakeIntensity;
        }
        if (absZ < threshold) {
            shakeOffset.z = ((rand() % 200 - 100) / 100.0f) * m_shakeIntensity;
        }
    }
    else {
        shakeOffset.x = ((rand() % 200 - 100) / 100.0f) * m_shakeIntensity;
        shakeOffset.y = ((rand() % 200 - 100) / 100.0f) * m_shakeIntensity;
        shakeOffset.z = ((rand() % 200 - 100) / 100.0f) * m_shakeIntensity;
    }

    position += shakeOffset;
    m_shakeDuration -= deltaTime;

    if (m_shakeDuration <= 0.0f) {
        m_isShaking = false;
    }
}

void SpringCamera::ApplyBoostShake(Vector3& position) const
{
    float speedRatio = CalculateSpeedRatio();
    if (speedRatio > 0.1f) {
        float shake = speedRatio * 0.3f;
        position.x += (rand() % 200 - 100) / 100.0f * shake;
        position.y += (rand() % 200 - 100) / 150.0f * shake;
    }
}

float SpringCamera::Lerp(float a, float b, float t) const
{
    return a + (b - a) * std::max(0.0f, std::min(1.0f, t));
}

// ===== 外部インターフェース実装 =====

void SpringCamera::SetTargetPlayer(Player* player)
{
    m_targetPlayer = player;
}

void SpringCamera::Shake(float intensity, float duration)
{
    m_shakeIntensity = intensity;
    m_shakeDuration = duration;
    m_isShaking = true;
}

Vector3 SpringCamera::GetForward() const
{
    Vector3 forward = m_lookat - m_position;
    forward.Normalize();
    return forward;
}

void SpringCamera::SetPositionSpring(float stiffness, float damping)
{
    m_normalParams.positionStiffness = stiffness;
    m_normalParams.positionDamping = damping;
}

void SpringCamera::SetLookAtSpring(float stiffness, float damping)
{
    m_normalParams.lookAtStiffness = stiffness;
    m_normalParams.lookAtDamping = damping;
}