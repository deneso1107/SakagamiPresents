#include "Title.h"
#include "system/CDirectInput.h"

#include"TitleCamera.h"

void Title::init()
{
    // スプライトの初期化
    m_screenBillboard = new ScreenFixedBillboard(Vector2(0.5f, 0.9f), 0.3f, 0.3f, L"assets/texture/Button.png");

    // タイトルは画面外上部から開始
    m_TitleBillboard = new ScreenFixedBillboard(Vector2(0.5f, 0.3f), 0.6f, 0.6f, L"assets/texture/タイトル.png");

	// プレイヤーの初期化
    m_player = std::make_unique<Player>();
    m_player->Init();

	m_sparkEmitter = std::make_unique<SparkEmitter>();
    if (!m_sparkEmitter->Init(Renderer::GetDevice()))
    {
        OutputDebugStringA("サンプラーステート作成失敗\n");
    }

	// 螺旋エフェクトの初期化
    m_spiralEffect = std::make_unique < TitleSpiralEffect>();
    m_spiralEffect->Initialize(m_player->GetPosition(), m_player->GetRotation(), m_player->GetForwardVector());

     m_spiralEffect->SetSpiralHeight(1600.0f);
     m_spiralEffect->SetSpiralDistance(30.0f);
     m_spiralEffect->SetSpiralRadius(6.0f);
     m_spiralEffect->SetSpiralRotations(5.0f);   // たくさん回転
     m_spiralEffect->SetDuration(3.0f);          // 速い
     m_spiralEffect->SetInfiniteMode(true);   // 無限上昇ON
     m_spiralEffect->SetLooping(true);        // ループは不要

    m_spiralEffect->Start();
    ///カメラの初期化
    TitleCamera& titleCam = TitleCamera::Instance();
    titleCam.Init();
    titleCam.SetTargetPlayer(m_player.get());

    // カメラオフセットをカスタマイズ（オプション）
    titleCam.SetCameraOffset(Vector3(8.0f, 0.0f, -30.0f));  // より後ろから

    titleCam.SetFixedMode(false);  // 固定モード

    // アニメーション用の変数初期化
    m_titlePosY = -0.3f;          // 開始位置（画面外上部）
    m_targetPosY = 0.3f;          // 目標位置
    m_titleVelocityY = 0.0f;      // 初期速度
    m_isAnimating = true;         // アニメーション中フラグ

	m_skydome = std::make_unique<Skydome>();
	m_skydome->Init();

    SoundManager::GetInstance().PlayBGM("titlebgm");

}

void Title::update(float deltatime)
{
    if (CDirectInput::GetInstance().CheckKeyBuffer(DIK_RETURN)|| InputManager::GetInstance()->GetButton(SDL_CONTROLLER_BUTTON_A))
    {
        SceneManager::ChangeScene("CarDriveScene",true);
        SoundManager::GetInstance().StopBGM();
    }

    m_spiralEffect->Update(deltatime);

    //プレイヤーの位置と回転をエフェクトから取得
    if (m_spiralEffect->IsActive())
    {
        Vector3 pos = m_spiralEffect->GetCurrentPosition();
        Vector3 rot = m_spiralEffect->GetCurrentRotation();
        Vector3 vel = m_spiralEffect->GetCurrentVelocity();

        // プレイヤーに適用
        m_player->SetPosition(pos);
        m_player->SetRotation(rot);
        m_player->SetVelocity(vel);  // 炎エフェクト用
    }


    //m_VideoBB->Update();
    //if (m_VideoBB && m_VideoBB->GetVideoPlayer()) {
    //    if (!m_VideoBB->GetVideoPlayer()->IsValid()) {
    //        OutputDebugStringA("VideoPlayer が無効です\n");
    //    }
    //}  　　

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
            m_TitleBillboard->SetScreenPosition(Vector2(0.5f, m_titlePosY));
        }
        m_TitleBillboard->Update();
    }

	TitleCamera::Instance().Update(deltatime);

    DirectX::XMFLOAT3 pos = m_player.get()->GetPosition();
    //pos.x += m_ParticlePos.x;x	
    pos.y -= 5.0f;
    //pos.z += m_ParticlePos.z;
    DirectX::XMFLOAT3 dir = DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f);
    m_sparkEmitter->Emit(pos, dir);
    m_sparkEmitter->Update(deltatime);

	m_skydome->Update(TitleCamera::Instance().GetPosition());
}

void Title::draw(float deltatime)
{
    // カメラのビュー行列を設定
    TitleCamera::Instance().Draw();
    m_player->Draw();
 
    m_skydome->Draw(deltatime);
    m_sparkEmitter->Render(Renderer::GetDeviceContext(), DirectX::XMMatrixIdentity());
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
    m_player->Dispose();
    changepic = false;
}