#pragma once
#include "system/IScene.h"
#include "scenemanager.h"
#include <string>
#include "ScreenFixedBillboard.h"
#include "Player.h"
#include "TitleMove.h"
#include "Skydome.h"
#include "SparkEmitter.h"
#include "ResultCamera.h"  // 追加
#include"NumberRenderer.h"

class Ending : public IScene
{
public:
    void update(float deltatime) override;
    void draw(uint64_t deltatime) override;
    void init() override;
    void loadAsync() override;
    void dispose() override;

private:
    // 演出の状態管理
    enum class ResultState
    {
        PlayerApproaching,
        ShowResultText,
        ShowScore,
        WaitInput
    };

    ResultState m_currentState;
    float m_stateTimer;

    // カメラ遷移用
    float m_cameraTransitionTimer;
    const float CAMERA_TRANSITION_SPEED = 2.0f;  // カメラ遷移の速度

    // プレイヤー演出用
    DirectX::XMFLOAT3 m_playerStartPos;
    DirectX::XMFLOAT3 m_playerEndPos;
    float m_playerMoveProgress;

    // 文字演出用
    float m_textScale;
    float m_textAlpha;
    bool m_isNewRecord;

    // スコア演出用
    int m_currentScore;
    int m_bestScore;
    int m_displayScore;
    float m_scoreSlideProgress;

    // UI要素
    ScreenFixedBillboard* m_screenBillboard;
    ScreenFixedBillboard* m_TitleBillboard;
    ScreenFixedBillboard* m_newRecordText;
    NumberRenderer* m_currentScoreUI;
    NumberRenderer* m_bestScoreUI;


    std::unique_ptr<Player> m_player;
    std::unique_ptr<TitleSpiralEffect> m_spiralEffect;
    std::unique_ptr<Skydome> m_skydome;
    std::unique_ptr<SparkEmitter> m_sparkEmitter;
    std::unique_ptr<SparkEmitter> m_sparkleEmitter;      // キラキラ用

    void UpdatePlayerApproaching(float deltatime);
    void UpdateShowResultText(float deltatime);
    void UpdateShowScore(float deltatime);
    void UpdateWaitInput(float deltatime);

    float EaseOutCubic(float t);
    float EaseOutElastic(float t);
    float Lerp(float start, float end, float t);
};