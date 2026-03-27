#include "InputManager.h"
InputManager* InputManager::instance = nullptr;

InputManager::InputManager()
    : controller(nullptr)
{
}

InputManager::~InputManager()
{
    Shutdown();
}

InputManager* InputManager::GetInstance()
{
    if (instance == nullptr)
    {
        instance = new InputManager();
    }
    return instance;
}

void InputManager::Destroy()
{
    if (instance != nullptr)
    {
        delete instance;
        instance = nullptr;
    }
}

bool InputManager::Initialize()
{
    if (SDL_Init(SDL_INIT_GAMECONTROLLER) < 0)
    {
        printf("コントローラーなし");
        return false;
    }

    // コントローラーを検索
    for (int i = 0; i < SDL_NumJoysticks(); i++)
    {
        if (SDL_IsGameController(i))
        {
            controller = SDL_GameControllerOpen(i);
            if (controller)
            {
                break;
            }
        }
        else
        {
			return false;
        }
    }

    return true;
}

void InputManager::Shutdown()
{
    if (controller != nullptr)
    {
        SDL_GameControllerClose(controller);
        controller = nullptr;
    }
    SDL_Quit();
}

void InputManager::Update()
{
    // 前フレームの状態を保存
    memcpy(m_prevButtons, m_currButtons, sizeof(m_currButtons));

    // 現在の状態を更新
    if (controller) {
        for (int i = 0; i < SDL_CONTROLLER_BUTTON_MAX; i++) {
            m_currButtons[i] = SDL_GameControllerGetButton(
                controller, (SDL_GameControllerButton)i) != 0;
        }
    }

    // イベント処理（コントローラーの抜き差し検知など）
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        if (event.type == SDL_CONTROLLERDEVICEADDED)
        {
            if (controller == nullptr)
            {
                controller = SDL_GameControllerOpen(event.cdevice.which);
            }
        }
        else if (event.type == SDL_CONTROLLERDEVICEREMOVED)
        {
            if (controller && event.cdevice.which == SDL_JoystickInstanceID(
                SDL_GameControllerGetJoystick(controller)))
            {
                SDL_GameControllerClose(controller);
                controller = nullptr;
            }
        }
    }
}

bool InputManager::GetButton(SDL_GameControllerButton button)
{
    return m_currButtons[button];
}

float InputManager::GetAxis(SDL_GameControllerAxis axis)
{
    if (controller == nullptr) return 0.0f;

    Sint16 value = SDL_GameControllerGetAxis(controller, axis);

    // デッドゾーン処理
    const int DEADZONE = 8000;
    if (abs(value) < DEADZONE) return 0.0f;

    // -1.0 ~ 1.0 に正規化
    return value / 32767.0f;
}

//押した瞬間だけtrue
bool InputManager::GetButtonTrigger(SDL_GameControllerButton button)
{
    return m_currButtons[button] && !m_prevButtons[button];
}

bool InputManager::IsConnected() const
{
    return controller != nullptr;
}