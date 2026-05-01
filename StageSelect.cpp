#include "StageSelect.h"
void StageSelect::init()
{
    m_selectIconBillBoard = std::make_unique<ScreenFixedBillboard>(Vector2(0.15f, 0.15f), 0.3f, 0.3f, L"assets/texture/SelectIcon.png");

	m_stage1BillBoard = std::make_unique<ScreenFixedBillboard>(Vector2(0.5f, 0.80f), 0.4f, 0.4f, L"assets/texture/stage1.png");
	m_stage2BillBoard = std::make_unique<ScreenFixedBillboard>(Vector2(0.5f, 0.80f), 0.4f, 0.4f, L"assets/texture/stage2.png");
	m_stage3BillBoard = std::make_unique<ScreenFixedBillboard>(Vector2(0.5f, 0.80f), 0.4f, 0.4f, L"assets/texture/stage3.png");

    // 矢印ビルボード初期化
    m_leftArrowBillBoard = std::make_unique<ScreenFixedBillboard>(
        Vector2(0.1f, 0.5f), 0.2f, 0.4f, L"assets/texture/leftArrow.png");
    m_rightArrowBillBoard = std::make_unique<ScreenFixedBillboard>(
        Vector2(0.9f, 0.5f), 0.2f, 0.4f, L"assets/texture/rightArrow.png");

    // 位相をずらしてバラバラに動かす
    m_leftArrow.Init(0.5f, 0.2f, 0.4f, 0.0f);
    m_rightArrow.Init(0.5f, 0.2f, 0.4f, 1.0f);

	m_stageManager.RegisterStage<StageModel1>();
	m_stageManager.RegisterStage<StageModel2>();
	m_stageManager.RegisterStage<StageModel3>();

    m_camera.Init();
    m_camera.SetFixedPosition(Vector3(0.0f, 20.0f, 200.0f), // カメラ位置
        Vector3(0.0f, 10.0f, 0.0f)); // 注視点
    m_uiAnimator.ForceSetPosition(Vector2(0.5f, 0.8f));
    m_uiAnimator.SetTargetPosition(Vector2(0.5f, 0.80f));

    m_skydome = std::make_unique<Skydome>();
    m_skydome->Init();

    // プレイヤーの初期化
    m_player = std::make_unique<Player>();
    m_player->Init();

    m_sparkEmitter = std::make_unique<SparkEmitter>();
    if (!m_sparkEmitter->Init(Renderer::GetDevice()))
    {
        OutputDebugStringA("サンプラーステート作成失敗\n");
    }

    m_decideEffect.Start(m_stageManager.GetCurrent()->GetPosition());
    m_player->SetPosition(m_decideEffect.GetCurrentPosition());

    SoundManager::GetInstance().PlayBGM("selectbgm");
}
void StageSelect::update(float deltatime)
{
    bool moved = false;
    float spinDir = 0.0f;

    // 入力処理
    if (CDirectInput::GetInstance().CheckKeyBufferTrigger(DIK_D) ||
        InputManager::GetInstance()->GetButtonTrigger(SDL_CONTROLLER_BUTTON_RIGHTSHOULDER)) 
    {
        m_rightArrow.OnPress();
        m_stageManager.Prev();
        m_currentBillboardIndex = m_stageManager.GetCurrentIndex();
        m_uiAnimator.Play(-1.0f); // 左入力
        spinDir = -1.0f;
        SoundManager::GetInstance().PlaySE("Click", 0.25f);
    }
    if (CDirectInput::GetInstance().CheckKeyBufferTrigger(DIK_A) ||
        InputManager::GetInstance()->GetButtonTrigger(SDL_CONTROLLER_BUTTON_LEFTSHOULDER))
    {
        m_leftArrow.OnPress();
        m_stageManager.Next();
        m_currentBillboardIndex = m_stageManager.GetCurrentIndex();
        m_uiAnimator.Play(1.0f);  // 右入力
        spinDir = 1.0f;
        SoundManager::GetInstance().PlaySE("Click", 0.25f);
    }

    if (CDirectInput::GetInstance().CheckKeyBufferTrigger(DIK_RETURN) ||
        InputManager::GetInstance()->GetButtonTrigger(SDL_CONTROLLER_BUTTON_A)) {
        if (!m_isDeciding) {
            m_isDeciding = true;
            m_decideEffect.Start(m_stageManager.GetCurrent()->GetPosition());
            // UIスライドアウト
            m_uiAnimator.ForceSetPosition(Vector2(0.5f, 1.5f));
            SoundManager::GetInstance().PlaySE("Click", 0.25f);
        }
    }

    m_decideEffect.Update(deltatime);
    if (m_decideEffect.IsActive()) {
        m_player->SetPosition(m_decideEffect.GetCurrentPosition());
        m_player->SetRotation(m_decideEffect.GetCurrentRotation());
        m_player->SetVelocity(m_decideEffect.GetCurrentVelocity());
    }

    // 演出完了でシーン遷移
    if (m_isDeciding && m_decideEffect.IsDone())
    {
        SoundManager::GetInstance().StopBGM();
        SceneManager::ChangeScene("CarDriveScene", true);
		SceneManager::SetStageNumber(m_stageManager.GetCurrentIndex());
    }


    // 毎フレームリリース（Triggerなので押した次のフレームには戻る）
    m_leftArrow.OnRelease();
    m_rightArrow.OnRelease();

    // アニメーション更新
    m_uiAnimator.Update(deltatime);
    m_leftArrow.Update(deltatime);
    m_rightArrow.Update(deltatime);
    m_leftArrow.Apply(m_leftArrowBillBoard.get(), 0.1f);
    m_rightArrow.Apply(m_rightArrowBillBoard.get(), 0.9f);

    // 現在のビルボードに適用
    ScreenFixedBillboard* currentBillboard = nullptr;
    switch (m_currentBillboardIndex)
    {
    case 0: currentBillboard = m_stage1BillBoard.get(); break;
    case 1: currentBillboard = m_stage2BillBoard.get(); break;
    case 2: currentBillboard = m_stage3BillBoard.get(); break;
    }
    m_uiAnimator.Apply(currentBillboard);


	m_stageManager.Update(deltatime);
	m_camera.Update(deltatime);

    DirectX::XMFLOAT3 pos = m_player.get()->GetPosition();
    //pos.x += m_ParticlePos.x;x	
    pos.y -= 5.0f;
    //pos.z += m_ParticlePos.z;
    DirectX::XMFLOAT3 dir = DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f);
    m_sparkEmitter->Emit(pos, dir);
    m_sparkEmitter->Update(deltatime);

    m_skydome->Update(m_camera.GetPosition());
}
void StageSelect::draw(float deltatime)
{
	m_camera.Draw();
    m_stageManager.DrawAll();
    m_skydome->Draw(false);
	m_selectIconBillBoard->Draw();

    // 現在のインデックスのビルボードだけ描画
    switch (m_currentBillboardIndex) {
    case 0: m_stage1BillBoard->Draw(); break;
    case 1: m_stage2BillBoard->Draw(); break;
    case 2: m_stage3BillBoard->Draw(); break;
    }

    m_leftArrowBillBoard->Draw();
    m_rightArrowBillBoard->Draw();
	//決定した時だけプレイヤーと火花を描画
    if (m_isDeciding)
    {
        m_player->Draw();
        m_sparkEmitter->Render(Renderer::GetDeviceContext(), DirectX::XMMatrixIdentity());
    }
}
void  StageSelect::dispose()
{
    m_isDeciding = false;
	m_stageManager.Dispose();
}
void StageSelect::loadAsync()
{
}