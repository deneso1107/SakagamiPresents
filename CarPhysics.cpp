#include "CarPhysics.h"

#include <cmath>
#include "system\imgui\imgui.h"

constexpr float PI = 3.14159265f;

using Vector3 = DirectX::SimpleMath::Vector3;

void CarPhysics::SetParams(const Params& params) {
    m_params = params;
}

void CarPhysics::SetPosition(const Vector3& pos) {
    m_position = pos;
}

void CarPhysics::SetRotation(const Vector3& rot) {
    m_rotation = rot;
    m_targetYaw = rot.y;
}

Vector3 CarPhysics::GetPosition() const {
    return m_position;
}

Vector3 CarPhysics::GetRotation() const {
    return m_rotation;
}

Vector3 CarPhysics::GetVelocity() const {
    return m_velocity;
}

 void CarPhysics::Init()
{
    ImGui::Begin("Debug Player Move");

    ImGui::SliderFloat("VALUE_MOVE_MODEL", &m_params.acceleration, 0.01f, 3.0f);
    ImGui::SliderFloat("VALUE_ROTATE_MODEL", &m_params.maxSpeed ,5.5f, 100.0f);


    ImGui::End();
}
void CarPhysics::SetInput(bool w, bool a, bool d, bool s, bool left, bool right) {
    // ドリフト状態判定
    if (w && a) m_driftState = DriftState::Left;
    else if (w && d) m_driftState = DriftState::Right;
    else m_driftState = DriftState::None;

    m_isAccelerating = w;

    // 移動方向に応じた目標回転角
    if (a) {//
        if (w) m_targetYaw = -PI * 0.25f;
        else if (s) m_targetYaw = -PI * 0.25f;
        else m_targetYaw = -PI * 0.5f;
    }
    else if (d) {
        if (w) m_targetYaw = PI * 0.25f;
        else if (s) m_targetYaw = -PI * 0.25f;
        else m_targetYaw = PI * 0.5f;
    }
    else if (w) {
        m_targetYaw = 0.0f;
    }
    else if (s) {
        m_targetYaw = PI;
    }

    // 左右キーで手動回転追加
    if (left) m_targetYaw -= 0.1f;
    if (right) m_targetYaw += 0.1f;
}

void CarPhysics::Update(float dt) 
{


    //向いている方向と{0,1,0}との外積をとって車のドリフト
    // 回転補間
    float diff = m_targetYaw - m_rotation.y;
    if (diff > PI) diff -= PI * 2.0f;
    if (diff < -PI) diff += PI * 2.0f;
    m_rotation.y += diff * m_params.rotationSmoothness;

    // 回転角を正規化
    if (m_rotation.y > PI) m_rotation.y -= PI * 2.0f;
    if (m_rotation.y < -PI) m_rotation.y += PI * 2.0f;

    // 前方向ベクトル（向いてる方向に対して進む）
    Vector3 forward = Vector3(std::sinf(m_rotation.y), 0.0f, std::cosf(m_rotation.y));

    // 加速処理
    if (m_isAccelerating) {
        m_velocity += forward * m_params.acceleration;
        if (m_velocity.Length() > m_params.maxSpeed) 
        {
            Vector3 normalized = m_velocity;
            normalized.Normalize();
            m_velocity = normalized * m_params.maxSpeed;
        }
    }

    // ドリフト中の旋回と減速
    if (m_driftState != DriftState::None) {
        float driftTurn = m_params.turnSpeed * m_params.driftTurnMultiplier * dt;
        if (m_driftState == DriftState::Left) m_rotation.y -= driftTurn;
        if (m_driftState == DriftState::Right) m_rotation.y += driftTurn;
        m_velocity *= (1.0f - m_params.friction);
    }

    // 通常の減速（摩擦）
    m_velocity -= m_velocity * m_params.friction;

    // 移動処理
    m_position += m_velocity * dt;
}