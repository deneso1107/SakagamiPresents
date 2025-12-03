#pragma once
#include "Camera.h"
#include "Player.h" // プレイヤークラスのinclude

class CheeseCamera : public Camera
{
private:
    Player* m_targetPlayer;

    // 基本カメラ設定
    float m_distance = 15.0f;
    float m_height = 8.0f;
    float m_followSpeed = 0.08f;
    float m_lookAtSpeed = 0.1f;

    // 加速時のカメラ演出パラメータ
    float m_normalDistance = 45.0f;        // 通常時の距離
    float m_boostDistance = 55.0f;         // 加速時の距離
    float m_normalFOV = 45.0f;             // 通常時のFOV(度数)
    float m_boostFOV = 55.0f;              // 加速時のFOV(度数)
    float m_cameraTransitionSpeed = 0.05f; // カメラ演出の遷移速度
	float m_currentBank = 0.0f;      // 現在のカメラバンク角度(補完用)

    // 現在の動的な値
    float m_currentDynamicDistance = 0.0f; // 速度に応じた追加距離
    float m_currentDynamicFOV = 0.0f;      // 速度に応じた追加FOV

    // 速度閾値
    float m_boostSpeedThreshold = 2.0f;   // この速度以上で加速演出開始
    float m_maxBoostSpeed = 3.0f;         // この速度で最大演出

    // カメラの現在状態
    Vector3 m_currentOffset;
    Vector3 m_targetOffset;
    Vector3 m_currentLookAt;
    Vector3 m_targetLookAt;

    bool m_enableDebug = false;


    float m_shakeIntensity = 0.0f;
    float m_shakeDuration = 0.0f;
    Vector3 m_originalPosition;
    bool m_isShaking = false;

    Vector3 CalculateTargetPosition() const;
    Vector3 CalculateTargetLookAt() const;
    Vector3 Lerp3(const Vector3& start, const Vector3& end, float t) const;
    float Lerp(float start, float end, float t) const;

    // 速度に基づく演出値の計算
    float CalculateSpeedRatio() const;
    void UpdateDynamicCameraParameters();

public:
    static CheeseCamera& Instance()
    {
        static CheeseCamera instance;
        return instance;
    }
    CheeseCamera();
    virtual ~CheeseCamera() = default;

    void Init();
    void Update(float);
    void Draw() override;

    void SetTargetPlayer(Player* player) { m_targetPlayer = player; }

    // カメラパラメータの設定
    void SetDistance(float distance) {
        m_distance = distance;
        m_normalDistance = distance;
    }
    void SetHeight(float height) { m_height = height; }
    void SetFollowSpeed(float speed) { m_followSpeed = speed; }
    void SetLookAtSpeed(float speed) { m_lookAtSpeed = speed; }

    // 加速演出パラメータの設定
    void SetBoostDistance(float distance) { m_boostDistance = distance; }
    void SetBoostFOV(float fov) { m_boostFOV = fov; }
    void SetBoostSpeedThreshold(float speed) { m_boostSpeedThreshold = speed; }
    void SetCameraTransitionSpeed(float speed) { m_cameraTransitionSpeed = speed; }

    void EnableDebug(bool enable) { m_enableDebug = enable; }
    void DrawDebugUI();
    void Shake(float intensity, float duration);

    Vector3 GetForward() const
    {
        // カメラのターゲット - カメラ位置
        Vector3 forward = m_lookat - m_position;
        forward.Normalize();
        return forward;
    }
};