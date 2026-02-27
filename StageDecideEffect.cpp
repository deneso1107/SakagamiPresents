#include"StageDecideEffect.h"
#include "BaseRoad.h"
void StageDecideEffect::Start(const Vector3& courseCenter)
{
    // カメラ手前のZ座標を基準にする
    float frontZ = 150.0f; // カメラのZ=60に近い位置
    float startY = courseCenter.y - 20.0f; // 少し下に

    m_courseCenter = courseCenter;
    m_startPosition = Vector3(courseCenter.x + 80.0f, startY, frontZ); // +で右外

    m_currentPosition = m_startPosition;
    m_currentRotation = Vector3(0.0f, MathUtils::DegreesToRadians(270.0f), 0.0f);
    m_currentVelocity = Vector3(0.0f, 0.0f, 0.0f);

    m_timer = 0.0f;
    m_isActive = true;
    m_isDone = false;
}

void StageDecideEffect::Update(float deltaTime)
{
    if (!m_isActive) return;
    m_timer += deltaTime;

    float t = std::min(m_timer / m_duration, 1.0f);

    // ベジェ曲線の制御点
    // P0: 右外スタート
    // P1: コース左側（通り過ぎ）
    // P2: 手前に大きくカーブ（Z+方向）
    // P3: コース中心（ゴール）
    Vector3 p0 = m_startPosition;                                                      // ← ここを修正
    Vector3 p1 = Vector3(m_courseCenter.x - 30.0f, m_startPosition.y, m_startPosition.z);
    Vector3 p2 = Vector3(m_courseCenter.x - 10.0f, m_startPosition.y, m_startPosition.z + 10.0f);
    Vector3 p3 = m_courseCenter;

    Vector3 prevPos = m_currentPosition;

    // フェーズ1（t=0〜0.6）：ゆっくりカーブ
    // フェーズ2（t=0.6〜1.0）：急加速で突撃
    float curveT;
    if (t < 0.6f) {
        curveT = EaseOutCubic(t / 0.6f) * 0.7f; // 曲線の0〜70%をゆっくり
    }
    else {
        float t2 = (t - 0.6f) / 0.4f;
        curveT = 0.7f + EaseInCubic(t2) * 0.3f; // 残り30%を急加速
    }

    m_currentPosition = CubicBezierXZ(curveT, p0, p1, p2, p3);

    // 速度（移動方向から計算）
    m_currentVelocity = (m_currentPosition - prevPos) * (1.0f / deltaTime);

    // 進行方向に向きを合わせる
    Vector3 dir = m_currentPosition - prevPos;
    if (dir.Length() > 0.001f) {
        dir.Normalize();
        m_currentRotation.y = atan2f(dir.x, dir.z);
    }

    // 突撃フェーズで少し傾ける（かっこよく）
    if (t >= 0.6f) {
        float t2 = (t - 0.6f) / 0.4f;
        m_currentRotation.x = Lerp(0.0f, MathUtils::DegreesToRadians(-15.0f), t2);
    }

    if (t >= 1.0f) {
        m_isActive = false;
        m_isDone = true;
    }
}