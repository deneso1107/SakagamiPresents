#pragma once
#include "Camera.h"
class StageSelectCamera : public Camera {
private:
    Vector3 m_position;
    Vector3 m_targetPosition;  // スライド目標位置
    Vector3 m_lookAt;
    Vector3 m_targetLookAt;    // 注視点の目標

    float m_height = 40.0f;
    float m_distance = 150.0f;
    float m_fov = 45.0f;
    float m_lerpSpeed = 3.0f;  // スライドの速さ

    float m_stageSpacing = 50.0f; // ステージ間の距離（StageManagerと合わせる）

public:
    void Init() override;
    void Update(float deltaTime) override;
    void Draw() override;

    // ステージインデックスに移動
    void SlideToStage(int,float);

    void SetStageSpacing(float spacing) { m_stageSpacing = spacing; }

    void SetFixedPosition(const Vector3&, const Vector3&);
};
