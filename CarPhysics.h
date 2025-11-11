#pragma once
#include <SimpleMath.h>
class CarPhysics {
public:
    using Vector3 = DirectX::SimpleMath::Vector3;

    struct Params {
        float acceleration = 0.2f;             // 加速力
        float maxSpeed = 5.5f;                  // 最大速度
        float turnSpeed = 0.5f;                 // ドリフト時の回転速度
        float driftTurnMultiplier = 1.8f;       // ドリフト時の旋回増加倍率
        float friction = 0.1f;                  // 減速係数（摩擦）
        float rotationSmoothness = 0.2f;        // 向き補間係数
    };

    enum class DriftState {
        None,
        Left,
        Right
    };

private:
    Params m_params;

    Vector3 m_position = {};
    Vector3 m_rotation = {}; // yのみ使用
    Vector3 m_velocity = {};

    float m_targetYaw = 0.0f;
    DriftState m_driftState = DriftState::None;
    bool m_isAccelerating = false;

public:
    // 初期化
    void SetParams(const Params& params);
    void SetPosition(const Vector3& pos);
    void SetRotation(const Vector3& rot);

    // 入力と更新
    void SetInput(bool w, bool a, bool d, bool s, bool left, bool right);
    void Init();
    void Update(float deltaTime);

    // 状態取得
    Vector3 GetPosition() const;
    Vector3 GetRotation() const;
    Vector3 GetVelocity() const;
};
