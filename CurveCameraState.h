#pragma once
#include "ICameraState.h"
class CurveCameraState : public ICameraState
{
private:
    float m_distance = 70.0f;
    float m_height = 7.0f;
    float m_sideOffset = 0.0f;
    float m_maxSideOffset = 15.0f;
    float m_followSpeed = 0.12f;

public:
    void OnEnter() override
    {
        m_currentFOV = 45.0f;
        m_currentBankAngle = 0.0f;

        if (m_targetPlayer) {
            Vector3 playerPos = m_targetPlayer->GetPosition();
            m_currentPosition = playerPos + Vector3(0, m_height, -m_distance);
            m_currentLookAt = playerPos + Vector3(0.0f, 2.0f, 0.0f);
        }
    }

    void OnExit() override
    {
        m_sideOffset = 0.0f;
    }

    void Update(float deltaTime) override
    {
        if (!m_targetPlayer) return;

        Vector3 playerPos = m_targetPlayer->GetPosition();
        Vector3 playerRot = m_targetPlayer->GetRotation();
        Vector3 playerVel = m_targetPlayer->GetVelocity();

        // 回転入力を取得(仮定: -1.0 ~ 1.0の範囲)
        float turnInput = playerRot.y; // 実際はハンドル入力値を使う

        // サイドオフセットを計算
        float targetSideOffset = turnInput * m_maxSideOffset;
        m_sideOffset = Lerp(m_sideOffset, targetSideOffset, 0.15f);

        // カメラ位置計算(カーブ外側に配置)
        Vector3 backward = Vector3(-sinf(playerRot.y), 0.0f, -cosf(playerRot.y));
        Vector3 right = Vector3(cosf(playerRot.y), 0.0f, -sinf(playerRot.y));

        Vector3 targetPos = playerPos + backward * m_distance +
            right * m_sideOffset + Vector3(0, m_height, 0);

        // 注視点を少し内側に
        Vector3 targetLookAt = playerPos + Vector3(0, 2, 0) - right * (m_sideOffset * 0.3f);

        m_currentPosition = Lerp3(m_currentPosition, targetPos, m_followSpeed);
        m_currentLookAt = Lerp3(m_currentLookAt, targetLookAt, m_followSpeed);
    }
};