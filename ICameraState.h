#pragma once
#include "Camera.h"
#include "Player.h"

// ====================================
// カメラ状態の基底クラス
// ====================================
class ICameraState
{
protected:
    Player* m_targetPlayer;

    // 共通パラメータ
    Vector3 m_currentPosition;
    Vector3 m_currentLookAt;
    float m_currentFOV = 45.0f;
    float m_currentBankAngle = 0.0f;

    // ヘルパー関数
    Vector3 Lerp3(const Vector3& start, const Vector3& end, float t) const
    {
        t = std::max(0.0f, std::min(1.0f, t));
        return Vector3(
            start.x + (end.x - start.x) * t,
            start.y + (end.y - start.y) * t,
            start.z + (end.z - start.z) * t
        );
    }

    float Lerp(float start, float end, float t) const
    {
        t = std::max(0.0f, std::min(1.0f, t));
        return start + (end - start) * t;
    }

public:
    virtual ~ICameraState() = default;

    virtual void OnEnter() = 0;  // 状態開始時
    virtual void OnExit() = 0;   // 状態終了時
    virtual void Update(float deltaTime) = 0;

    void SetPlayer(Player* player) { m_targetPlayer = player; }

    Vector3 GetPosition() const { return m_currentPosition; }
    Vector3 GetLookAt() const { return m_currentLookAt; }
    float GetFOV() const { return m_currentFOV; }
    float GetBankAngle() const { return m_currentBankAngle; }
};