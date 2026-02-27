#pragma once
#include <algorithm>
#include <cmath>
#include "ObjectBase.h"

class StageDecideEffect
{
private:

    Vector3 m_startPosition;
    Vector3 m_courseCenter;

    float m_timer = 0.0f;
    float m_duration = 2.0f; // 全体の演出時間
    bool  m_isActive = false;
    bool  m_isDone = false;

    Vector3 m_currentPosition;
    Vector3 m_currentRotation;
    Vector3 m_currentVelocity;

    float Lerp(float a, float b, float t) const {
        return a + (b - a) * std::max(0.0f, std::min(1.0f, t));
    }
    float EaseOutCubic(float t) { return 1.0f - powf(1.0f - t, 3.0f); }
    float EaseInCubic(float t) { return t * t * t; }

    // ベジェ曲線（4点）でXZ経路を作る
    Vector3 CubicBezierXZ(float t,
        const Vector3& p0, const Vector3& p1,
        const Vector3& p2, const Vector3& p3)
    {
        float u = 1.0f - t;
        float tt = t * t;
        float uu = u * u;
        float uuu = uu * u;
        float ttt = tt * t;

        Vector3 result;
        result.x = uuu * p0.x + 3 * uu * t * p1.x + 3 * u * tt * p2.x + ttt * p3.x;
        result.y = p0.y; // Y固定
        result.z = uuu * p0.z + 3 * uu * t * p1.z + 3 * u * tt * p2.z + ttt * p3.z;
        return result;
    }


public:
    // 演出開始（コース中心座標を渡す）
    void Start(const Vector3& courseCenter);
    void Update(float deltaTime);
    bool    IsActive()           const { return m_isActive; }
    bool    IsDone()             const { return m_isDone; }
    Vector3 GetCurrentPosition() const { return m_currentPosition; }
    Vector3 GetCurrentRotation() const { return m_currentRotation; }
    Vector3 GetCurrentVelocity() const { return m_currentVelocity; }
};