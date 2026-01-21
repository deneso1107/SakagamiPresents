#include "TitleCamera.h"
TitleCamera& TitleCamera::Instance()
{
    static TitleCamera instance;
    return instance;
}

void TitleCamera::Init()
{
    // 基底クラスの初期化
    Camera::Init();

    // デフォルトのカメラオフセット
    // プレイヤーの前方から見る位置
    m_cameraOffset = Vector3(5.0f, 10.0f, -25.0f);   // 右5m、上10m、前25m
    m_lookAtOffset = Vector3(0.0f, 5.0f, 10.0f);     // 少し上、少し奥を見る

    m_fov = 60.0f;  // 広めの視野角
    m_isFixedMode = true;

    printf("TitleCamera Initialized\n");
}

void TitleCamera::Update(float deltaTime)
{
    if (!m_targetPlayer) return;

    Vector3 playerPos = m_targetPlayer->GetPosition();

    if (m_isFixedMode)
    {
        // ★★★ 固定モード：プレイヤーの初期位置を基準に固定 ★★★

        // 最初の1回だけ固定位置を計算
        static bool initialized = false;
        if (!initialized)
        {
            // プレイヤーの初期位置から相対的にカメラを配置
            m_fixedPosition = playerPos + m_cameraOffset;
            m_fixedLookAt = playerPos + m_lookAtOffset;
            initialized = true;

            printf("TitleCamera Fixed at (%.2f, %.2f, %.2f)\n",
                m_fixedPosition.x, m_fixedPosition.y, m_fixedPosition.z);
        }

        // 固定位置を維持
        m_position = m_fixedPosition;
        m_lookat = m_fixedLookAt;
    }
    else
    {
        // ★★★ 追従モード：プレイヤーを常に捉え続ける ★★★

        // プレイヤーの現在位置からオフセット
        m_position = playerPos + m_cameraOffset;
        m_lookat = playerPos + m_lookAtOffset;
    }
}