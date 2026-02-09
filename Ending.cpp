#include "Ending.h"
#include "system/CDirectInput.h"

void Ending::init()
{
    m_currentState = ResultState::PlayerApproaching;
    m_stateTimer = 0.0f;
    m_cameraTransitionTimer = 0.0f;

    // プレイヤーの初期化
    m_player = std::make_unique<Player>();
    m_player->Init();
    m_player->SetResultMode(true); // リザルトモードを有効化

    // プレイヤーの開始位置と終了位置
    m_playerStartPos = DirectX::XMFLOAT3(-1200.0f, 600.0f, 2000.0f);   // 左(X-)、上(Y+)、奥(Z+)
    m_playerEndPos = DirectX::XMFLOAT3(5.0f, 3.0f, -10.0f);        // やや右、地面、手前
    m_playerMoveProgress = 0.0f;
    m_player->SetPosition(m_playerStartPos);
	m_player->SetScale(Vector3(7.5f, 7.5f, 7.5f));

    // カメラの初期化
   
     //カメラを固定位置に設定（プレイヤーの着地点を基準に）
    Vector3 cameraPos(15.0f, 12.0f, -35.0f);      // 右やや後ろから
    Vector3 lookAtPos(0.0f, 5.0f, 0.0f);           // 画面中央やや上を注視

    ResultCamera::Instance().Init();
    ResultCamera::Instance().SetTargetPlayer(m_player.get());
    ResultCamera::Instance().SetFixedCamera(true, cameraPos, lookAtPos);

    //ResultCamera::Instance().Init();
    //ResultCamera::Instance().SetTargetPlayer(m_player.get());
    //ResultCamera::Instance().SetCameraState(ResultCamera::CameraState::WideShot);

    // スコア情報
    m_currentScore = m_gameScore;
    m_bestScore = 100;
    m_isNewRecord = (m_currentScore > m_bestScore);
    m_displayScore = 0;

    m_textScale = 0.0f;
    m_textAlpha = 0.0f;
    m_scoreSlideProgress = 0.0f;

    // UI要素
    if (m_isNewRecord)
    {
        m_newRecordText = new ScreenFixedBillboard(Vector2(0.3f, 0.5f), 0.3f, 0.3f, L"assets/texture/text/NewRecord.png");
        m_bestScore = m_currentScore;
        m_gameScore = 0.0f;
        SoundManager::GetInstance().PlayBGM("ResultHigh");
    }
    else
    {
        m_newRecordText = new ScreenFixedBillboard(Vector2(0.3f, 0.5f), 0.3f, 0.3f, L"assets/texture/text/Result.png");
        SoundManager::GetInstance().PlayBGM("ResultNormal");
    }

	m_currentScoreUI = new NumberRenderer();
	m_bestScoreUI = new NumberRenderer();
    m_currentScoreTargetPos = Vector2(0.55f, 0.5f);  // 画面右側
    m_bestScoreTargetPos = Vector2(0.55f, 0.3f);
    m_currentScoreUI->Init(Vector2(2.5f, 0.5f), 0.04f, 0.06f, 0.01f, true);  // X=1.3で画面外
    m_bestScoreUI->Init(Vector2(2.5f, 0.3f), 0.04f, 0.04f, 0.01f, true);
    m_currentScoreUI->SetNumber(static_cast<int>(m_currentScore));
    m_bestScoreUI->SetNumber(static_cast<int>(m_bestScore));

    //スコアの背景を表示
    m_BestGroundBillBoard = std::make_unique<ScreenFixedBillboard>(Vector2(0.75f, 0.265f), 0.04f*4, 0.06f * 3, L"assets/texture/text/Best.png");
    m_ScoreGroundBillBoard = std::make_unique<ScreenFixedBillboard>(Vector2(0.75f, 0.46f), 0.04f*4, 0.06f * 3, L"assets/texture/text/Score.png");

    //一度Updateを呼んでビルボードを作成
    m_currentScoreUI->Update(0.0f);
    m_bestScoreUI->Update(0.0f);

    // スライド進捗初期化
    m_currentScoreSlideProgress = 0.0f;
    m_bestScoreSlideProgress = 0.0f;

    //テスト
    m_sparkEmitter = std::make_unique<SparkEmitter>();
    if (!m_sparkEmitter->Init(Renderer::GetDevice()))
    {
        OutputDebugStringA("サンプラーステート作成失敗\n");
    }

    m_sparkleEmitter = std::make_unique<SparkEmitter>();
    if (!m_sparkleEmitter->Init(Renderer::GetDevice()))
    {
        OutputDebugStringA("キラキラエフェクト作成失敗\n");
    }

    // 新記録かどうかで色を変更
    m_sparkleEmitter->SetSparkleMode(m_isNewRecord, 1280.0f);  // 80.0f = 広範囲

    m_skydome = std::make_unique<Skydome>();
    m_skydome->Init();
}

void Ending::update(float deltatime)
{
    //m_currentScoreUI->TestAnimation();
    m_stateTimer += deltatime;
    m_cameraTransitionTimer += deltatime * CAMERA_TRANSITION_SPEED;
    if (m_cameraTransitionTimer > 1.0f) m_cameraTransitionTimer = 1.0f;

    // 状態ごとの更新処理
    switch (m_currentState)
    {
    case ResultState::PlayerApproaching:
        UpdatePlayerApproaching(deltatime);
        break;
    case ResultState::ShowResultText:
        UpdateShowResultText(deltatime);
        break;
    case ResultState::ShowScore:
        UpdateShowScore(deltatime);
        break;
    case ResultState::WaitInput:
        UpdateWaitInput(deltatime);
        break;
    }

    // カメラ更新
    ResultCamera::Instance().SetStateTransitionProgress(m_cameraTransitionTimer);
    ResultCamera::Instance().Update(deltatime);

    m_currentScoreUI->Update(deltatime);
    m_bestScoreUI->Update(deltatime);
	Vector3 camPos = ResultCamera::Instance().GetPosition();
	camPos.y -= 350.0f; //スカイドームを下にオフセット（下が見えるから）
	m_skydome->Update(camPos);


    DirectX::XMFLOAT3 pos = m_player.get()->GetPosition();
    //pos.x += m_ParticlePos.x;x	
    pos.y -= 5.0f;
    //pos.z += m_ParticlePos.z;
    DirectX::XMFLOAT3 dir = DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f);
    m_sparkEmitter->Emit(pos, dir);
    m_sparkEmitter->Update(deltatime);

    // キラキラエフェクト
    if (m_currentState >= ResultState::ShowResultText)
    {
        // 画面中央やや上でキラキラを発生
        DirectX::XMFLOAT3 sparklePos(0.0f, -15.0f, 0.0f);
        m_sparkleEmitter->Emit(sparklePos, dir, ParticleBehaviorType::Sparkle);
    }
    m_sparkleEmitter->Update(deltatime);
}

void Ending::UpdatePlayerApproaching(float deltatime)
{
    const float APPROACH_DURATION = 1.3f;

    m_playerMoveProgress += deltatime / APPROACH_DURATION;

    if (m_playerMoveProgress >= 1.0f)
    {
        m_playerMoveProgress = 1.0f;

        ResultCamera::Instance().SetFixedCamera(false, Vector3(), Vector3());
        ResultCamera::Instance().SetCameraState(ResultCamera::CameraState::CloseUp);
        m_cameraTransitionTimer = 0.0f;

        m_currentState = ResultState::ShowResultText;
        m_stateTimer = 0.0f;

        // 状態遷移と同時にアニメーション開始
        if (m_currentScoreUI) {
            m_currentScoreUI->StartAnimation(NumberRenderer::AnimationType::ScaleBounce, 0.6f);
        }
        if (m_bestScoreUI) {
            m_bestScoreUI->StartAnimation(NumberRenderer::AnimationType::ScaleBounce, 0.8f); // 少し遅れて
        }
    }

    // イージングを使った滑らかな動き
    float easedProgress = EaseOutCubic(m_playerMoveProgress);

    // XYZ全て補間
    float currentX = Lerp(m_playerStartPos.x, m_playerEndPos.x, easedProgress);
    float currentZ = Lerp(m_playerStartPos.z, m_playerEndPos.z, easedProgress);

    // Y軸は放物線で自然な落下
    // 開始位置から終了位置へ、途中で少し高く上がる
    float linearY = Lerp(m_playerStartPos.y, m_playerEndPos.y, easedProgress);
    // sin関数で放物線を作る（0〜π で山なり）
    float arc = sin(easedProgress * 3.14159f) * 8.0f;  // 8.0fは放物線の高さ
    float currentY = linearY + arc;

    DirectX::XMFLOAT3 currentPos(currentX, currentY, currentZ);
    m_player->SetPosition(currentPos);

    // オプション：プレイヤーを進行方向に向ける
    // 次のフレームの位置を計算
    float nextProgress = m_playerMoveProgress + 0.01f;
    if (nextProgress < 1.0f)
    {
        float nextX = Lerp(m_playerStartPos.x, m_playerEndPos.x, nextProgress);
        float nextZ = Lerp(m_playerStartPos.z, m_playerEndPos.z, nextProgress);

        // 進行方向ベクトル
        float dirX = nextX - currentX;
        float dirZ = nextZ - currentZ;

        // Y軸回転角度を計算（atan2で方向を求める）
        float angle = atan2(dirX, dirZ);

        // プレイヤーの回転を設定（必要に応じて）
        DirectX::XMFLOAT3 rotation(0.0f, DirectX::XMConvertToDegrees(angle), 0.0f);
        //m_player->SetRotation(rotation);  // もしSetRotationメソッドがあれば
    }
}

void Ending::UpdateShowResultText(float deltatime)
{
    const float TEXT_POP_DURATION = 0.5f;

    if (m_stateTimer < TEXT_POP_DURATION)
    {
        float t = m_stateTimer / TEXT_POP_DURATION;
        m_textScale = EaseOutElastic(t);
        m_textAlpha = t;
    }
    else
    {
        m_textScale = 1.0f;
        m_textAlpha = 1.0f;

        ResultCamera::Instance().SetCameraState(ResultCamera::CameraState::Result);
        m_cameraTransitionTimer = 0.0f;

        m_currentState = ResultState::ShowScore;
        m_stateTimer = 0.0f;
    }
}
void Ending::UpdateShowScore(float deltatime)
{
    const float SCORE_SLIDE_DURATION = 0.5f;      // スライドイン時間
    const float SCORE_DELAY = 0.2f;               // ハイスコアの遅延
    const float SCORE_COUNT_DURATION = 1.0f;      // カウントアップ時間

    // 今回のスコアをスライドイン
    if (m_stateTimer < SCORE_SLIDE_DURATION)
    {
        m_currentScoreSlideProgress = m_stateTimer / SCORE_SLIDE_DURATION;
        m_currentScoreSlideProgress = EaseOutCubic(m_currentScoreSlideProgress);

        // 画面外(X=1.3)から目標位置(X=0.85)へ
        float currentX = Lerp(m_currentScoreUI->GetPosition().x, m_currentScoreTargetPos.x, m_currentScoreSlideProgress);
        m_currentScoreUI->SetPosition(Vector2(currentX, m_currentScoreTargetPos.y));
        m_currentScoreUI->SetNumber(static_cast<int>(m_currentScore));
    }

    // ハイスコアをスライドイン（少し遅れて）
    if (m_stateTimer > SCORE_DELAY && m_stateTimer < SCORE_DELAY + SCORE_SLIDE_DURATION)
    {
        m_bestScoreSlideProgress = (m_stateTimer - SCORE_DELAY) / SCORE_SLIDE_DURATION;
        m_bestScoreSlideProgress = EaseOutCubic(m_bestScoreSlideProgress);

        float bestX = Lerp(m_bestScoreUI->GetPosition().x, m_bestScoreTargetPos.x, m_bestScoreSlideProgress);
        m_bestScoreUI->SetPosition(Vector2(bestX, m_bestScoreTargetPos.y));
    }

    // スコアのカウントアップ（スライド完了後）
    if (m_stateTimer > SCORE_SLIDE_DURATION + SCORE_DELAY)
    {
        float countStartTime = SCORE_SLIDE_DURATION + SCORE_DELAY;
        float countProgress = (m_stateTimer - countStartTime) / SCORE_COUNT_DURATION;

        if (countProgress < 1.0f)
        {
            // 0から最終スコアまでカウントアップ
            int displayScore = (int)(m_currentScore * countProgress);
            m_currentScoreUI->SetNumber(m_currentScore);

        }
        else
        {
            m_currentScoreUI->SetNumber(m_currentScore);

            // カウントアップ完了
            if (m_stateTimer > countStartTime + SCORE_COUNT_DURATION + 0.5f)
            {
                m_currentState = ResultState::WaitInput;
                m_stateTimer = 0.0f;
            }
        }
    }
}

void Ending::UpdateWaitInput(float deltatime)
{
    if (CDirectInput::GetInstance().CheckKeyBuffer(DIK_RETURN))
    {
        SceneManager::ChangeScene("Title");
    }

}

void Ending::draw(float deltatime)//文字動かないンゴゴゴ あとメモリリークヤバイ
{
    ResultCamera::Instance().Draw();

    if (m_isNewRecord)
    {
        m_skydome->Draw(true);
    }
    else
    {
        m_skydome->Draw(false);
    }
    m_player->Draw();
    m_sparkEmitter->Render(Renderer::GetDeviceContext(), DirectX::XMMatrixIdentity());
    m_sparkleEmitter->Render(Renderer::GetDeviceContext(), DirectX::XMMatrixIdentity());
    if (m_currentState >= ResultState::ShowResultText)
    {
        m_BestGroundBillBoard->Draw();
        if (m_newRecordText)
            m_newRecordText->Draw();
    }

    if (m_currentState >= ResultState::ShowScore)
    {
        m_ScoreGroundBillBoard->Draw();
        if (m_currentScoreUI)
            m_currentScoreUI->Draw(false);
        if (m_bestScoreUI)
            m_bestScoreUI->Draw(false);
    }
}

void Ending::loadAsync()
{
}

void Ending::dispose()
{
    SoundManager::GetInstance().StopBGM();
    delete m_TitleBillboard;
    delete m_screenBillboard;
    delete m_newRecordText;
    delete m_currentScoreUI;
    delete m_bestScoreUI;
}

float Ending::EaseOutCubic(float t)
{
    return 1.0f - pow(1.0f - t, 3.0f);
}

float Ending::EaseOutElastic(float t)
{
    const float c4 = (2.0f * 3.14159f) / 3.0f;

    if (t == 0.0f) return 0.0f;
    if (t == 1.0f) return 1.0f;

    return pow(2.0f, -10.0f * t) * sin((t * 10.0f - 0.75f) * c4) + 1.0f;
}

float Ending::Lerp(float start, float end, float t)
{
    return start + (end - start) * t;
}