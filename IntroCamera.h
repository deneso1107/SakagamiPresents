#pragma once
#include "Camera.h"
#include"Player.h"
#include"SpringCamera.h"
class IntroCamera : public Camera {
private:
    enum class Phase {
        HighAngle,          // 螺旋開始：高い位置から俯瞰
        Descent,            // 降下中：徐々にカメラを下げる
        GroundApproach,     // 着地直前：地面から見上げる
        Landing,            // 着地の瞬間：カメラシェイク
        Transition          // 通常カメラへ遷移
    };

    Phase m_currentPhase = Phase::HighAngle;
    Player* m_targetPlayer = nullptr;
    bool m_isIntroFinished = false;  // ★演出終了フラグを追加

    float m_transitionProgress;
    // カメラ設定
    float m_fov = 45.0f;
    float m_cameraShakeIntensity = 0.0f;
    Vector3 m_shakeOffset = Vector3(0.0f, 0.0f, 0.0f);

    // フェーズ遷移の閾値（Playerのm_spiralTimeやprogressで判断）
    float m_phaseTransitionProgress[4] = { 0.0f, 0.25f, 0.38f, 0.41f };

public:
    static IntroCamera& Instance();
    void Init() override;
    void SetTargetPlayer(Player* player) { m_targetPlayer = player; }
    void Update(float deltaTime) override;
    bool IsIntroFinished() const { return m_isIntroFinished; }  // ★終了判定用
    void ResetIntro() { m_isIntroFinished = false; m_currentPhase = Phase::HighAngle; }

private:
    void UpdatePhase();
    void UpdateHighAngleCamera(float deltaTime);
    void UpdateDescentCamera(float deltaTime);
    void UpdateGroundApproachCamera(float deltaTime);
    void UpdateLandingCamera(float deltaTime);
    void UpdateTransitionCamera(float deltaTime);
    Vector3 Lerp3(const Vector3& a, const Vector3& b,float t) const;
    float Lerp(const float& a, const float& b,float t) const;
    float GetPlayerProgress(); // Playerの進行度を取得
    Vector3 CalculateCameraShake(float intensity);
    float Vector3Length(const Vector3&);
    Camera* m_currentCamera;
};

