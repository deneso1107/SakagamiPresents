#include "SimpleCamera.h"
SimpleFollowCamera::SimpleFollowCamera()
    : m_targetPlayer(nullptr)
{
}

SimpleFollowCamera& SimpleFollowCamera::Instance()
{
    static SimpleFollowCamera instance;
    return instance;
}

void SimpleFollowCamera::Init()
{
    Camera::Init();

    if (m_targetPlayer) {
        Vector3 playerPos = m_targetPlayer->GetPosition();
        Vector3 playerRot = m_targetPlayer->GetRotation();

        // プレイヤーの後ろ方向
        Vector3 backward = Vector3(-sinf(playerRot.y), 0.0f, -cosf(playerRot.y));

        // カメラ位置を設定
        m_position = playerPos + backward * m_distance;
        m_position.y += m_height;

        // 注視点はプレイヤーの少し上
        m_lookat = playerPos + Vector3(0.0f, 2.0f, 0.0f);
    }
}

void SimpleFollowCamera::Update(float deltaTime)
{
    if (!m_targetPlayer) return;

    Vector3 playerPos = m_targetPlayer->GetPosition();
    Vector3 playerRot = m_targetPlayer->GetRotation();

    // プレイヤーの後ろ方向を計算
    Vector3 backward = Vector3(-sinf(playerRot.y), 0.0f, -cosf(playerRot.y));

    // カメラ位置を即座に更新(補間なし)
    m_position = playerPos + backward * m_distance;
    m_position.y += m_height;

    // 注視点も即座に更新(補間なし)
    m_lookat = playerPos + Vector3(0.0f, 2.0f, 0.0f);
}

void SimpleFollowCamera::Draw()
{
    // ビュー変換行列作成
    Vector3 up = Vector3(0.0f, 1.0f, 0.0f);
    m_viewmtx = DirectX::XMMatrixLookAtLH(m_position, m_lookat, up);

    Renderer::SetViewMatrix(&m_viewmtx);

    // プロジェクション行列(固定FOV: 45度)
    float fieldOfView = DirectX::XMConvertToRadians(45.0f);
    float aspectRatio = static_cast<float>(Application::GetWidth()) /
        static_cast<float>(Application::GetHeight());

    m_projmtx = DirectX::XMMatrixPerspectiveFovLH(
        fieldOfView, aspectRatio, 0.1f, 3000.0f);

    Renderer::SetProjectionMatrix(&m_projmtx);
}

void SimpleFollowCamera::SetTargetPlayer(Player* player)
{
    m_targetPlayer = player;
}

Vector3 SimpleFollowCamera::GetForward() const
{
    Vector3 forward = m_lookat - m_position;
    forward.Normalize();
    return forward;
}