#include "StageSelectCamera.h"
void StageSelectCamera::Init() 
{
    Camera::Init();

    // ステージ0の位置（X=0）を見るように初期化
    m_position = Vector3(0.0f, m_height, -m_distance);
    m_targetPosition = m_position;
    m_lookAt = Vector3(0.0f, 0.0f, 0.0f);
    m_targetLookAt = m_lookAt;
}

void StageSelectCamera::Update(float deltaTime)
{
    // Lerpでなめらかにスライド（SpringCameraのLerpと同じ方式）
    auto Lerp = [](Vector3 a, Vector3 b, float t) {
        return a + (b - a) * std::max(0.0f, std::min(1.0f, t));
        };

    float t = m_lerpSpeed * deltaTime;
    m_position = Lerp(m_position, m_targetPosition, t);
    m_lookAt = Lerp(m_lookAt, m_targetLookAt, t);
}

void StageSelectCamera::Draw() 
{
    // SpringCamera::Draw()と全く同じ書き方
    Vector3 up = Vector3(0.0f, 1.0f, 0.0f);
    m_viewmtx = DirectX::XMMatrixLookAtLH(m_position, m_lookAt, up);
    Renderer::SetViewMatrix(&m_viewmtx);

    float fieldOfView = DirectX::XMConvertToRadians(m_fov);
    float aspectRatio = static_cast<float>(Application::GetWidth()) /
        static_cast<float>(Application::GetHeight());

    m_projmtx = DirectX::XMMatrixPerspectiveFovLH(
        fieldOfView, aspectRatio, 0.1f, 3000.0f);

    Renderer::SetProjectionMatrix(&m_projmtx);
}

void StageSelectCamera::SlideToStage(int index, float spacing)
{
    float targetX = index * spacing; // モデルの配置と完全一致
    m_targetPosition = Vector3(targetX, m_height, -m_distance);
    m_targetLookAt = Vector3(targetX, 0.0f, 0.0f);
}

void StageSelectCamera::SetFixedPosition(const Vector3& pos, const Vector3& lookAt) {
    m_position = pos;
    m_targetPosition = pos;
    m_lookAt = lookAt;
    m_targetLookAt = lookAt;
}