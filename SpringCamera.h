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

// カメラモード
enum class CameraMode {
    NORMAL,      // 通常レース
    FALLING,     // 落下中
    RESPAWNING   // リスポーン演出中
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
    float m_transitionSpeed = 0.08f;
    float m_MaxBoostingFOV = 60.0f;

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

    CameraMode m_currentMode = CameraMode::NORMAL;

    // 落下モード用パラメータ
    Vector3 m_fallingCameraPosition;  // 落下時のカメラ固定位置
    Vector3 m_fallingStartPlayerPos;  //落下開始時のプレイヤー位置
    float m_fallingModeTimer = 0.0f;
    float m_fallingModeDuration = 1.5f; // 落下演出の長さ
    float m_fallingCameraHeight = 20.0f; //プレイヤーの上何m上にカメラを置くか

	Vector3 m_fallPlayerPosition; // 落下中のプレイヤー位置取得用

    // 落下モード用の更新
    void UpdateFallingMode(float deltaTime);
    void UpdateNormalMode(float deltaTime);
    void UpdateRespawningMode(float deltaTime); // オプション

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

    // IntroCameraが遷移時にスプリングを引き継げるようにする
    const Spring& GetPositionSpring() const { return m_positionSpring; }
    const Spring& GetLookAtSpring() const { return m_lookAtSpring; }
    float GetCurrentFOV() const { return m_currentFOV; }

    void SetPositionSpring(const Spring& spring) { m_positionSpring = spring; }
    void SetLookAtSpring(const Spring& spring) { m_lookAtSpring = spring; }
	void SetCurrentFOV(float fov) { m_currentFOV = fov; }
	void PlusCurrentFOV() { m_boostParams.fov = m_MaxBoostingFOV; }

    void SetCameraMode(CameraMode mode);
    CameraMode GetCameraMode() const { return m_currentMode; }

    // 落下演出用
    void StartFallingMode();
    void EndFallingMode();

    //デバッグ用
    void debugCameraParam();
};