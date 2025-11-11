#pragma once
#include <algorithm>
class SceneManager;
class PlayerStateManager;
class GameManager
{
public:
    static GameManager& Instance()
    {
        static GameManager instance;
        return instance;
    }
    void Initialize(SceneManager* sceneMgr, PlayerStateManager* playerMgr)
    {
        m_sceneManager = sceneMgr;
        m_playerStateManager = playerMgr;
        m_TimeScale = 1.0f;
    }


    void Update(uint64_t deltaTimeMicros) {
        // スムーズな遷移
        float deltaTimeSeconds = static_cast<float>(deltaTimeMicros) / 1000000.0f;
        if (abs(m_TimeScale - m_TargetTimeScale) > 0.001f) {
            m_TimeScale += (m_TargetTimeScale - m_TimeScale)* deltaTimeSeconds * smoothSpeed;
        }
        m_scaledDeltaTime = deltaTimeSeconds * m_TimeScale;
        // --- 一時停止処理 ---
        if (m_isPaused)
            m_scaledDeltaTime = 0.0f;
    }

    void SetTimeScale(float scale)
    {
        m_TargetTimeScale = std::clamp(scale, 0.0f, 2.0f);
    }
    float GetTimeScale() const { return m_TimeScale; }
    float GetScaledDelta() const { return m_scaledDeltaTime; }
    void Pause() { m_isPaused = true; }
    void Resume() { m_isPaused = false; }
    bool IsPaused() const { return m_isPaused; }

private:
    GameManager() = default;
    SceneManager* m_sceneManager = nullptr;
    PlayerStateManager* m_playerStateManager = nullptr;
    float m_TimeScale = 1.0f;
    float m_TargetTimeScale = 1.0f;///TimeScaleの補完をする際に使用
    float m_scaledDeltaTime = 0.0f; // スケーリングされたデルタタイム
    float smoothSpeed = 3.0f; // ← 遷移の速さ（好みで調整）
    bool m_isPaused = false;
};//GameManagerの続き

