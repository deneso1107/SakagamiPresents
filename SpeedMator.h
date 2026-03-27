#pragma once
#include"ScreenFixedBillboard.h"
#include <algorithm>
class SpeedMator
{
private:
    bool m_setSpeed;
    float m_speedParam;
    const float m_maxSpeed = 5.9f;  // 最大速度(Playerのスピードで調整)
    const float m_minAngle = 0.0f; // 開始角度（左下）
    const float m_maxAngle = -180.0f;  // 終了角度（右下）
    float m_currentDisplaySpeed = 0.0f;
    const float m_smoothingSpeed = 5.0f; // 補間速度

    // オーバーフロー演出関連
    bool m_isOverflowing;                           // オーバーフロー中かどうか
    bool m_isVibrating;                             // 振動中かどうか
    float m_overflowRotation;                       // 回転角度の累積値

    // オーバーフロー閾値設定
    const float m_needleVibrationSpeed = 4.0f;    // 針振動開始速度
    const float m_overflowThreshold = 5.5f;       // オーバーフロー開始速度
    const float m_maxOverflowSpeed = 1440.0f;        // 回転しまくる速度
    const float m_maxRotationSpeed = 14400.0f;       // 最大回転速度(度/秒) - 1秒で4回転
    float m_overflowRotationSpeed = 360.0f;         // 現在の回転速度
    const float m_baseRotationSpeed = 360.0f;       // 基本回転速度(度/秒)

    // 振動演出のパラメータ
    float m_vibrationOffset;                        // 振動によるオフセット
    float m_vibrationTime = 0.0f;                   // 振動用の時間累積
    const float m_vibrationAmplitude = 2.5f;       // 振動の振幅(度)
    const float m_vibrationFrequency = 80.0f;       // 振動の周波数

public:
    std::vector<std::unique_ptr<ScreenFixedBillboard>> m_matorPic;

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