#pragma once
#include "ICameraState.h"
class CollisionCameraState : public ICameraState
{
private:
    float m_shakeIntensity = 0.0f;
    float m_shakeDuration = 0.0f;
    float m_shakeTimer = 0.0f;
    Vector3 m_basePosition;
    Vector3 m_baseLookAt;
    bool m_isShaking = false;

public:
    void OnEnter() override
    {
        m_currentFOV = 45.0f;
        m_currentBankAngle = 0.0f;
        m_shakeIntensity = 2.0f;
        m_shakeDuration = 0.5f;
        m_shakeTimer = 0.0f;
        m_isShaking = true;

        if (m_targetPlayer) {
            Vector3 playerPos = m_targetPlayer->GetPosition();
            m_basePosition = playerPos + Vector3(0, 8, -80);
            m_baseLookAt = playerPos + Vector3(0, 2, 0);
        }
    }

    void OnExit() override
    {
        m_isShaking = false;
    }

    void Update(float deltaTime) override
    {
        if (!m_targetPlayer || !m_isShaking) return;

        m_shakeTimer += deltaTime;

        if (m_shakeTimer >= m_shakeDuration) {
            m_isShaking = false;
            m_currentPosition = m_basePosition;
            m_currentLookAt = m_baseLookAt;
            return;
        }

        // å∏êäÇ∑ÇÈêUìÆ
        float t = m_shakeTimer / m_shakeDuration;
        float currentIntensity = m_shakeIntensity * (1.0f - t);

        Vector3 shakeOffset(
            ((rand() % 200 - 100) / 100.0f) * currentIntensity,
            ((rand() % 200 - 100) / 100.0f) * currentIntensity,
            ((rand() % 200 - 100) / 100.0f) * currentIntensity
        );

        m_currentPosition = m_basePosition + shakeOffset;
        m_currentLookAt = m_baseLookAt + shakeOffset * 0.5f;
    }

    bool IsShakeFinished() const { return !m_isShaking; }
};
