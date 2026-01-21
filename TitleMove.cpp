#include "TitleMove.h"
#include <algorithm>
#include <cmath>

TitleSpiralEffect::TitleSpiralEffect()
    : m_startPosition(0, 0, 0)
    , m_startRotation(0, 0, 0)
    , m_direction(0, 0, 1)
    , m_spiralTime(0.0f)
    , m_spiralDuration(3.5f)
    , m_spiralHeight(60.0f)
    , m_spiralDistance(35.0f)
    , m_spiralRadius(5.0f)
    , m_spiralRotations(3.0f)
    , m_currentPosition(0, 0, 0)
    , m_currentRotation(0, 0, 0)
    , m_currentVelocity(0, 0, 0)
    , m_isActive(false)
    , m_isLooping(false)  // デフォルトでループ
    , m_isInfiniteMode(true)   // デフォルトは無限上昇
    , m_currentCycle(0)
    , m_cycleHeight(60.0f)
    , m_cycleDistance(35.0f)
{
}

void TitleSpiralEffect::Initialize(const Vector3& startPos, const Vector3& startRot, const Vector3& direction)
{
    m_startPosition = startPos;
    m_startRotation = startRot;
    m_direction = direction;
    m_direction.Normalize();

    m_currentPosition = m_startPosition;
    m_currentRotation = m_startRotation;
    m_spiralTime = 0.0f;
}

void TitleSpiralEffect::Start()
{
    m_isActive = true;
    m_spiralTime = 0.0f;
    m_currentCycle = 0;  // サイクルカウンターをリセット
}

void TitleSpiralEffect::Stop()
{
    m_isActive = false;
}

void TitleSpiralEffect::Reset()
{
    m_spiralTime = 0.0f;
    m_currentCycle = 0;  // サイクルカウンターもリセット
    m_currentPosition = m_startPosition;
    m_currentRotation = m_startRotation;
}

void TitleSpiralEffect::Update(float deltaTime)
{
    if (!m_isActive) return;

    // ===== 螺旋演出中 =====
    if (!m_hasFinishedSpiral)
    {
        m_spiralTime += deltaTime;

        float t = std::min(m_spiralTime / m_spiralDuration, 1.0f);

        Vector3 angularVel;

        CalculateSpiralPosition(
            t,
            m_currentPosition,
            m_currentRotation,
            m_currentVelocity,
            angularVel
        );

        // ★ 螺旋が終わった瞬間
        if (t >= 1.0f)
        {
            m_hasFinishedSpiral = true;

            // フェーズ2の「最後の速度」を保存
            m_linearVelocity = m_currentVelocity;
            m_angularVelocity = angularVel; // ★ 追加
        }

        return;
    }

    // ===== 直線移動フェーズ（無限） =====
    m_currentPosition += m_linearVelocity * deltaTime;
    m_currentRotation += m_angularVelocity * deltaTime;

    // 回転は固定でOK（必要なら微調整）
    m_currentRotation.x = -1.4f;
}

void TitleSpiralEffect::CalculateSpiralPosition(float t, Vector3& outPos, Vector3& outRot, Vector3& outVel,Vector3& outAngularVel)
{
    // ★★★ ゴール演出と同じ螺旋計算 ★★★

    float forward, height;
    float spiralRadius = m_spiralRadius;

    if (t < 0.4f) {
        // フェーズ1: ゆっくり上昇
        float t1 = t / 0.4f;

        forward = m_spiralDistance * 0.2f * t1 * t1;
        height = m_spiralHeight * 0.15f * t1 * t1;

        // 螺旋回転（ゆっくり）
        float spiralAngle = t1 * 3.14159f * 2.0f * m_spiralRotations * 0.3f;
        spiralRadius *= t1;

        // 螺旋オフセット
        Vector3 rightDir = Vector3(-m_direction.z, 0, m_direction.x);
        rightDir.Normalize();
        Vector3 spiralOffset = rightDir * (cosf(spiralAngle) * spiralRadius);
        spiralOffset += m_direction * (sinf(spiralAngle) * spiralRadius * 0.5f);

        // 位置
        Vector3 forwardOffset = m_direction * forward;
        Vector3 upOffset = Vector3(0, height, 0);
        outPos = m_startPosition + forwardOffset + upOffset + spiralOffset;

        // 回転
        outRot.y = m_startRotation.y + spiralAngle;
        outRot.x = -0.3f * t1;  // 徐々に上を向く
        outRot.z = 0.0f;

        // 速度
        float speed = (m_spiralDistance / m_spiralDuration) * 0.5f;
        outVel = m_direction * speed;
        outVel.y = speed * 0.01f;

    }
    else {
        // フェーズ2: 急加速
        float t2 = (t - 0.4f) / 0.6f;
        float accel = t2 * t2 * t2;

        forward = m_spiralDistance * 0.2f + m_spiralDistance * 0.8f * accel;
        height = m_spiralHeight * 0.15f + m_spiralHeight * 0.85f * accel;

        // 螺旋回転（速く）
        float spiralAngle = (0.3f + t2 * 0.7f) * 3.14159f * 2.0f * m_spiralRotations;

        // 螺旋オフセット
        Vector3 rightDir = Vector3(-m_direction.z, 0, m_direction.x);
        rightDir.Normalize();
        Vector3 spiralOffset = rightDir * (cosf(spiralAngle) * spiralRadius);
        spiralOffset += m_direction * (sinf(spiralAngle) * spiralRadius * 0.5f);

        // 位置
        Vector3 forwardOffset = m_direction * forward;
        Vector3 upOffset = Vector3(0, height, 0);
        outPos = m_startPosition + forwardOffset + upOffset + spiralOffset;

        // 回転
        outRot.y = m_startRotation.y + spiralAngle;
        outRot.x = Lerp(-0.3f, -1.4f, t2);  // 上向き
        outRot.z = sinf(spiralAngle) * 0.3f * t2;

        // 速度
        float speedMult = 1.0f + t2 * 3.0f;
        float speed = (m_spiralDistance / m_spiralDuration) * speedMult;
        outVel = m_direction * speed;
        outVel.y = speed *0.1f * speedMult;

        float angularSpeed =(3.14159f * 2.0f * m_spiralRotations) / m_spiralDuration;

        outAngularVel.y = angularSpeed;
        outAngularVel.z = cosf(spiralAngle) * 0.3f;
        printf("%f\n", t);
    }
}
void TitleSpiralEffect::CalculateStraightPosition(float t, Vector3& outPos, Vector3& outRot, Vector3& outVel)
{
    float forward, height;
    float spiralRadius = m_spiralRadius;

    // フェーズ2: 急加速
    float t2 = (t - 0.5f) / 0.95f;
    float accel = t2 * t2 * t2;

    forward = m_spiralDistance * 0.2f + m_spiralDistance * 0.8f * accel;
    height = m_spiralHeight * 0.15f + m_spiralHeight * 0.85f * accel;

    // 螺旋回転（速く）
    float spiralAngle = (0.3f + t2 * 0.7f) * 3.14159f * 2.0f * m_spiralRotations;

    // 螺旋オフセット
    Vector3 rightDir = Vector3(-m_direction.z, 0, m_direction.x);
    rightDir.Normalize();
    Vector3 spiralOffset = rightDir * (cosf(spiralAngle) * spiralRadius);
    spiralOffset += m_direction * (sinf(spiralAngle) * spiralRadius * 0.5f);

    // 位置
    Vector3 forwardOffset = m_direction * forward;
    Vector3 upOffset = Vector3(0, height, 0);
    outPos = m_startPosition + forwardOffset + upOffset + spiralOffset;

    // 回転
    outRot.y = m_startRotation.y + spiralAngle;
    outRot.x = Lerp(-0.3f, -1.4f, t2);  // 上向き
    outRot.z = sinf(spiralAngle) * 0.3f * t2;

    // 速度
    float speedMult = 1.0f + t2 * 3.0f;
    float speed = (m_spiralDistance / m_spiralDuration) * speedMult;
    outVel = m_direction * speed;
    outVel.y = speed * 1.5f * speedMult;
    printf("%f\n", t);
}


float TitleSpiralEffect::Lerp(float a, float b, float t) const
{
    return a + (b - a) * std::max(0.0f, std::min(1.0f, t));
}