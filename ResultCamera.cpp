#include "ResultCamera.h"

ResultCamera& ResultCamera::Instance()
{
    static ResultCamera instance;
    return instance;
}

void ResultCamera::Init()
{
    // 基底クラスの初期化
    Camera::Init();

    // ワイドショット（固定カメラ用）
    m_wideShotParams.offset = Vector3(0.0f, 15.0f, -50.0f);
    m_wideShotParams.lookAtOffset = Vector3(0.0f, 10.0f, 20.0f);  // やや奥を見る
    m_wideShotParams.fov = 70.0f;

    // クローズアップ
    m_closeUpParams.offset = Vector3(0.0f, 10.0f, -25.0f);
    m_closeUpParams.lookAtOffset = Vector3(0.0f, 8.0f, 0.0f);
    m_closeUpParams.fov = 60.0f;

    // リザルト表示時
    m_resultParams.offset = Vector3(-8.0f, 5.0f, -30.0f);
    m_resultParams.lookAtOffset = Vector3(-3.0f, 4.0f, 0.0f);
    m_resultParams.fov = 60.0f;

    m_cameraState = CameraState::WideShot;
    m_currentParams = m_wideShotParams;
    m_useFixedCamera = false;


    printf("ResultCamera Initialized\n");
}

void ResultCamera::SetFixedCamera(bool fixed, const Vector3& pos, const Vector3& lookAt)
{
    m_useFixedCamera = fixed;
    if (fixed)
    {
        m_fixedCameraPos = pos;
        m_fixedLookAt = lookAt;
    }
}

void ResultCamera::Update(float deltaTime)
{
    if (!m_targetPlayer) return;

    if (m_useFixedCamera)
    {
        // ★固定カメラモード：プレイヤーを追従しない★
        m_position = m_fixedCameraPos;
        m_lookat = m_fixedLookAt;
    }
    else
    {
        // ★通常モード：プレイヤーを追従★
        Vector3 playerPos = m_targetPlayer->GetPosition();

        CameraParams targetParams;
        switch (m_cameraState)
        {
        case CameraState::WideShot:
            targetParams = m_wideShotParams;
            break;
        case CameraState::CloseUp:
            targetParams = m_closeUpParams;
            break;
        case CameraState::Result:
            targetParams = m_resultParams;
            break;
        }

        float easedProgress = EaseInOutCubic(m_transitionProgress);

        m_currentParams.offset = LerpVector3(m_currentParams.offset, targetParams.offset, easedProgress);
        m_currentParams.lookAtOffset = LerpVector3(m_currentParams.lookAtOffset, targetParams.lookAtOffset, easedProgress);
        m_currentParams.fov = m_currentParams.fov + (targetParams.fov - m_currentParams.fov) * easedProgress;

        m_position = playerPos + m_currentParams.offset;
        m_lookat = playerPos + m_currentParams.lookAtOffset;
    }
}

float ResultCamera::EaseInOutCubic(float t)
{
    if (t < 0.5f)
    {
        return 4.0f * t * t * t;
    }
    else
    {
        float f = (2.0f * t - 2.0f);
        return 0.5f * f * f * f + 1.0f;
    }
}

Vector3 ResultCamera::LerpVector3(const Vector3& start, const Vector3& end, float t)
{
    return Vector3(
        start.x + (end.x - start.x) * t,
        start.y + (end.y - start.y) * t,
        start.z + (end.z - start.z) * t
    );
}