#include "Title.h"
#include "system/CDirectInput.h"

void Title::init()
{
    // スプライトの初期化
    m_screenBillboard = new ScreenFixedBillboard(Vector2(0.5f, 0.9f), 0.3f, 0.3f, L"assets/texture/Button.png");

    // タイトルは画面外上部から開始
    m_TitleBillboard = new ScreenFixedBillboard(Vector2(0.5f, 0.3f), 0.6f, 0.6f, L"assets/texture/タイトル.png");

    m_VideoBB = ScreenFixedBillboard::CreateFromVideo(
        Vector2(0.5f, 0.5f),
        1.0f, 1.0f,
        L"assets/video/画面録画 2025-12-18 173922.mp4"
    );

    if (m_VideoBB == nullptr) {
        MessageBoxA(nullptr, "動画の読み込みに失敗しました", "Error", MB_OK);
    }
    else {
        // 動画情報を確認
        VideoPlayer* player = m_VideoBB->GetVideoPlayer();
        if (player) {
            char msg[256];
            printf("動画サイズ: %dx%d\n長さ: %.2f秒",
                player->GetWidth(),
                player->GetHeight(),
                player->GetDurationSeconds());
            //MessageBoxA(nullptr, "Video Info", "OK", MB_OK);
        }
    }
    m_VideoBB->SetLooping(true);
    m_VideoBB->PlayVideo();

    // アニメーション用の変数初期化
    m_titlePosY = -0.3f;          // 開始位置（画面外上部）
    m_targetPosY = 0.3f;          // 目標位置
    m_titleVelocityY = 0.0f;      // 初期速度
    m_isAnimating = true;         // アニメーション中フラグ
}

void Title::update(float deltatime)
{
    if (CDirectInput::GetInstance().CheckKeyBuffer(DIK_RETURN))
    {
        SceneManager::ChangeScene("CarDriveScene", true);
    }

    m_VideoBB->Update();
    if (m_VideoBB && m_VideoBB->GetVideoPlayer()) {
        if (!m_VideoBB->GetVideoPlayer()->IsValid()) {
            OutputDebugStringA("VideoPlayer が無効です\n");
        }
    }

    // タイトルの落下アニメーション
    if (m_isAnimating)
    {
        const float gravity = 2.0f;      // 重力加速度
        const float bounce = 0.6f;       // バウンス係数（0.0～1.0）
        const float stopThreshold = 0.01f; // 停止判定の閾値

        // 重力を加える
        m_titleVelocityY += gravity * deltatime;

        // 位置を更新
        m_titlePosY += m_titleVelocityY * deltatime;

        // 目標位置を超えたらバウンス
        if (m_titlePosY >= m_targetPosY)
        {
            m_titlePosY = m_targetPosY;
            m_titleVelocityY = -m_titleVelocityY * bounce;

            // 速度が小さくなったら停止
            if (abs(m_titleVelocityY) < stopThreshold)
            {
                m_titleVelocityY = 0.0f;
                m_isAnimating = false;
            }
        }

        // ビルボードの位置を更新
        if (m_TitleBillboard)
        {
            m_TitleBillboard->SetScreenPosition(Vector2(0.5f, 0.1));
        }
    }
}

void Title::draw(uint64_t deltatime)
{
    m_VideoBB->Draw();
    m_screenBillboard->Draw();
    m_TitleBillboard->Draw();
}

void Title::loadAsync()
{
}

void Title::dispose()
{
    if (m_screenBillboard)
    {
        delete m_screenBillboard;
        m_screenBillboard = nullptr;
    }

    if (m_TitleBillboard)
    {
        delete m_TitleBillboard;
        m_TitleBillboard = nullptr;
    }

    changepic = false;
}