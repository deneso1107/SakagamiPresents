#pragma once
#include"ScreenFixedBillboard.h"
#include <algorithm>
class SpeedMator
{
private:
    bool m_SetSpeed;
    float m_SpeedParam;
    const float m_MaxSpeed = 2.75f;  // 最大速度(Playerのスピードで調整)
    const float m_MinAngle = 0.0f; // 開始角度（左下）
    const float m_MaxAngle = -180.0f;  // 終了角度（右下）
    float m_CurrentDisplaySpeed = 0.0f;
    const float m_SmoothingSpeed = 5.0f; // 補間速度

public:
    std::vector<std::unique_ptr<ScreenFixedBillboard>> m_MatorPic;

    SpeedMator();
    void Init();
    void Update(uint64_t);
    void Draw();
    void SetSpeed(float speed);

private:
    float CalculateNeedleAngle(float speed);
    void UpdateNeedleRotation();

    float Lerp(float a, float b, float t)
    {
        return a + t * (b - a);
    }

    float GetDeltaTime()
    {
        // 実装に応じてデルタタイムを返す
        return 0.0154f; // 例: 60FPS想定
    }
};