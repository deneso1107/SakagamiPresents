#pragma once
#include"ScreenFixedBillboard.h"
#include <algorithm>
class SpeedMator
{
private:
    bool m_SetSpeed;
    float m_SpeedParam;
    const float m_MaxSpeed = 5.9f;  // 最大速度(Playerのスピードで調整)
    const float m_MinAngle = 0.0f; // 開始角度（左下）
    const float m_MaxAngle = -180.0f;  // 終了角度（右下）
    float m_CurrentDisplaySpeed = 0.0f;
    const float m_SmoothingSpeed = 5.0f; // 補間速度

    // オーバーフロー演出関連
    bool m_IsOverflowing;                           // オーバーフロー中かどうか
    bool m_IsVibrating;                             // 振動中かどうか
    float m_OverflowRotation;                       // 回転角度の累積値

    // オーバーフロー閾値設定
    const float m_NeedleVibrationSpeed = 4.0f;    // 針振動開始速度
    const float m_OverflowThreshold = 5.5f;       // オーバーフロー開始速度
    const float m_MaxOverflowSpeed = 1440.0f;        // 回転しまくる速度
    const float m_MaxRotationSpeed = 14400.0f;       // 最大回転速度(度/秒) - 1秒で4回転
    float m_OverflowRotationSpeed = 360.0f;         // 現在の回転速度
    const float m_BaseRotationSpeed = 360.0f;       // 基本回転速度(度/秒)

    // 振動演出のパラメータ
    float m_VibrationOffset;                        // 振動によるオフセット
    float m_VibrationTime = 0.0f;                   // 振動用の時間累積
    const float m_VibrationAmplitude = 2.5f;       // 振動の振幅(度)
    const float m_VibrationFrequency = 80.0f;       // 振動の周波数

public:
    std::vector<std::unique_ptr<ScreenFixedBillboard>> m_MatorPic;

    SpeedMator();
    void Init();
    void Update(float);
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