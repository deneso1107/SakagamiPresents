#pragma once
#include "Camera.h"
#include "Player.h"

// ===================================
// スプリング(バネ)の構造体
// ===================================
struct Spring
{
    Vector3 position;      // 現在位置
    Vector3 velocity;      // 速度(モーメンタム)
    Vector3 target;        // 目標位置

    float stiffness = 150.0f;   // バネの硬さ(収束)
    float damping = 20.0f;      // 減衰係数(振動を抑える)

    // スプリングの更新(フックの法則)
    void Update(float deltaTime);

    // 即座にターゲットに移動(初期化用)
    void Snap();
};

// ===================================
// スプリングカメラ本体
// ===================================
class SpringCamera : public Camera
{
private:
    Player* m_targetPlayer;

    // スプリング(カメラ位置と注視点)
    Spring m_positionSpring;
    Spring m_lookAtSpring;

    // カメラパラメータ
    struct CameraParams 
    {
        float distance = 75.0f;
        float height = 6.0f;
        float fov = 45.0f;
        float anticipation = 0.3f;  // 速度先読み係数
        float lookAheadDist = 0.5f; // 注視点先読み距離

        // スプリングパラメータ
        float positionStiffness = 150.0f;
        float positionDamping = 20.0f;
        float lookAtStiffness = 200.0f;
        float lookAtDamping = 25.0f;
        char name;
    };

    // 各状態のパラメータ
    CameraParams m_normalParams;
    CameraParams m_boostParams;
    CameraParams m_currentParams;

    // 演出パラメータ
    float m_currentBank = 0.0f;
    float m_currentFOV = 45.0f;
    float m_transitionSpeed = 0.05f;

    // 速度関連
    float m_boostSpeedThreshold = 2.0f;
    float m_maxBoostSpeed = 3.0f;

    // シェイク
    bool m_isShaking = false;
    float m_shakeIntensity = 0.0f;
    float m_shakeDuration = 0.0f;

    float m_basePitchAngle;      // 基本的なカメラピッチ角度（度）
    float m_currentPitchOffset;  // 現在のピッチオフセット（ラジアン）
    float m_targetPitchOffset;   // 目標ピッチオフセット
    float m_pitchTransitionSpeed; // ピッチ遷移速度

    // 坂道用パラメータ
    float m_maxUphillPitch;   // 上り坂の最大ピッチ角（度）
    float m_maxDownhillPitch; // 下り坂の最大ピッチ角（度）

    // ===== プライベート関数 =====
    float CalculateSpeedRatio() const;
    CameraParams DetermineTargetParams() const;
    Vector3 CalculateIdealCameraPosition() const;
    Vector3 CalculateIdealLookAtPosition() const;
    void ApplyShake(Vector3& position, float deltaTime);
    void ApplyBoostShake(Vector3& position) const;
    float Lerp(float a, float b, float t) const;
    float CalculateGroundSlopeFromVelocity() const;
    float CalculatePitchOffset() const;
	Vector3 CalculateIdealCameraPositionWithPitch() const;

public:
    SpringCamera();
    virtual ~SpringCamera() = default;

    static SpringCamera& Instance();

    void Init() override;
    void Update(float deltaTime);
    void Draw() override;

    // 外部インターフェース
    void SetTargetPlayer(Player* player);
    void Shake(float intensity, float duration);
    Vector3 GetForward() const;

    // デバッグ用: スプリングパラメータ調整
    void SetPositionSpring(float stiffness, float damping);
    void SetLookAtSpring(float stiffness, float damping);
    //Vector3 GetForward() const
    //{
    //    // カメラのターゲット - カメラ位置
    //    Vector3 forward = m_lookat - m_position;
    //    forward.Normalize();
    //    return forward;
    //}
};