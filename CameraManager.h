#pragma once
#include"ICameraState.h"
#include "NormalCameraState.h"
#include "BoostCameraState.h"
#include "CurveCameraState.h"
#include "CollisionCameraState.h"
enum class CameraStateType
{
    Normal,
    Boost,
    Curve,
    Collision
};

class CameraManager : public Camera
{
private:
    Player* m_targetPlayer;

    ICameraState* m_currentState;
    CameraStateType m_currentStateType;

    // 各状態のインスタンス
    NormalCameraState m_normalState;
    BoostCameraState m_boostState;
    CurveCameraState m_curveState;
    CollisionCameraState m_collisionState;

    // 状態遷移の条件チェック
    void CheckStateTransition()
    {
        if (!m_targetPlayer) return;

        Vector3 vel = m_targetPlayer->GetVelocity();
        float speed = sqrt(vel.x * vel.x + vel.z * vel.z);

        // 衝突状態は最優先(演出が終わるまで維持)
        if (m_currentStateType == CameraStateType::Collision) {
            if (m_collisionState.IsShakeFinished()) {
                ChangeState(CameraStateType::Normal);
            }
            return;
        }
        else {
            if (m_currentStateType != CameraStateType::Normal) {
                ChangeState(CameraStateType::Normal);
            }
        }
    }

public:
    CameraManager()
        : m_targetPlayer(nullptr)
        , m_currentState(nullptr)
        , m_currentStateType(CameraStateType::Normal)
    {
    }

    void Init() override
    {
        Camera::Init();

        if (!m_targetPlayer) return;

        // 各状態を初期化
        m_normalState.SetPlayer(m_targetPlayer);
        m_boostState.SetPlayer(m_targetPlayer);
        m_curveState.SetPlayer(m_targetPlayer);
        m_collisionState.SetPlayer(m_targetPlayer);

        // 初期状態を設定
        ChangeState(CameraStateType::Normal);
    }

    void Update(float deltaTime)
    {
        // 状態遷移チェック
        CheckStateTransition();

        // 現在の状態を更新
        if (m_currentState) {
            m_currentState->Update(deltaTime);

            // 状態からカメラパラメータを取得
            m_position = m_currentState->GetPosition();
            m_lookat = m_currentState->GetLookAt();
        }
    }

    void Draw() override
    {
        if (!m_currentState) return;

        // ビュー変換行列作成
        Vector3 up = Vector3(0.0f, 1.0f, 0.0f);
        m_viewmtx = DirectX::XMMatrixLookAtLH(m_position, m_lookat, up);

        // バンク角(ロール)を適用
        float bankRad = DirectX::XMConvertToRadians(m_currentState->GetBankAngle());
        Matrix4x4 bankRot = Matrix4x4::CreateRotationY(bankRad);
        m_viewmtx = bankRot * m_viewmtx;

        Renderer::SetViewMatrix(&m_viewmtx);

        // プロジェクション行列作成
        float currentFOV = m_currentState->GetFOV();
        float fieldOfView = DirectX::XMConvertToRadians(currentFOV);
        float aspectRatio = static_cast<float>(Application::GetWidth()) /
            static_cast<float>(Application::GetHeight());
        float nearPlane = 0.1f;
        float farPlane = 3000.0f;

        m_projmtx = DirectX::XMMatrixPerspectiveFovLH(
            fieldOfView,
            aspectRatio,
            nearPlane,
            farPlane);

        Renderer::SetProjectionMatrix(&m_projmtx);
    }

    void ChangeState(CameraStateType newStateType)
    {
        if (m_currentStateType == newStateType) return;

        // 現在の状態を終了
        if (m_currentState) {
            m_currentState->OnExit();
        }

        // 新しい状態に切り替え
        m_currentStateType = newStateType;

        switch (newStateType) {
        case CameraStateType::Normal:
            m_currentState = &m_normalState;
            break;
        case CameraStateType::Boost:
            m_currentState = &m_boostState;
            break;
        case CameraStateType::Curve:
            m_currentState = &m_curveState;
            break;
        case CameraStateType::Collision:
            m_currentState = &m_collisionState;
            break;
        }

        // 新しい状態を開始
        if (m_currentState) {
            m_currentState->OnEnter();
        }
    }

    // プレイヤーの設定
    void SetTargetPlayer(Player* player)
    {
        m_targetPlayer = player;
    }

    // 外部から強制的に状態変更(衝突時など)
    void ForceCollisionState()
    {
        ChangeState(CameraStateType::Collision);
    }

    CameraStateType GetCurrentStateType() const { return m_currentStateType; }
};