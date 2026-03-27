#pragma once
#include <SDL.h>
#include <stdio.h>
#include <string.h>
class InputManager
{
private:
    static InputManager* instance;
    SDL_GameController* controller;

    // 前フレームのボタン状態
    bool m_prevButtons[SDL_CONTROLLER_BUTTON_MAX] = {};
    bool m_currButtons[SDL_CONTROLLER_BUTTON_MAX] = {};

    // コンストラクタをprivateに
    InputManager();
    ~InputManager();

public:
    // コピー禁止
    InputManager(const InputManager&) = delete;
    InputManager& operator=(const InputManager&) = delete;

    // インスタンス取得
    static InputManager* GetInstance();
    static void Destroy();

    // 押した瞬間だけtrue
    bool GetButtonTrigger(SDL_GameControllerButton button);

    // 初期化・終了
    bool Initialize();
    void Shutdown();

    // 毎フレーム更新
    void Update();

    // 入力取得
    bool GetButton(SDL_GameControllerButton button);
    float GetAxis(SDL_GameControllerAxis axis);

    // コントローラーが接続されているか
    bool IsConnected() const;
};
