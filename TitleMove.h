#pragma once
#include "system/commontypes.h"

// タイトル画面用：クルクル螺旋上昇エフェクト（シンプル版）
// ゴール演出の動きをそのまま使用、UIやシーン遷移は無し

enum class ESpiralState
{
    Spiral,   // 螺旋演出中
    Linear    // 直線移動中
};
class TitleSpiralEffect 
{
private:
    // 螺旋パラメータ
    Vector3 m_startPosition;       // 開始位置
    Vector3 m_startRotation;       // 開始回転
    Vector3 m_direction;           // 上昇方向

    // タイマー
    float m_spiralTime;
    float m_spiralDuration;

    // 螺旋設定
    float m_spiralHeight;          // 上昇高度
    float m_spiralDistance;        // 前方距離
    float m_spiralRadius;          // 螺旋半径
    float m_spiralRotations;       // 回転数

    // 現在の状態
    Vector3 m_currentPosition;
    Vector3 m_currentRotation;
    Vector3 m_currentVelocity;

    bool m_isActive;
    bool m_isLooping;              // ループ再生するか
    bool m_isInfiniteMode;         // 無限上昇モード

    // 無限上昇用
    int m_currentCycle;            // 現在のサイクル数
    float m_cycleHeight;           // 1サイクルあたりの高さ
    float m_cycleDistance;         // 1サイクルあたりの距離   

    ESpiralState m_state;
    bool    m_hasFinishedSpiral = false;
    Vector3 m_linearVelocity;
    Vector3 m_angularVelocity; // 回転速度（rad/sec）
    bool    m_hasEnteredLinear = false;

public:
    TitleSpiralEffect();

    // 初期化
    void Initialize(const Vector3& startPos, const Vector3& startRot, const Vector3& direction);

    // 開始
    void Start();

    // 停止
    void Stop();

    // 更新
    void Update(float deltaTime);

    // リセット（最初から再生）
    void Reset();

    // 現在の状態を取得
    Vector3 GetCurrentPosition() const { return m_currentPosition; }
    Vector3 GetCurrentRotation() const { return m_currentRotation; }
    Vector3 GetCurrentVelocity() const { return m_currentVelocity; }

    // パラメータ設定
    void SetSpiralHeight(float height) { m_spiralHeight = height; }
    void SetSpiralDistance(float distance) { m_spiralDistance = distance; }
    void SetSpiralRadius(float radius) { m_spiralRadius = radius; }
    void SetSpiralRotations(float rotations) { m_spiralRotations = rotations; }
    void SetDuration(float duration) { m_spiralDuration = duration; }
    void SetLooping(bool loop) { m_isLooping = loop; }
    void SetInfiniteMode(bool infinite) { m_isInfiniteMode = infinite; }  // ★追加

    // 状態取得
    bool IsActive() const { return m_isActive; }
    float GetProgress() const { return m_spiralTime / m_spiralDuration; }

private:
    // 螺旋位置計算
    void CalculateSpiralPosition(float t, Vector3& outPos, Vector3& outRot, Vector3& outVel, Vector3& outAngularVel);
    void CalculateStraightPosition(float t, Vector3& outPos, Vector3& outRot, Vector3& outVel);

    // ユーティリティ
    float Lerp(float a, float b, float t) const;
};