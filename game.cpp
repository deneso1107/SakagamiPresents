#include	"system/renderer.h"
#include    "system/DebugUI.h"
#include    "system/CDirectInput.h"
#include	"scenemanager.h"
#include	"fpscontrol.h"
#include"GameManager.h"
void gameinit() 
{
	// レンダラの初期化    
	Renderer::Init();

	// DirectInputの初期化
	CDirectInput::GetInstance().Init(Application::GetHInstance(), 
		Application::GetWindow(),
		Application::GetWidth(),
		Application::GetHeight());

	// シーンマネージャの初期化
	SceneManager::Init();

	// デバッグUIの初期化
	DebugUI::Init(Renderer::GetDevice(), Renderer::GetDeviceContext());

}

void gameupdate(uint64_t deltatime)
{
	CDirectInput::GetInstance().GetKeyBuffer();		// キーボードの状態を取得
	CDirectInput::GetInstance().GetMouseState();	// マウスの状態を取得
	GameManager::Instance().Update(deltatime);//わからんンんンんンん！！！！！！！！！(バグ一覧　重力ゲージ適応されないし、unini64とfloatが喧嘩してる　終わり)
	// シーンマネージャの更新
	float scaledDelta = GameManager::Instance().GetScaledDelta();
	SceneManager::Update(scaledDelta);//こーーーーーーーーれ基底クラス

}

void gamedraw(uint64_t deltatime) 
{
	// ========== 第1パス: シャドウマップ生成 ==========
	if (Renderer::IsShadowMapEnabled())
	{
		//printf("\n========== SHADOW MAP PASS START ==========\n");

		Renderer::BeginShadowPass();

		// ここが重要！SceneManager::Draw()が呼ばれているか確認
		//printf("Calling SceneManager::Draw for shadow map...\n");
		float scaledDelta = GameManager::Instance().GetScaledDelta();
		SceneManager::Draw(scaledDelta);
		//printf("SceneManager::Draw completed\n");

		Renderer::EndShadowPass();

		//printf("========== SHADOW MAP PASS END ==========\n\n");
	}
	else
	{
		//printf("Shadow map is DISABLED\n");
	}

	// ========== 第2パス: 通常描画 ==========
	//printf("========== NORMAL PASS START ==========\n");

	Renderer::Begin();

	float scaledDelta = GameManager::Instance().GetScaledDelta();
	SceneManager::Draw(scaledDelta);

	DebugUI::Render();

	Renderer::End();

	//printf("========== NORMAL PASS END ==========\n\n");
}

void gamedispose() 
{
	// デバッグUIの終了処理
	DebugUI::DisposeUI();

	// シーンマネージャの終了処理
	SceneManager::Dispose();	// デバッグUIの描画

	// レンダラの終了処理
	Renderer::Uninit();

}

void gameloop()
{
	static FPS fpsrate(65);
	static uint64_t accumulator = 0;
	const uint64_t FIXED_DELTA_TIME = 1000000 / 65; // 約15384マイクロ秒

	uint64_t frame_time = fpsrate.CalcDelta();
	accumulator += frame_time;

	// 固定タイムステップで更新処理を実行
	/*while (accumulator >= FIXED_DELTA_TIME) {
		gameupdate(FIXED_DELTA_TIME);
		accumulator -= FIXED_DELTA_TIME;
	}*/
	gameupdate(FIXED_DELTA_TIME);
	// 描画は毎フレーム実行
	gamedraw(FIXED_DELTA_TIME);

	fpsrate.Tick();
	//uint64_t delta_time = 0;
	//// フレームの待ち時間を計算する
	//static FPS fpsrate(65);
	//// 前回実行されてからの経過時間を計算する
	//delta_time = fpsrate.CalcDelta();
	////std::cout << "Delta Time (seconds): " << delta_time / 1000000.0f << std::endl;//ここ鈴木先生に聞きましょう
	//// 更新処理、描画処理を呼び出す
	//gameupdate(delta_time);
	//gamedraw(delta_time);
	//// 規定時間までWAIT
	//fpsrate.Tick();
}