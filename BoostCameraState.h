#pragma once
#include "ICameraState.h"
class BoostCameraState : public ICameraState
{
private:
    float m_normalDistance = 45.0f;
    float m_boostDistance = 55.0f;
    float m_height = 8.0f;
    float m_normalFOV = 45.0f;
    float m_boostFOV = 55.0f;
    float m_followSpeed = 0.08f;
    float m_transitionSpeed = 0.05f;

    float m_currentDynamicDistance = 0.0f;
    float m_currentDynamicFOV = 0.0f;

    float CalculateSpeedRatio() const
    {
        Vector3 playerVel = m_targetPlayer->GetVelocity();
        float speed = sqrt(playerVel.x * playerVel.x + playerVel.z * playerVel.z);

        float threshold = m_targetPlayer->GetNormalSpeed();
        float maxSpeed = m_targetPlayer->GetMaxSpeed();

        if (speed <= threshold) return 0.0f;
        if (speed >= maxSpeed) return 1.0f;

        return (speed - threshold) / (maxSpeed - threshold);
    }

public:
    void OnEnter() override
    {
        if (m_targetPlayer) {
            Vector3 playerPos = m_targetPlayer->GetPosition();
            m_currentPosition = playerPos + Vector3(0, m_height, -m_normalDistance);
            m_currentLookAt = playerPos + Vector3(0.0f, 2.0f, 0.0f);
        }
    }

    void OnExit() override
    {
        m_currentDynamicDistance = 0.0f;
        m_currentDynamicFOV = 0.0f;
    }

    void Update(float deltaTime) override
    {
        if (!m_targetPlayer) return;

        // 速度比率の計算
        float speedRatio = CalculateSpeedRatio();

        // 動的パラメータの更新
        float targetDistance = (m_boostDistance - m_normalDistance) * speedRatio;
        float targetFOV = (m_boostFOV - m_normalFOV) * speedRatio;

        m_currentDynamicDistance = Lerp(m_currentDynamicDistance, targetDistance, m_transitionSpeed);
        m_currentDynamicFOV = Lerp(m_currentDynamicFOV, targetFOV, m_transitionSpeed);

        // FOVを更新
        m_currentFOV = m_normalFOV + m_currentDynamicFOV;

        // カメラバンクの計算
        Vector3 steer = m_targetPlayer->GetRotation();
        float bankDeg = (steer.y / 3.14f) * 1.25f;
        m_currentBankAngle = Lerp(m_currentBankAngle, bankDeg, 0.1f);

        // 位置計算
        Vector3 playerPos = m_targetPlayer->GetPosition();
        Vector3 playerRot = m_targetPlayer->GetRotation();
        Vector3 backward = Vector3(-sinf(playerRot.y), 0.0f, -cosf(playerRot.y));

        float totalDistance = m_normalDistance + m_currentDynamicDistance;
        Vector3 targetPos = playerPos + backward * totalDistance + Vector3(0, m_height, 0);

        // エンジン振動
        if (speedRatio > 0.1f) {
            float shake = speedRatio * 0.3f;
            targetPos.x += (rand() % 200 - 100) / 100.0f * shake;
            targetPos.y += (rand() % 200 - 100) / 150.0f * shake;
        }

        m_currentPosition = Lerp3(m_currentPosition, targetPos, m_followSpeed);
        m_currentLookAt = Lerp3(m_currentLookAt, playerPos + Vector3(0, 2, 0), m_followSpeed);
    }
};