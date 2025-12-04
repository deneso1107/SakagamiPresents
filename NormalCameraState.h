#pragma once
#include "ICameraState.h"
class NormalCameraState : public ICameraState
{
private:
    float m_distance = 75.0f;
    float m_height = 6.0f;
    float m_followSpeed = 0.1f;
    float m_lookAtSpeed = 0.15f;

public:
    void OnEnter() override
    {
        m_currentFOV = 45.0f;
        m_currentBankAngle = 0.0f;

        if (m_targetPlayer) {
            Vector3 playerPos = m_targetPlayer->GetPosition();
            Vector3 playerRot = m_targetPlayer->GetRotation();

            Vector3 backward = Vector3(-sinf(playerRot.y), 0.0f, -cosf(playerRot.y));
            m_currentPosition = playerPos + backward * m_distance + Vector3(0, m_height, 0);
            m_currentLookAt = playerPos + Vector3(0.0f, 2.0f, 0.0f);
        }
    }

    void OnExit() override
    {
        // 必要に応じて終了処理
    }

    void Update(float deltaTime) override
    {
        if (!m_targetPlayer) return;

        Vector3 playerPos = m_targetPlayer->GetPosition();
        Vector3 playerRot = m_targetPlayer->GetRotation();

        // 目標位置計算
        Vector3 backward = Vector3(-sinf(playerRot.y), 0.0f, -cosf(playerRot.y));
        Vector3 targetPos = playerPos + backward * m_distance + Vector3(0, m_height, 0);
        Vector3 targetLookAt = playerPos + Vector3(0.0f, 2.0f, 0.0f);

        // スムーズに追従
        m_currentPosition = Lerp3(m_currentPosition, targetPos, m_followSpeed);
        m_currentLookAt = Lerp3(m_currentLookAt, targetLookAt, m_lookAtSpeed);
    }
};