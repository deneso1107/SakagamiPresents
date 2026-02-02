#include    <memory>
#include    <string>
#include    "system/commontypes.h"
#include	"system/collision.h"
#include	"CarDriveScene.h"
#include	"system/renderer.h"
#include    "system/CDirectInput.h"
#include	"system/CPlaneMesh.h"
#include    "Player.h"
#include    "Skydome.h"
#include    "Enemies.h"
// 平行光源の方向セット
void CarDriveScene::debugDirectionalLight()
{
	static Vector4 direction = Vector4(0.0f, -1.0f, 1.0f, 0.0f);
	static bool enableShadow = true;
	static bool showShadowMap = false;
	static float lightDistance = 50.0f;  // 追加
	static float orthoSize = 100.0f;     // 追加
	static float TestSize_ = 1.0f;     // 追加

	ImGui::Begin("debug Directional Light");
	ImGui::SliderFloat3("direction", &direction.x, -1, 1);
	ImGui::Checkbox("Enable Shadow", &enableShadow);
	ImGui::Checkbox("Show Shadow Map", &showShadowMap);
	ImGui::SliderFloat("Light Distance", &lightDistance, 10.0f, 200.0f);  // 追加
	ImGui::SliderFloat("Ortho Size", &orthoSize, 10.0f, 500.0f);          // 追加
	ImGui::SliderFloat("Ortho Size", &TestSize_, 1.0f, 500.0f);          // 追加

	Renderer::EnableShadowMap(enableShadow);

	direction.Normalize();

	// ライト情報のセット
	LIGHT light{};
	light.Enable = true;
	light.Direction = direction;
	light.Direction.Normalize();
	light.Ambient = Color(0.2f, 0.2f, 0.2f, 1.0f);
	light.Diffuse = Color(1.0f, 1.0f, 1.0f, 1.0f);
	Renderer::SetLight(light);

	// プレイヤーの位置を取得（もしあれば）
	 Vector3 playerPos = m_player->GetPosition();

	// ライトをプレイヤーの上に配置
	DirectX::XMVECTOR lightPos = DirectX::XMVectorSet(
		playerPos.x,  // playerPos.x
		lightDistance,  // 上から照らす
		playerPos.z,  // playerPos.z
		1.0f
	);

	DirectX::XMVECTOR targetPos = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
	DirectX::XMVECTOR upVec = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	DirectX::XMMATRIX lightView = DirectX::XMMatrixLookAtLH(lightPos, targetPos, upVec);
	DirectX::XMMATRIX lightProjection = DirectX::XMMatrixPerspectiveFovLH//透視投影??
	(
		orthoSize, orthoSize,TestSize_ , lightDistance * 2.5f);

	Renderer::SetLightMatrix(lightView, lightProjection);

	// デバッグ情報を表示
	ImGui::Text("Light Position: (%.1f, %.1f, %.1f)",
		-direction.x * lightDistance,
		-direction.y * lightDistance,
		-direction.z * lightDistance);

	// シャドウマップの可視化
	if (showShadowMap && enableShadow)
	{
		ID3D11ShaderResourceView* shadowSRV = Renderer::GetShadowMapSRV();
		if (shadowSRV)
		{
			ImGui::Text("Shadow Map:");
			ImGui::Image((void*)shadowSRV, ImVec2(256, 256));
		}
	}

	ImGui::End();
}

// デバッグフリーカメラ
void CarDriveScene::debugFreeCamera()
{
	//ImGui::Begin("debug Free camera");

	//static float radius = 100.0f;
	//static Vector3 pos = Vector3(0, 0, radius);
	//static Vector3 lookat = Vector3(0, 0, 0);
	//static float elevation = -90.0f * PI / 180.0f;
	//static float azimuth = PI/2.0f;

	//static Vector3 spherecenter = Vector3(0, 0, 0);	

	//ImGui::SliderFloat("Radius", &radius, 1,800);
	//ImGui::SliderFloat("Elevation", &elevation, -PI, PI);
	//ImGui::SliderFloat("Azimuth", &azimuth, -PI, PI);

	//ImGui::SliderFloat3("lookat ", &lookat.x, -100, 100);

	//// カメラの位置を極座標からデカルト座標に変換
	//m_FreeCamera.SetRadius(radius);
	//m_FreeCamera.SetElevation(elevation);
	//m_FreeCamera.SetAzimuth(azimuth);
	//m_FreeCamera.SetLookat(lookat);

	//// カメラの位置を極座標から求める
	//m_FreeCamera.CalcCameraPosition();

	//ImGui::End();
}

void CarDriveScene::debugChangeCamera()
{
	ImGui::Begin("Change Camera");
	if (ImGui::Button("SpringCamera")) {
		m_NowCamera = true;
	}
	if (ImGui::Button("FreeCamera")) {
		m_NowCamera = false;
	}
	ImGui::End();
}

void CarDriveScene::debugChangeScene()
{
	ImGui::Begin("Change Scene");
	if (ImGui::Button("On")) {
		m_Debug = true;
	}
	if (ImGui::Button("False")) {
		m_Debug = false;
	}
	ImGui::End();
}

void CarDriveScene::debugParticlePos()
{
	ImGui::Begin("Particle Position");

	ImGui::SliderFloat("Radius", &m_ParticlePos.x, -15, 15);
	ImGui::SliderFloat("Elevation", &m_ParticlePos.y, -15, 15);
	ImGui::SliderFloat("Azimuth", &m_ParticlePos.z, -15, 15);
	ImGui::Text("Particle Position X: %.2f", m_ParticlePos.x);
	ImGui::Text("Particle Position Y: %.2f", m_ParticlePos.y);
	ImGui::Text("Particle Position Z: %.2f", m_ParticlePos.z);
	ImGui::End();
}

void CarDriveScene::init()
{
	if (Renderer::GetDevice() == nullptr)
	{
		MessageBox(nullptr, "Renderer not initialized!", "Error", MB_OK);
		return;
	}

	// シャドウマップの初期化
	Renderer::InitShadowMap(2048);
	Renderer::EnableShadowMap(true);
	// スクリーン固定ビルボードの初期化
	m_screenBillboard = new ScreenFixedBillboard(Vector2(0.1f, 0.1f), 0.15f, 0.15f, L"assets/texture/haikei.jpg");
	EffectManager::Instance().Initialize();
	m_timeRenderer. Init(Vector2(0.95f, 0.15f),0.035f, 0.055f, 0.01f, true);
	m_scoreRenderer.Init(Vector2(0.95f, 0.3f), 0.035f, 0.055f, 0.01f, true);
	m_RemainingTime = 150.0f;
	//m_timeRenderer.Update();  //数値が変化したらUpdateを呼ぶ
	// 画面左上に配置（位置: 0.1, 0.1、サイズ: 0.15 x 0.15）
	//m_BillboardLoad->Init(Vector2(0.1f, 0.1f), 0.15f, 0.15f, L"assets/texture/haikei.jpg");

	m_speedMator = new SpeedMator();
	m_speedMator->Init();

	m_Gauge = new ScreenGaugeBillboard();
	// ゲージを初期化（画面左上に配置）
	bool success = m_Gauge->Init(
		Vector2(0.10f, 0.35f),  // 画面位置 (x=15%, y=10%)
		0.2f, 0.075f,           // サイズ (幅20%, 高さ5%)
		L"assets/texture/BackGround.png", // 外枠画像
		L"assets/texture/Bar.png",  // 内容画像（赤いバーなど）
		Vector2(0.02f, 0.01f)  // 内容の余白
	);

	m_Gauge->SetAnimationSpeed(1.5f); // アニメーション速度を設定

	//// ビルボードの初期化
	//m_billboard = new Billboard();
	//// 位置(x, y, z)、幅、高さ、テクスチャファイルパス
	//m_billboard->Init(Vector3(0.0f, 0.0f, 0.0f), 10.0f, 10.0f, L"assets/texture/space.png");

	// ローカル軸表示用線分の初期化
	m_segments[0] = std::make_unique<::Segment>(Vector3(0, 0, 0), Vector3(100, 0, 0));
	m_segments[1] = std::make_unique<::Segment>(Vector3(0, 0, 0), Vector3(0, 100, 0));
	m_segments[2] = std::make_unique<::Segment>(Vector3(0, 0, 0), Vector3(0, 0, 100));

	// フィールドの初期化
	m_field = std::make_unique<Field>();
	m_field->Init();

	// フィールドの初期化
	m_road = new Road;
	m_road->Init();

	// プレイヤの初期化
	m_player = std::make_unique<Player>();
	m_player->Init();
	//ここがあるとなぜか最初に車が動く
	m_player->SetRoadManager(&roadManager);
	m_player->SetRoad(m_road);

	// カメラの初期化とプレイヤーの設定
	// カメラの初期化とプレイヤーの設定
	SpringCamera::Instance().SetTargetPlayer(m_player.get());
	SpringCamera::Instance().Init();
	SimpleFollowCamera::Instance().SetTargetPlayer(m_player.get());
	SimpleFollowCamera::Instance().Init();	
	CheeseCamera::Instance().SetTargetPlayer(m_player.get());
	CheeseCamera::Instance().Init();
	IntroCamera::Instance().SetTargetPlayer(m_player.get());
	IntroCamera::Instance().Init();
	GoalCamera::Instance().SetTargetPlayer(m_player.get());
	GoalCamera::Instance().Init();
	m_introCamera = &IntroCamera::Instance();

	m_currentCamera = &IntroCamera::Instance();



	// スカイドームの初期化
	m_skydome = std::make_unique<Skydome>();
	m_skydome->Init();

	//InitEnemies(this, m_field.get());

	InitPostProcess(); // ポストプロセス初期化

	m_start = std::make_unique<Start>();	// スタート地点の初期化
	m_start->Init();	// スタート地点の初期化
	m_goal = std::make_unique<Goal>();	// スタート地点の初期化
	m_goal->Init();	// スタート地点の初期化

	m_item = std::make_unique<BoostItem>();	// スタート地点の初期化
	m_item->Init();	// スタート地点の初期化   ここまで




	if (auto start = roadManager.GetStart())
	{
		Vector3 startPos = start->GetPosition();
		m_player->SetPosition(startPos);
		m_player->StartRaceSequence(startPos);
		//敵配置
		SetupEnemiesOnRoad();
		SetupTreeOnRoad();
	}

	m_player->SetPostProcessSetter([this](bool use, float strength) {
		m_usePostProcess = use;
		m_aberrationStrength = strength;
		});

	m_CameraManager.SetTargetPlayer(m_player.get());
	m_CameraManager.Init();



	if (!m_sparkEmitter.Init(Renderer::GetDevice()))
	{
		OutputDebugStringA("サンプラーステート作成失敗\n");
	}
}

void CarDriveScene::loadAsync()
{
	roadManager.ResizeGrid(7, 18);//East=東　West＝西　North＝北　South＝南
	roadManager.InitializeGridSpacing();  // グリッド間隔を初期化
	roadManager.SetRoad(0, 1, RoadType::START_LINE, Direction::SOUTH);
	roadManager.SetRoad(0, 2, RoadType::STRAIGHT
		, Direction::NORTH);
	roadManager.SetRoad(0, 3, RoadType::STRAIGHT, Direction::NORTH);//北↑
	roadManager.SetRoad(0, 4, RoadType::SLOPE_UP, Direction::NORTH);
	roadManager.SetRoad(0, 5, RoadType::STRAIGHT, Direction::SOUTH);
	roadManager.SetRoad(0, 6, RoadType::STRAIGHT, Direction::SOUTH);
	roadManager.SetRoad(0, 7, RoadType::SLOPE_DOWN, Direction::NORTH);
	roadManager.SetRoad(0, 8, RoadType::STRAIGHT, Direction::NORTH);
	roadManager.SetRoad(0, 9, RoadType::STRAIGHT, Direction::NORTH);
	//Curveダート地帯
	roadManager.SetRoad(0, 10, RoadType::TURN_LEFT, Direction::NORTH);//東に向いてほしい
	roadManager.SetRoad(1, 10, RoadType::STRAIGHT, Direction::EAST);//東→
	roadManager.SetRoad(2, 10, RoadType::TURN_LEFT, Direction::SOUTH);//北に向いてほしい
	roadManager.SetRoad(2, 11, RoadType::STRAIGHT, Direction::NORTH);
	roadManager.SetRoad(2, 12, RoadType::STRAIGHT, Direction::NORTH);
	roadManager.SetRoad(2, 13, RoadType::TURN_LEFT, Direction::EAST);//
	roadManager.SetRoad(1, 13, RoadType::STRAIGHT, Direction::WEST);//←西
	roadManager.SetRoad(0, 11, RoadType::DIRT, Direction::NORTH);
	roadManager.SetRoad(0, 12, RoadType::DIRT, Direction::NORTH);
	roadManager.SetRoad(0, 13, RoadType::TURN_LEFT, Direction::WEST);
	//ここまで
	roadManager.SetRoad(0, 14, RoadType::STRAIGHT, Direction::NORTH);
	roadManager.SetRoad(1, 14, RoadType::DIRT, Direction::EAST);
	roadManager.SetRoad(2, 14, RoadType::DIRT, Direction::EAST);
	roadManager.SetRoad(3, 14, RoadType::DIRT, Direction::EAST);
	roadManager.SetRoad(4, 14, RoadType::DIRT, Direction::EAST);
	roadManager.SetRoad(0, 15, RoadType::STRAIGHT, Direction::NORTH);
	roadManager.SetRoad(0, 16, RoadType::STRAIGHT, Direction::NORTH);
	roadManager.SetRoad(0, 17, RoadType::TURN_LEFT, Direction::NORTH);
	roadManager.SetRoad(1, 17, RoadType::STRAIGHT, Direction::EAST);
	roadManager.SetRoad(2, 17, RoadType::STRAIGHT, Direction::EAST);
	roadManager.SetRoad(3, 17, RoadType::STRAIGHT, Direction::EAST);
	roadManager.SetRoad(4, 17, RoadType::STRAIGHT, Direction::EAST);
	roadManager.SetRoad(5, 17, RoadType::TURN_LEFT, Direction::EAST);
	roadManager.SetRoad(5, 16, RoadType::STRAIGHT, Direction::SOUTH);//南↓
	roadManager.SetRoad(5, 15, RoadType::STRAIGHT, Direction::SOUTH);
	roadManager.SetRoad(5, 14, RoadType::STRAIGHT, Direction::SOUTH);
	roadManager.SetRoad(5, 13, RoadType::TURN_LEFT, Direction::SOUTH);
	roadManager.SetRoad(4, 13, RoadType::TURN_LEFT, Direction::NORTH);
	roadManager.SetRoad(4, 12, RoadType::STRAIGHT, Direction::SOUTH);
	roadManager.SetRoad(4, 11, RoadType::TURN_LEFT, Direction::WEST);
	roadManager.SetRoad(5, 11, RoadType::TURN_LEFT, Direction::EAST);
	roadManager.SetRoad(5, 10, RoadType::STRAIGHT, Direction::SOUTH);

	roadManager.SetRoad(5, 9, RoadType::TURN_LEFT, Direction::WEST);
	roadManager.SetRoad(6, 9, RoadType::TURN_LEFT, Direction::EAST);
	roadManager.SetRoad(6, 8, RoadType::STRAIGHT, Direction::SOUTH);
	roadManager.SetRoad(6, 7, RoadType::TURN_LEFT, Direction::SOUTH);
	roadManager.SetRoad(5, 7, RoadType::TURN_LEFT, Direction::NORTH);
	roadManager.SetRoad(5, 6, RoadType::STRAIGHT, Direction::SOUTH);
	roadManager.SetRoad(5, 5, RoadType::SLOPE_UP, Direction::SOUTH);
	roadManager.SetRoad(5, 4, RoadType::SLOPE_UP, Direction::SOUTH);
	roadManager.SetRoad(5, 3, RoadType::SLOPE_UP, Direction::SOUTH);
	roadManager.SetRoad(5, 2, RoadType::SLOPE_UP, Direction::SOUTH);
	roadManager.SetRoad(5, 1, RoadType::SLOPE_UP, Direction::SOUTH);
	//roadManager.SetRoad(5, 0, RoadType::GOAL_LINE, Direction::NORTH);
	//roadManager.SetRoad(1, 14, RoadType::STRAIGHT, Direction::SOUTH);
	//roadManager.SetRoad(2, 4, RoadType::STRAIGHT, Direction::SOUTH);
	//roadManager.SetRoad(2, 3, RoadType::STRAIGHT, Direction::SOUTH);
	//roadManager.SetRoad(2, 2, RoadType::STRAIGHT, Direction::SOUTH);
	//roadManager.SetRoad(2, 1, RoadType::STRAIGHT, Direction::SOUTH);
	//roadManager.SetRoad(2, 0, RoadType::TURN_LEFT, Direction::SOUTH);
	//roadManager.SetRoad(1, 0, RoadType::GOAL_LINE, Direction::EAST);
	//roadManager.SetRoad(0, 0, RoadType::TURN_LEFT, Direction::WEST);



	////ながーーーーーいお付き合い
	//roadManager.SetRoad(0, 7, RoadType::STRAIGHT, Direction::SOUTH);
	//roadManager.SetRoad(0, 8, RoadType::STRAIGHT, Direction::SOUTH);
	//roadManager.SetRoad(0, 9, RoadType::STRAIGHT, Direction::SOUTH);
	//roadManager.SetRoad(0, 10, RoadType::STRAIGHT, Direction::SOUTH);
	//roadManager.SetRoad(0, 11, RoadType::STRAIGHT, Direction::SOUTH);
	//roadManager.SetRoad(0, 12, RoadType::STRAIGHT, Direction::SOUTH);
	//roadManager.SetRoad(0, 13, RoadType::STRAIGHT, Direction::SOUTH);
	//roadManager.SetRoad(0, 14, RoadType::STRAIGHT, Direction::SOUTH);
	//roadManager.SetRoad(0, 15, RoadType::STRAIGHT, Direction::SOUTH);
	//roadManager.SetRoad(0, 16, RoadType::STRAIGHT, Direction::SOUTH);
	//roadManager.SetRoad(0, 17, RoadType::GOAL_LINE, Direction::SOUTH);
}
void CarDriveScene::SetupEnemiesOnRoad()
{
	MultiFormationConfig config;
	config.totalEnemyCount = 200;  // 合計10体

	std::vector<BaseRoad*> straightRoads = roadManager.GetRoadByType(RoadType::STRAIGHT);
	for (auto& road : straightRoads)
	{
		if (!straightRoads.empty())
		{
			FormationConfig line;
			line.formation = EnemyFormation::DIAGONAL;
			line.enemyCount = 5;
			line.centerPos = road->GetPosition();  // 最初のストレート道路
			switch (road->GetDirection())//角度に応じて敵の配置位置を調整
			{
			case Direction::WEST:
				line.diagonalAngle = 90.0f;//←←←
				break;
			case Direction::SOUTH:
				line.diagonalAngle = -0.0f;//↓↓↓↓↓
				break;
			case Direction::EAST:
				line.diagonalAngle = -90.0f;//→→→
				break;
			case Direction::NORTH:
				line.diagonalAngle = 180.0f;//↑↑↑↑↑
				break;

			}
			line.spacing = 15.0f;

			config.AddFormation(line);
		}
	}
	std::vector<BaseRoad*> rightTurnRoads = roadManager.GetRoadByType(RoadType::TURN_LEFT);
	for (auto& road : rightTurnRoads)
	{
		if (!rightTurnRoads.empty())
		{
			FormationConfig right;
			right.formation = EnemyFormation::DIAGONAL;
			right.enemyCount = 5;
			Vector3 offset;
			switch (road->GetDirection())//角度に応じて敵の配置位置を調整
			{
			case Direction::NORTH:
				right.diagonalAngle = 45.0f;
				offset = Vector3(0.0f, 0.0f, -100.0f); // 適切なオフセット値を設定
				break;
			case Direction::EAST:
				right.diagonalAngle = 135.0f;
				offset = Vector3(-100.0f, 0.0f, 0.0f); // 適切なオフセット値を設定
				break;

			}
			right.centerPos = road->GetPosition() + offset;  // 最初のストレート道路
			config.AddFormation(right);
		}
	}

	//// 円形配置で3体
	//FormationConfig circle;
	//circle.formation = EnemyFormation::CIRCLE;
	//circle.enemyCount = 5;
	//circle.centerPos = startPos;
	//circle.circleRadius = 30.0f;
	//config.AddFormation(circle);

	//// ランダム配置で残り3体
	//FormationConfig random;
	//random.formation = EnemyFormation::RANDOM;
	//random.enemyCount = 3;
	//config.AddFormation(random);

	InitEnemiesWithMultiFormation(this, m_field.get(), config);
}	
void CarDriveScene::SetupTreeOnRoad()
{
	MultiTreeFormationConfig config;
	config.totalTreeCount = 200;  // 合計10体

	std::vector<BaseRoad*> straightRoads = roadManager.GetRoadByType(RoadType::STRAIGHT);
	for(auto& road:straightRoads)
	{ 
		if (!straightRoads.empty())
		{
			TreeFormationConfig treeconfig_right;
			TreeFormationConfig treeconfig_left;
			 treeconfig_right.treeCount = 4;
			 treeconfig_right.centerPos = road->GetPosition();  // 最初のストレート道路
			 treeconfig_right.randomizeScale = false;
			 treeconfig_left.treeCount = 3;
			 treeconfig_left.centerPos = road->GetPosition();  // 最初のストレート道路
			 treeconfig_left.randomizeScale = false;
			switch (road->GetDirection())//角度に応じて敵の配置位置を調整
			{
			case Direction::WEST:
				treeconfig_right.formation = TreeFormation::LINE_X;
				treeconfig_right.centerPos.x += (road->GetActualModelSize().x * road->GetScale().x) / 2;
				treeconfig_right.centerPos.z += (road->GetActualModelSize().z * road->GetScale().z) / 5;

				treeconfig_left.formation = TreeFormation::LINE_X;
				treeconfig_left.centerPos.x += (road->GetActualModelSize().x * road->GetScale().x) / 2;
				treeconfig_left.centerPos.z -= (road->GetActualModelSize().z * road->GetScale().z) / 5;
				break;
			case Direction::SOUTH:
				treeconfig_right.formation = TreeFormation::LINE;
				treeconfig_right.centerPos.x += (road->GetActualModelSize().x * road->GetScale().x) / 2;
				treeconfig_right.centerPos.z -= (road->GetActualModelSize().z * road->GetScale().z) / 2.5;

				treeconfig_left.formation = TreeFormation::LINE;
				treeconfig_left.centerPos.x -= (road->GetActualModelSize().x * road->GetScale().x) / 2;
				treeconfig_left.centerPos.z -= (road->GetActualModelSize().z * road->GetScale().z) / 2.5;
				break;
			case Direction::EAST:
				treeconfig_right.formation = TreeFormation::LINE_X;
				treeconfig_right.centerPos.x -= (road->GetActualModelSize().x * road->GetScale().x) / 2;
				treeconfig_right.centerPos.z -= (road->GetActualModelSize().z * road->GetScale().z) / 5;

				treeconfig_left.formation = TreeFormation::LINE_X;
				treeconfig_left.centerPos.x -= (road->GetActualModelSize().x * road->GetScale().x) / 2;
				treeconfig_left.centerPos.z += (road->GetActualModelSize().z * road->GetScale().z) / 5;
				break;
			case Direction::NORTH:
				treeconfig_right.formation = TreeFormation::LINE;
				treeconfig_right.centerPos.x -= (road->GetActualModelSize().x * road->GetScale().x) / 2;
				treeconfig_right.centerPos.z += (road->GetActualModelSize().z * road->GetScale().z) / 2.5;

				treeconfig_left.formation = TreeFormation::LINE;
				treeconfig_left.centerPos.x += (road->GetActualModelSize().x * road->GetScale().x) / 2;
				treeconfig_left.centerPos.z += (road->GetActualModelSize().z * road->GetScale().z) / 2.5;
				break;

			}
			treeconfig_right.spacing = 50.0f;
			treeconfig_left.spacing = 75.0f;

			config.AddFormation(treeconfig_right);
			config.AddFormation(treeconfig_left);
		}
	}

	m_TreeManager.Init(config);

	//スコアをリセット
	m_gameScore = 0;
}

float m_slowMotionStartTime = -1.0f;
float m_slowMotionDuration = 1.0f; // スロー演出の長さ（秒）
bool m_isInSlowMotion = false;
void CarDriveScene::update(float deltatime)//uint64_tとfloatの衝突　圧倒的衝突
{

	// キーで切り替え
	if (CDirectInput::GetInstance().CheckKeyBuffer(DIK_2)) {
		m_currentCamera = &SimpleFollowCamera::Instance();
	}
	if (CDirectInput::GetInstance().CheckKeyBuffer(DIK_3)) {
		m_currentCamera = &SpringCamera::Instance();
	}
	if (CDirectInput::GetInstance().CheckKeyBuffer(DIK_4)) {
		m_currentCamera = &CheeseCamera::Instance();
	}


	if (m_introCamera->IsIntroFinished()) {
		// SpringCameraのスプリングをIntroCameraの最終状態で初期化
		SpringCamera& springCam = SpringCamera::Instance();

		// 位置スプリングを現在の状態に設定
		Spring posSpring = springCam.GetPositionSpring();
		posSpring.position = m_introCamera->GetPosition();
		posSpring.velocity = Vector3(0, 0, 0);  // 速度はリセット
		springCam.SetPositionSpring(posSpring);

		// LookAtスプリングも同様
		Spring lookSpring = springCam.GetLookAtSpring();
		lookSpring.position = m_introCamera->GetLookat();
		lookSpring.velocity = Vector3(0, 0, 0);
		springCam.SetLookAtSpring(lookSpring);

		// カメラ切り替え
		m_currentCamera = &springCam;
		m_introCamera->ResetIntro();
	}

	//次は加速しましょう
	m_time += deltatime;

	if (CDirectInput::GetInstance().CheckKeyBuffer(DIK_P))
	{
		m_usePostProcess =true;
		m_aberrationStrength = 0.02f;
	}

       auto currentRoad = roadManager.GetGoalRoad();
     // 道路の端にいるかチェック（1.5f = 端から1.5単位以内）
     if (currentRoad) {
     	// 端の詳細を取得
     	EdgeType edgeType = currentRoad->GetPlayerEdgeType(m_player->GetPosition(), 15.0f);
     
     	switch (edgeType) {
     	case EdgeType::LEFT:
     		printf("左端！注意！\n");
     		// 左端の処理（警告音を鳴らす、速度を下げるなど）
     		break;
     
     	case EdgeType::RIGHT:
     		printf("右端！注意！\n");
     		break;
     
     	case EdgeType::CORNER:
     		printf("角！危険！\n");
			//SceneManager::ChangeScene("CarDriveScene", true);
     		// より強い減速
     		break;
        case EdgeType::BACK:
        	printf("後ろ\n");//Scene変更完了！
			if (!m_player->GetOnGoal())
			{
				m_player->OnGoal();
				m_gameScore += m_RemainingTime;
				m_currentCamera = &GoalCamera::Instance();

			}
        	// より強い減速
        	break;
        case EdgeType::FRONT:
        	printf("前\n");
        	// より強い減速
        	break;
          	}
	 }

	if (CDirectInput::GetInstance().CheckKeyBuffer(DIK_1))
	{
		//EffectManager::Instance().SpawnEffect("Flash", Vector3(-90.0f, 0.0f, 0.0f));
		DirectX::XMFLOAT3 pos_ = { 0.0f, 0.0f, 0.0f };
		pos_.x += 1.0f;
		pos_.y += 2.0f;
		EffectManager::Instance().SpawnEffect("SparkleParticle", pos_/*m_ParticlePos*/);//一旦諦めます！
	}

	if (CDirectInput::GetInstance().CheckKeyBuffer(DIK_M))
	{
		GameManager::Instance().SetTimeScale(1.0f); // スロー開始
	}
	if (m_usePostProcess)// ポストプロセス適用できた
    {
	
		float speed = m_player->GetSpeed();
		float maxSpeed = m_player->GetMaxSpeed();// 最高速度（少しずれがあるので、マジの最大速度を出したいならPlayerのMovmentの最後に固定値を入れる）
		float speedRatio = speed / maxSpeed;

		//printf("Speed: %.2f, Ratio: %.2f\n", speed, maxSpeed);
		// --- スロー開始判定 ---
		if (speedRatio > 1.0f && m_shockwaveTime < 0.0f)
		{
			m_shockwaveTime = m_time;
			m_shockwaveIntensity = 1.0f;

			// スローモーション開始（初回のみ）
			if (!m_isInSlowMotion)
			{
				m_isInSlowMotion = true;
				m_slowMotionStartTime = m_time;
				//GameManager::Instance().SetTimeScale(0.2f); // スローへ
			}
		}

		// --- スロー終了処理 ---
		if (m_isInSlowMotion)
		{
			float elapsedSlow = m_time - m_slowMotionStartTime;
			if (elapsedSlow >= m_slowMotionDuration)
			{
  				GameManager::Instance().SetTimeScale(1.0f); // 通常速度へ//まじで早いかわからん(周りに物をおいて確認)
				m_isInSlowMotion = false;
			}
		}

		// --- 衝撃波エフェクト ---
		if (m_shockwaveTime >= 0.0f)
		{
			float elapsed = m_time - m_shockwaveTime;
			float t = elapsed / m_shockwaveDuration;
			t = std::clamp(t, 0.0f, 1.0f);

			float val = std::max(0.0f, 1.0f - t);
			m_shockwaveIntensity = powf(val, 3.0f);
			m_shockwaveProgress = t;

			if (t >= 1.0f || speedRatio < 0.9f)
			{
				m_shockwaveTime = -1.0f;
				m_shockwaveIntensity = 0.0f;
				m_shockwaveProgress = 0.0f;
			}
		}

		// --- 通常のエフェクト更新 ---

		//ポストプロセスを設定
		m_enableMotionBlur =true;//ここ注意
		m_enableChromaticAberration = true;	
		m_blurStrength = std::clamp(speedRatio * 1.4f, 0.0f, 0.4f);
		m_aberrationStrength = std::clamp(speedRatio * 0.1f, 0.0f, 0.1f);
		m_LineSpeed = speedRatio * 0.05f;
		m_LineTime += deltatime;
    }


	if (GM31::GE::Collision::CollisionSphere(m_player->GetCollision(), m_item->GetCollision()))
	{
		m_item->PlayerBoostGauge(m_player.get());//アイテム取得
	}	

	m_player->Update(deltatime);	// プレイヤの更新
	m_speedMator->SetSpeed(m_player->GetSpeed());
	m_speedMator->Update(deltatime);
	// カメラの更新を先に行う


	m_currentCamera->Update(deltatime);

	m_screenBillboard->Update();

	m_road->Update((deltatime)); // ミリ秒に変換

	// ゲージを更新（deltaTimeを渡す）
	m_Gauge->Update(deltatime);

	m_RemainingTime -= deltatime;
	m_timeRenderer.SetNumber(static_cast<int>(m_RemainingTime));
	m_scoreRenderer.SetNumber(static_cast<int>(m_gameScore));

	m_timeRenderer.Update(deltatime);//吹っ飛ばした時のスコアってSceneを作ってフェードアウトさせたら終わり///////////////////
	m_scoreRenderer.Update(deltatime);

	// プレイヤーの体力に基づいてゲージ値を設定
	float boostGauge = m_player->GetBoostGauge();
	if (boostGauge > 1.0f) {
		boostGauge = boostGauge / 100.0f;  // 0-100の値なら正規化
	}
	m_Gauge->SetValue(boostGauge, true); // アニメーション有効
	m_item->Update(deltatime);
	roadManager.UpdateAll(deltatime); // 道路マネージャの描画
	bool onField = false;

	if (GM31::GE::Collision::CollisionSphereAABB(m_player->GetCollision(), m_field->GetFieldCollision()))
	{
		// フィールドに接触している場合
		Vector3 playerPos = m_player->GetPosition();
		float fieldTop = m_field->GetFieldCollision().max.y;

		// プレイヤーがフィールドの上にいるかチェック
		if (playerPos.y - m_player->GetCollision().radius <= fieldTop + 0.1f) {

			//// GameSceneのUpdate内で、位置を強制変更する前にプレイヤーの状態を保存
			//Vector3 originalVelocity = m_player->GetVelocity();
			//// 位置補正後
			//m_player->SetVelocity(originalVelocity); // 速度を復元
			// フィールドの上に配置
			playerPos.y = fieldTop + m_player->GetCollision().radius;
			m_player->SetPosition(playerPos);
			m_player->SetGrounded(true);
			onField = true;

			// 垂直速度をリセット
			m_player->ResetVerticalVelocity();
		}
	}
	UpdateEnemies(deltatime);
	// 道路の更新


    DirectX::XMFLOAT3 pos = m_player.get()->GetPosition();
    //pos.x += m_ParticlePos.x;x	
    pos.y -= 2.5f;
    //pos.z += m_ParticlePos.z;
    DirectX::XMFLOAT3 dir = DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f);
    m_sparkEmitter.Emit(pos, dir);
	m_sparkEmitter.Update(deltatime);
	EffectManager::Instance().Update(deltatime);
	m_skydome->Update(m_currentCamera->GetPosition());
}

void CarDriveScene::draw(float deltatime)
{
	ID3D11DeviceContext* context = Renderer::GetDeviceContext();

	//PostProcessが実際に必要か判定
	bool needsPostProcess = m_usePostProcess &&
		((m_enableMotionBlur && m_blurStrength > 0.01f) ||
			(m_enableChromaticAberration && m_aberrationStrength > 0.0f) ||
			(m_enableShockwave && m_shockwaveIntensity > 0.01f));

	// 【追加】まずデフォルトのレンダーターゲットと深度バッファを設定
	if (!needsPostProcess) {
		ID3D11RenderTargetView* backBuffer = nullptr;
		ID3D11Texture2D* backBufferTexture = nullptr;
		Renderer::GetSwapChain()->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&backBufferTexture);
		Renderer::GetDevice()->CreateRenderTargetView(backBufferTexture, nullptr, &backBuffer);
		backBufferTexture->Release();

		// 深度バッファも設定
		context->OMSetRenderTargets(1, &backBuffer, Renderer::GetDepthStencilView());
		backBuffer->Release();
	}

	if (needsPostProcess)
	{
		context->OMSetRenderTargets(1, &m_sceneRTV, m_sceneDSV);

		float clearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
		context->ClearRenderTargetView(m_sceneRTV, clearColor);
		context->ClearDepthStencilView(
			m_sceneDSV,
			D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,
			1.0f, 0
		);

		D3D11_VIEWPORT viewport = {};
		viewport.Width = (FLOAT)Application::GetWidth();
		viewport.Height = (FLOAT)Application::GetHeight();
		viewport.MinDepth = 0.0f;
		viewport.MaxDepth = 1.0f;
		context->RSSetViewports(1, &viewport);
	}

	// 【重要】3D描画前に必ず深度テストを有効化
	if (m_defaultDepthState) {
		context->OMSetDepthStencilState(m_defaultDepthState, 0);
	}

	// カメラ設定
	if (m_NowCamera) {
		m_currentCamera->Draw();
	}
	else {
		//m_FreeCamera.Draw();
	}

	//m_CameraManager.Draw();

	Matrix4x4 viewMatrix =Renderer::GetViewMatrix();

	Matrix4x4 worldMatrix = DirectX::XMMatrixIdentity();
	Renderer::SetWorldMatrix(&worldMatrix);

	//Color axiscol[3] = {
	//	Color(1, 0, 0, 1),
	//	Color(0, 1, 0, 1),
	//	Color(0, 1, 1, 1)
	//};

	//for (int axisno = 0; axisno < 3; axisno++)
	//{
	//	Matrix4x4 rotmtx = Matrix4x4::Identity;
	//	m_segments[axisno]->Draw(rotmtx, axiscol[axisno]);
	//}

	m_skydome->Draw(m_player->GetIsMaxSpeed());
	//m_road->Draw();
	m_goal->Draw();
	m_item->Draw();
	DrawEnemies();
	roadManager.DrawAll();
	m_TreeManager.Draw();
	//m_field->Draw();
	m_player->Draw();

	EffectManager::Instance().Draw(context,viewMatrix);
	m_sparkEmitter.Render(context, worldMatrix);


	if (needsPostProcess)//2D描画前にポストプロセス適用し、画像に影響を及ぼさないように変更
	{
		ApplyPostProcess();
	}

	// === 2D UI描画 ===
	// 【修正】prevDepthStateを保存せず、毎回作成
	ID3D11DepthStencilState* noDepthState = nullptr;
	D3D11_DEPTH_STENCIL_DESC depthDesc = {};
	depthDesc.DepthEnable = FALSE;
	depthDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	depthDesc.DepthFunc = D3D11_COMPARISON_ALWAYS;
	depthDesc.StencilEnable = FALSE;
	Renderer::GetDevice()->CreateDepthStencilState(&depthDesc, &noDepthState);

	if (noDepthState) {
		context->OMSetDepthStencilState(noDepthState, 0);
	}

	m_screenBillboard->Draw();
	m_speedMator->Draw();
	m_Gauge->Draw();
	m_timeRenderer.Draw(true);
	m_scoreRenderer.Draw(true,false);

	// 【重要】2D描画後、明示的に深度テストを再度有効化
	if (m_defaultDepthState) {
		context->OMSetDepthStencilState(m_defaultDepthState, 0);
	}

	if (noDepthState) {
		noDepthState->Release();
	}
}


void CarDriveScene::InitPostProcess()// ポストプロセス複数に適応させる
{

	// 画面サイズ取得
	int width = Application::GetWidth();
	int height = Application::GetHeight();

	// シーン描画用テクスチャ作成
	D3D11_TEXTURE2D_DESC textureDesc = {};
	textureDesc.Width = width;
	textureDesc.Height = height;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;

	Renderer::GetDevice()->CreateTexture2D(&textureDesc, nullptr, &m_sceneTexture);
	Renderer::GetDevice()->CreateRenderTargetView(m_sceneTexture, nullptr, &m_sceneRTV);
	Renderer::GetDevice()->CreateShaderResourceView(m_sceneTexture, nullptr, &m_sceneSRV);



	// 【重要】元のバックバッファを取得して保存
	ID3D11Texture2D* backBufferTexture = nullptr;
	Renderer::GetSwapChain()->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&backBufferTexture);
	Renderer::GetDevice()->CreateRenderTargetView(backBufferTexture, nullptr, &m_originalBackBuffer);
	backBufferTexture->Release();

	D3D11_TEXTURE2D_DESC depthDesc = {};
	depthDesc.Width = width;
	depthDesc.Height = height;
	depthDesc.MipLevels = 1;
	depthDesc.ArraySize = 1;
	depthDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthDesc.SampleDesc.Count = 1;
	depthDesc.SampleDesc.Quality = 0;
	depthDesc.Usage = D3D11_USAGE_DEFAULT;
	depthDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthDesc.CPUAccessFlags = 0;
	depthDesc.MiscFlags = 0;

	HRESULT hr = Renderer::GetDevice()->CreateTexture2D(&depthDesc, nullptr, &m_sceneDepthTexture);
	if (FAILED(hr)) {
		printf("ERROR: Failed to create depth texture! HRESULT = 0x%08X\n", hr);
		return;
	}

	hr = Renderer::GetDevice()->CreateDepthStencilView(m_sceneDepthTexture, nullptr, &m_sceneDSV);
	if (FAILED(hr)) {
		printf("ERROR: Failed to create depth stencil view! HRESULT = 0x%08X\n", hr);
		return;
	}

	D3D11_DEPTH_STENCIL_DESC depthDesc_ = {};
	depthDesc_.DepthEnable = TRUE;
	depthDesc_.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	depthDesc_.DepthFunc = D3D11_COMPARISON_LESS;
	depthDesc_.StencilEnable = FALSE;

	 hr = Renderer::GetDevice()->CreateDepthStencilState(&depthDesc_, &m_defaultDepthState);
	if (FAILED(hr)) {
		printf("ERROR: Failed to create default depth state\n");
	}
	else {
		printf("SUCCESS: Default depth state created\n");
	}

	// サンプラー作成
	D3D11_SAMPLER_DESC samplerDesc = {};
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	hr = Renderer::GetDevice()->CreateSamplerState(&samplerDesc, &m_sampler);
	if (FAILED(hr)) {
		printf("ERROR: CreateSamplerState failed!\n");
	}

	// ポストプロセス用シェーダ読み込み
	m_chromaticAberrationShader.Create(
		"shader/chromatic_aberrationVS.hlsl",
		"shader/chromatic_aberrationPS.hlsl"
	);

		// ポストプロセス用シェーダ読み込み
		m_motionBlurShader.Create(
			"shader/chromatic_aberrationVS.hlsl",
			"shader/motion_blurPS.hlsl"
	);
	if (FAILED(hr)) {
		printf("ERROR: Shader compilation failed!\n");
	}

	CreateIntermediateTexture();
}
void CarDriveScene::CreateIntermediateTexture()
{
	int width = Application::GetWidth();
	int height = Application::GetHeight();

	D3D11_TEXTURE2D_DESC textureDesc = {};
	textureDesc.Width = width;
	textureDesc.Height = height;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	textureDesc.SampleDesc.Count = 1;

	Renderer::GetDevice()->CreateTexture2D(&textureDesc, nullptr, &m_intermediateTexture);
	Renderer::GetDevice()->CreateRenderTargetView(m_intermediateTexture, nullptr, &m_intermediateRTV);
	Renderer::GetDevice()->CreateShaderResourceView(m_intermediateTexture, nullptr, &m_intermediateSRV);
}
void CarDriveScene::ApplyPostProcess()
{
	ID3D11DeviceContext* context = Renderer::GetDeviceContext();

	// ステート保存
	ID3D11DepthStencilState* originalDepthState = nullptr;
	UINT originalStencilRef = 0;
	context->OMGetDepthStencilState(&originalDepthState, &originalStencilRef);

	ID3D11RasterizerState* originalRasterState = nullptr;
	context->RSGetState(&originalRasterState);

	// 深度テスト無効化
	ID3D11DepthStencilState* depthState = nullptr;
	D3D11_DEPTH_STENCIL_DESC depthStencilDesc = {};
	depthStencilDesc.DepthEnable = FALSE;
	Renderer::GetDevice()->CreateDepthStencilState(&depthStencilDesc, &depthState);
	context->OMSetDepthStencilState(depthState, 0);

	// ラスタライザ設定
	ID3D11RasterizerState* rasterState = nullptr;
	D3D11_RASTERIZER_DESC rasterDesc = {};
	rasterDesc.FillMode = D3D11_FILL_SOLID;
	rasterDesc.CullMode = D3D11_CULL_NONE;
	Renderer::GetDevice()->CreateRasterizerState(&rasterDesc, &rasterState);
	context->RSSetState(rasterState);

	ID3D11ShaderResourceView* currentInput = m_sceneSRV;
	ID3D11RenderTargetView* currentOutput = nullptr;

	//どのエフェクトが有効か事前に判定
	bool hasMotionBlur = m_enableMotionBlur && m_blurStrength > 0.01f;
	bool hasChromatic = m_enableChromaticAberration && m_aberrationStrength > 0.001f;
	bool hasShockwave = m_enableShockwave && m_shockwaveIntensity > 0.01f;

	//パス1: Motion Blur
	if (hasMotionBlur)
	{
		// 次のパスがあるか確認
		bool isLastPass = !hasChromatic && !hasShockwave;

		// 最後のパスならバックバッファへ、そうでなければ中間テクスチャへ
		currentOutput = isLastPass ? m_originalBackBuffer : m_intermediateRTV;

		ApplyMotionBlur(context, currentInput, currentOutput);

		// 次のパスがあれば、出力を次の入力にする
		if (!isLastPass)
		{
			currentInput = m_intermediateSRV;
		}
	}

	//パス2: Chromatic Aberration
	if (hasChromatic)
	{
		// 次のパスがあるか確認
		bool isLastPass = !hasShockwave;

		currentOutput = isLastPass ? m_originalBackBuffer : m_intermediateRTV;

		ApplyChromaticAberration(context, currentInput, currentOutput);

		if (!isLastPass)
		{
			currentInput = m_intermediateSRV;
		}
	}

	// ステート復元
	context->OMSetDepthStencilState(originalDepthState, originalStencilRef);//え？同時にできるんですか？
	context->RSSetState(originalRasterState);

	if (depthState) depthState->Release();
	if (rasterState) rasterState->Release();
	if (originalDepthState) originalDepthState->Release();
	if (originalRasterState) originalRasterState->Release();


}

void CarDriveScene::ApplyMotionBlur(ID3D11DeviceContext* context,
	ID3D11ShaderResourceView* input, ID3D11RenderTargetView* output)
{
	context->OMSetRenderTargets(1, &output, nullptr);

	float clearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
	context->ClearRenderTargetView(output, clearColor);

	//【変更】定数バッファを拡張（放射状ブラー対応）
	struct PostProcessBuffer {
		float blurStrength;        // 追加
		float aberrationStrength;
		float centerX;             // 追加
		float centerY;             // 追加
		float time; //追加
		float speedLineSpeed; //追加（スクロール速度）
		float shockwaveIntensity;
		float shockwaveProgress;
	};
	PostProcessBuffer cb;
	cb.blurStrength = m_blurStrength;              // 追加
	cb.aberrationStrength = m_aberrationStrength;
	cb.centerX = 0.5f;                             // 追加：画面中心
	cb.centerY = 0.5f;                             // 追加
	cb.time = m_LineTime;//追加
	cb.speedLineSpeed = m_LineSpeed;//追加
	cb.shockwaveIntensity = m_shockwaveIntensity; // 高速時ほど強く
	cb.shockwaveProgress = m_shockwaveProgress; // 高速時ほど強く

	ID3D11Buffer* constantBuffer = nullptr;
	D3D11_BUFFER_DESC bufferDesc = {};
	bufferDesc.ByteWidth = sizeof(PostProcessBuffer);
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

	D3D11_SUBRESOURCE_DATA initData = {};
	initData.pSysMem = &cb;

	Renderer::GetDevice()->CreateBuffer(&bufferDesc, &initData, &constantBuffer);

	m_motionBlurShader.SetGPU();
	context->PSSetConstantBuffers(0, 1, &constantBuffer);
	context->PSSetShaderResources(0, 1, &input);
	context->PSSetSamplers(0, 1, &m_sampler);

	context->IASetInputLayout(nullptr);
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	context->Draw(4, 0);

	if (constantBuffer) constantBuffer->Release();
}

void CarDriveScene::ApplyChromaticAberration(ID3D11DeviceContext* context,
	ID3D11ShaderResourceView* input, ID3D11RenderTargetView* output)
{
	context->OMSetRenderTargets(1, &output, nullptr);

	float clearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
	context->ClearRenderTargetView(output, clearColor);

	struct ChromaticBuffer {
		float aberrationStrength;
		float time;
		float padding1;
		float padding2;
	};

	ChromaticBuffer cb;
	cb.aberrationStrength = m_aberrationStrength;
	cb.time = m_LineTime;

	ID3D11Buffer* constantBuffer = nullptr;
	D3D11_BUFFER_DESC bufferDesc = {};
	bufferDesc.ByteWidth = sizeof(ChromaticBuffer);
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

	D3D11_SUBRESOURCE_DATA initData = {};
	initData.pSysMem = &cb;

	Renderer::GetDevice()->CreateBuffer(&bufferDesc, &initData, &constantBuffer);

	m_chromaticAberrationShader.SetGPU();
	context->PSSetConstantBuffers(0, 1, &constantBuffer);
	context->PSSetShaderResources(0, 1, &input);
	context->PSSetSamplers(0, 1, &m_sampler);

	context->Draw(4, 0);

	if (constantBuffer) constantBuffer->Release();
}

void CarDriveScene::dispose()
{
	// ポストプロセス用リソースの解放
	if (m_defaultDepthState) {
		m_defaultDepthState->Release();
		m_defaultDepthState = nullptr;
	}
	if (m_sceneDepthTexture) {
		m_sceneDepthTexture->Release();
		m_sceneDepthTexture = nullptr;
	}
	if (m_sceneTexture) {
		m_sceneTexture->Release();
		m_sceneTexture = nullptr;
	}
	if (m_sceneRTV) {
		m_sceneRTV->Release();
		m_sceneRTV = nullptr;
	}
	if (m_sceneSRV) {
		m_sceneSRV->Release();
		m_sceneSRV = nullptr;
	}
	if (m_sampler) {
		m_sampler->Release();
		m_sampler = nullptr;
	}

	// その他のリソース解放
	if (m_screenBillboard) {
		delete m_screenBillboard;
		m_screenBillboard = nullptr;
	}
	if (m_speedMator) {
		delete m_speedMator;
		m_speedMator = nullptr;
	}
	if (m_Gauge) {
		delete m_Gauge;
		m_Gauge = nullptr;
	}
	if (m_road) {
		delete m_road;
		m_road = nullptr;
	}
	roadManager.DisposeAll();

	EffectManager::Instance().Finalize();
	TreeManager::DisposeSharedResources();
}
