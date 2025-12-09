#include "system/commontypes.h"
#include "system/CStaticMesh.h"
#include "system/CStaticMeshRenderer.h"
#include "system/CDirectInput.h"
#include "system/DebugUI.h"
#include "Player.h"
#include"Enemies.h"
#include <iostream>
#include "ChaseCamera.h"
#include "EffeectManager.h"
#include "SpringCamera.h"



float VALUE_MOVE_MODEL = 2.0f;						// キー入力時の移動量
float VALUE_ROTATE_MODEL = PI * 0.02f;				// キー入力時の回転量
float RATE_ROTATE_MODEL = 0.40f;					// １フレーム当たりの回転割合
float RATE_MOVE_MODEL = 0.20f;						// １フレーム当たりの減衰割合

// debug用
static Vector3 g_rotation = Vector3(0.0f, 0.0f, 0.0f);
static Vector3 g_position = Vector3(0.0f, 0.0f, 0.0f);
static Vector3 g_scale = Vector3(1.0f, 1.0f, 1.0f);

static void DebugPlayerMoveParameter() {

	ImGui::Begin("Debug Player Move Parameter");

	ImGui::SliderFloat("VALUE_MOVE_MODEL", &VALUE_MOVE_MODEL, 0.01f, 3.0f);
	ImGui::SliderFloat("VALUE_ROTATE_MODEL", &VALUE_ROTATE_MODEL, 0.01f, PI / 4.0f);
	ImGui::SliderFloat("RATE_ROTATE_MODEL", &RATE_ROTATE_MODEL, 0.0f, 1.0f);
	ImGui::SliderFloat("RATE_MOVE_MODEL", &RATE_MOVE_MODEL, 0.0f, 1.0f);

	ImGui::Text("ROTATION %f %f %f", g_rotation.x, g_rotation.y, g_rotation.z);
	ImGui::Text("POSITION %f %f %f", g_position.x, g_position.y, g_position.z);
	ImGui::Text("SCALE %f %f %f", g_scale.x, g_scale.y, g_scale.z);

	ImGui::End();
}

void  Player::DebugPlayerMoveParameter_() {

	ImGui::Begin("eeeeeeeeeeeeeeeeeeeeeeeeeeeeeee");

	ImGui::SliderFloat("YYYYYYY", &testparticle_y, -100.1f, 105.0f);
	ImGui::SliderFloat("XXXXXXXX", &testparticle_x, 0-100.1f, 105.0f);
	ImGui::SliderFloat("ZZZZZZZZZZZZZZ", &testparticle_z, 0-100.1f, 105.0f);
	ImGui::End();
}

void Player::Init()
{
	// モデルの初期化
	m_mesh.Load(
		"assets/model/Cow_Rocket_Adventure_1112064202_texture.fbx",// モデル名
		"assets/model/");						// テクスチャのパス

	//レンダラ初期化
	m_meshrenderer.Init(m_mesh);

	// ★自動計算: BottomYが負の場合は補正が必要★
	float bottomY = m_mesh.GetBottomY();

	if (bottomY < 0.0f) {
		// 負の値の場合、その分だけ持ち上げる
		m_groundOffset = -bottomY; // 例: -(-0.716) = 0.716
	}
	else {
		m_groundOffset = 0.0f;
	}

	// シェーダーの初期化
	m_shader.Create(
		"shader/vertexLightingVS.hlsl",				// 頂点シェーダー
		"shader/vertexLightingPS.hlsl");			// ピクセルシェーダー

	// 通常描画用シェーダー（影あり）- 新規
	m_shadowShader.Create(
		"shader/vertexLightingShadowVS.hlsl",
		"shader/vertexLightingShadowPS.hlsl");

	// シャドウマップ生成用シェーダー - 新規
	m_shadowMapShader.Create(
		"shader/ShadowMapVS.hlsl",
		"shader/ShadowMapPS.hlsl");


	DebugUI::RedistDebugFunction(DebugPlayerMoveParameter);

	//m_physics.SetPosition(m_Position);
	//m_physics.SetRotation(m_Rotation);
	DebugUI::RedistDebugFunction([this]() {
		m_physics.Init();
		});

	DebugUI::RedistDebugFunction([this]()
		{
			DebugPlayerMoveParameter_();
		});
	//DebugUI::RedistDebugFunction([this]() {
	//	DebugTerrainRotation_();
	//	});

	if (!m_sparkEmitter.Init(Renderer::GetDevice()))
	{
		OutputDebugStringA("サンプラーステート作成失敗\n");
	}
	m_Scale = Vector3(10.0f, 10.0f, 10.0f);
}
//まだちょいがたつく
void Player::Update(float deltatime)
{
	if (m_HitStopTimer > 0.0f)//ヒットストップがなんかしっくりこない
	{
		m_HitStopTimer -= deltatime;

		if (m_HitStopTimer <= 0.0f)
		{
			// 段階的に速度を戻す（スムーズ）
			float currentScale = GameManager::Instance().GetTimeScale();
			if (currentScale < 1.0f)
			{
				float newScale = currentScale + deltatime * 10.0f; // 10倍速で戻す
				if (newScale >= 1.0f)
				{
					newScale = 1.0f;
				}
				GameManager::Instance().SetTimeScale(newScale);
			}


			m_HitStopTimer = 0.0f;
		}
	}

	float timeScale = GameManager::Instance().GetTimeScale();
    float scaledDeltaTime = deltatime * timeScale;  // ★ここで一度だけ計算

	float throttle = 0.0f;
	float steering = 0.0f;
	// ブースト入力チェック
	bool boostInput = CDirectInput::GetInstance().CheckKeyBuffer(DIK_SPACE);

	// ブースト状態の更新
	UpdateBoostSystem(boostInput, deltatime);

	// ★★★ ハイブリッド速度システムの更新 ★★★
	UpdateSpeedBonusSystem(deltatime);

	if (CDirectInput::GetInstance().CheckKeyBuffer(DIK_O))
	{
		AddBoostGauge(5.0f);
	}

	// 前進・後退の入力
	if (CDirectInput::GetInstance().CheckKeyBuffer(DIK_W)) {
		throttle = 1.0f;
	}
	else if (CDirectInput::GetInstance().CheckKeyBuffer(DIK_S)) {
		throttle = -0.5f; // 後退は前進より遅く
	}

	// ステアリング入力
	if (CDirectInput::GetInstance().CheckKeyBuffer(DIK_A)) {
		steering = -1.0f; // 左
	}
	else if (CDirectInput::GetInstance().CheckKeyBuffer(DIK_D)) {
		steering = 1.0f;  // 右
	}


	// ドリフト入力チェック
	bool driftInput = false;
	if (CDirectInput::GetInstance().CheckKeyBuffer(DIK_Q)) {
		driftInput = true;
		m_DriftDirection = -1.0f; // 左ドリフト
	}
	else if (CDirectInput::GetInstance().CheckKeyBuffer(DIK_E)) {
		driftInput = true;
		m_DriftDirection = 1.0f;  // 右ドリフト
	}

	// 車の向きベクトルを計算
	Vector3 forwardDir = Vector3(sinf(m_Rotation.y), 0.0f, cosf(m_Rotation.y));
	Vector3 rightDir = Vector3(cosf(m_Rotation.y), 0.0f, -sinf(m_Rotation.y));

	float currentSpeed = sqrt(m_Velocity.x * m_Velocity.x + m_Velocity.z * m_Velocity.z);
	float speedFactor = std::min(currentSpeed / m_MaxSpeed, 1.0f);

	// ドリフト状態の管理
	if (driftInput && currentSpeed > 0.5f) {
		m_IsDrifting = true;
	}
	else if (!driftInput || currentSpeed < 0.2f) {
		m_IsDrifting = false;
	}

	// 前フレームの位置を保存
	m_previousPosition = m_Position;

	// 重力を適用（地面に接触していない場合）
	ApplyGravity(deltatime );

	// ドリフト時とノーマル時で異なる処理
	if (m_IsDrifting)
	{
		// ドリフト中の処理
		UpdateDriftMovement(throttle, steering, forwardDir, rightDir, speedFactor);
	}
	else {
		// 通常の処理
		UpdateNormalMovement(throttle, steering, forwardDir, rightDir, speedFactor);
	}

	// ★修正：高速移動時の安全な位置更新★
	UpdatePositionWithCollisionCheck(timeScale);


	float modelHeight = m_mesh.GetHeight();  // Max.y - Min.y
	float bottomOffsetY = m_mesh.GetBottomY();

	// オプション1: 底面からの高さで計算
	m_BoundingSphere =
	{
		Vector3(
			m_Position.x,
			m_Position.y + (bottomOffsetY * m_Scale.y) + (m_BoundingSphere.radius * 0.5f),
			m_Position.z
		),
		0.5f,
	};

	// 滑らかな地形追従処理
	UpdateSmoothTerrainFollowing(deltatime);//道心折れたので明日がエンジンゲージを作りましょう

	// リセット
	if (CDirectInput::GetInstance().CheckKeyBuffer(DIK_RETURN)) {
		m_Position = Vector3(0.0f, 0.0f, 0.0f);
		m_Rotation = Vector3(0.0f, 0.0f, 0.0f);
		m_Velocity = Vector3(0.0f, 0.0f, 0.0f);
		m_IsDrifting = false;
		m_IsBoosting = false; // ブースト状態もリセット

		// ★★★ ハイブリッド速度システムもリセット ★★★
		m_PermanentSpeedBonus = 1.0f;
		m_TemporarySpeedBonus = 1.0f;
		m_SpeedBoostTimer = 0.0f;
		m_ConsecutiveHits = 0;
	}

	// 敵との当たり判定
	for (auto& enemy : GetAllEnemys()) {
		if (CollisionSphere(m_BoundingSphere, enemy->GetEnemyBoundingSphere())) {
			if (!enemy->m_IsKnockedBack) {

				OnCollisionWithEnemy(*enemy);
				AddBoostGauge(5.0f);//敵を吹っ飛ばした際に加速ゲージを取得　加速の最大値も少し上昇
			}
		}
	}

	m_sparkEmitter.Update(scaledDeltaTime);
}
// ブーストシステムの更新処理
void Player::UpdateBoostSystem(bool boostInput, float deltaSeconds)
{
	if (boostInput && m_BoostGauge > 0.0f)

	{
		//m_PostProcessSetter(true, 0.02f);
		// ブースト使用中
		m_IsBoosting = true;
		m_BoostGauge -= m_BoostConsumption * deltaSeconds;
		// ゲージが0以下になったら停止
		if (m_BoostGauge <= 0.0f) {
			m_BoostGauge = 0.0f;
			m_IsBoosting = false;
		}
	}
	else {
		// ブースト未使用
		m_IsBoosting = false;
		m_PostProcessSetter(false, 0.0f);
	}
}

void Player::UpdateSpeedBonusSystem(float deltatime)
{
	// 一時ボーナスのタイマー更新
	if (m_SpeedBoostTimer > 0.0f)
	{
		m_SpeedBoostTimer -= deltatime;

		if (m_SpeedBoostTimer <= 0.0f)
		{
			m_SpeedBoostTimer = 0.0f;
		}
	}

	// 一時ボーナスの減衰（タイマーが切れた後）
	if (m_SpeedBoostTimer <= 0.0f && m_TemporarySpeedBonus > 1.0f)
	{
		// 緩やかに1.0に戻す
		m_TemporarySpeedBonus = Lerp(m_TemporarySpeedBonus, 1.0f, m_TemporaryBonusDecaySpeed);

		// 1.0に十分近づいたら完全に1.0にする
		if (abs(m_TemporarySpeedBonus - 1.0f) < 0.01f)
		{
			m_TemporarySpeedBonus = 1.0f;
		}
	}

	// 速度に応じたポストプロセス効果（常時適用）
	float currentMultiplier = GetCurrentSpeedMultiplier();
	if (currentMultiplier > 1.5f)
	{
		// 速度倍率が1.5倍以上の時はモーションブラーを強化
		float blurIntensity = std::min((currentMultiplier - 1.0f) * 0.02f, 0.05f);
		m_PostProcessSetter(true, blurIntensity);
	}
	else if (!m_IsBoosting)
	{
		// ブーストもしていない、速度倍率も低い場合は無効化
		m_PostProcessSetter(false, 0.0f);
	}
}


// ★★★ 新規メソッド: 現在の速度倍率を取得 ★★★
float Player::GetCurrentSpeedMultiplier() const
{
	return m_PermanentSpeedBonus * m_TemporarySpeedBonus;
}

void Player::UpdateSmoothTerrainFollowing(uint64_t deltatime)
{
	bool onRoad = false;

	if (m_roadManager) {
		float terrainHeight;
		Vector3 terrainNormal;

		bool terrainFound = m_roadManager->GetTerrainHeight(m_Position, terrainHeight, terrainNormal);

		if (terrainFound) {
			float bottomOffsetY = m_mesh.GetBottomY();
			float carBottomY = m_Position.y + (bottomOffsetY * m_Scale.y);
			float heightDifference = carBottomY - terrainHeight;

			float horizontalSpeed = sqrt(m_Velocity.x * m_Velocity.x + m_Velocity.z * m_Velocity.z);
			bool isHighSpeed = m_IsBoosting && horizontalSpeed > 2.0f;

			float contactThreshold = isHighSpeed ? 1.5f : 2.0f;

			if (heightDifference <= contactThreshold) {
				onRoad = true;
				m_isGrounded = true;

				m_targetHeight = terrainHeight - (bottomOffsetY * m_Scale.y);

				float currentHeight = m_Position.y;
				float heightDiff = abs(m_targetHeight - currentHeight);

				Vector3 horizontalVelocity = Vector3(m_Velocity.x, 0.0f, m_Velocity.z);
				float horizontalSpeedForLerp = horizontalVelocity.Length();
				bool isNearlyStationary = horizontalSpeedForLerp < 0.05f;

				// ★★★ 下り坂判定の追加 ★★★
				bool isMovingDownhill = false;
				if (horizontalSpeedForLerp > 0.1f) {
					Vector3 movementDir = horizontalVelocity;
					movementDir.Normalize();

					// 地形の下り方向を計算
					Vector3 gravityDir(0, -1, 0);
					Vector3 slopeDir = gravityDir - terrainNormal * gravityDir.Dot(terrainNormal);
					slopeDir.Normalize();

					float slopeDot = movementDir.Dot(slopeDir);
					isMovingDownhill = (slopeDot > 0.1f); // 下り坂を移動中
				}

				// 高速移動時の高さ補正
				if (isHighSpeed) {
					if (heightDiff > 1.5f) {
						// 即座に修正
						m_Position.y = m_targetHeight;

						// ★★★ 垂直速度を強制的にリセット ★★★
						m_verticalVelocity = 0.0f;

						printf("High-speed terrain correction: %.3f\n", heightDiff);
					}
					else {
						// より積極的な補間
						float lerpSpeed = isMovingDownhill ? 0.5f : 0.3f; // 下り坂ではより強く
						m_Position.y = Lerp(currentHeight, m_targetHeight, lerpSpeed);

						// ★★★ 下り坂では垂直速度を減衰 ★★★
						if (isMovingDownhill && m_verticalVelocity < 0.0f) {
							m_verticalVelocity *= m_downhillVelocityDamping;
						}
					}
				}
				else {
					// 通常速度時の処理
					if (heightDiff > 3.0f) {
						m_Position.y = m_targetHeight;
						m_verticalVelocity = 0.0f; // ★追加: リセット
					}
					else {
						float lerpSpeed = isNearlyStationary ? 0.25f : 0.1f;
						if (heightDiff < 0.05f) lerpSpeed = 0.5f;
						m_Position.y = Lerp(currentHeight, m_targetHeight, lerpSpeed);
					}
				}

				// ★★★ 地面接触時は垂直速度をリセット ★★★
				m_verticalVelocity = 0.0f;

				// 地形法線の更新
				if (terrainNormal.y > 0.1f) {
					float normalLerpSpeed = 0.08f;
					m_terrainNormal = Lerp3(m_terrainNormal, terrainNormal, normalLerpSpeed);
					m_terrainNormal.Normalize();
				}

				RoadType currentRoadType;
				if (m_roadManager->GetRoadSurfaceType(m_Position, currentRoadType))
				{
					ApplyRoadSurfaceEffect(currentRoadType, deltatime);
				}

				// 坂道処理
				if (IsOnSlope() && horizontalSpeedForLerp > 0.01f)
				{
					Vector3 gravityDirection = Vector3(0, -1, 0);
					Vector3 slopeDirection = gravityDirection - m_terrainNormal * gravityDirection.Dot(m_terrainNormal);
					slopeDirection.Normalize();

					float slopeInfluence = (1.0f - m_terrainNormal.y) * (isHighSpeed ? 0.05f : 0.1f);

					Vector3 movementDirection = horizontalVelocity;
					movementDirection.Normalize();

					float slopeDot = movementDirection.Dot(slopeDirection);
					m_slopeDot = slopeDot;

					Vector3 up(0, 1, 0);
					float angleFromVertical = acosf(m_terrainNormal.Dot(up));
					m_slopeAngle = angleFromVertical;

					bool hasThrottleInput = (CDirectInput::GetInstance().CheckKeyBuffer(DIK_W) ||
						CDirectInput::GetInstance().CheckKeyBuffer(DIK_S));

					if (slopeDot > 0 && hasThrottleInput)
					{
						// 下り坂 - 何もしない（バウンド防止のため加速を控える）
					}
					else {
						// 上り坂抵抗
						float resistanceFactor = 1.0f;
						m_Velocity *= resistanceFactor;
					}
				}
			}
		}

		if (onRoad) {
			UpdateCarRotationFromTerrain(m_terrainNormal);
		}
	}

	// 道路に接触していない場合
	if (!onRoad) {
		m_isGrounded = false;
		Vector3 horizontalNormal = Vector3(0.0f, 1.0f, 0.0f);
		m_terrainNormal = Lerp3(m_terrainNormal, horizontalNormal, 0.02f);
		m_terrainNormal.Normalize();
		UpdateCarRotationFromTerrain(m_terrainNormal);
	}

	// ★★★ 前フレームの状態を保存 ★★★
	m_wasGroundedLastFrame = m_isGrounded;
	m_previousTerrainNormal = m_terrainNormal;
}

float Player::GetGroundSlope() const
{
	if (!IsOnSlope()) {
		return 0.0f;  // 平地
	}

	// slopeDotの値に応じて角度を返す
	// slopeDot > 0: 下り坂（カメラは上から見下ろす = 正の値）
	// slopeDot < 0: 上り坂（カメラは下から見上げる = 負の値）

	// 画像から: 下り 0.99, 登り 0.98
	// これはほぼ同じ値なので、slopeAngleも使って判定

	const float threshold = 0.0f;

	if (m_slopeDot > threshold) {
		// 下り坂: 負の角度を返す（カメラ側で下り坂判定）
		return -m_slopeAngle;
	}
	else  {
		// 上り坂: 正の角度を返す（カメラ側で上り坂判定）
		return m_slopeAngle;
	}

}
bool Player::IsOnSlope() const
{
	return m_terrainNormal.y < m_slopeThreshold;
}

// Vector3の線形補間関数
// 重力を適用するメソッド
void Player::ApplyGravity(float deltatime)
{
	if (!m_isGrounded) 
	{
		// 重力を垂直速度に加算
		float deltaSeconds = deltatime/* / 1000000.0f*/; // マイクロ秒を秒に変換
		m_verticalVelocity += m_gravity * (deltaSeconds*0.1f) * 60.0f; // 60FPS基準で調整

		// 落下速度の制限（ターミナル速度）
		const float maxFallSpeed = -10.0f;
		if (m_verticalVelocity < maxFallSpeed) {
			m_verticalVelocity = maxFallSpeed;
		}
	}
}

void Player::ApplyRoadSurfaceEffect(RoadType surfaceType, float deltatime)
{
	float timeScale = GameManager::Instance().GetTimeScale();

	switch (surfaceType) {
	case RoadType::DIRT:
	{
		// ダートでは摩擦が大きく、速度が低下
		float dirtFriction = 0.96f;  // 通常より強い減速（0.98fが通常）
		m_Velocity.x *= pow(dirtFriction, timeScale);
		m_Velocity.z *= pow(dirtFriction, timeScale);

		// 最大速度も制限
		float dirtMaxSpeedRatio = 0.75f;  // 通常の75%の速度
		float dirtMaxSpeed = m_MaxSpeed * dirtMaxSpeedRatio;

		if (m_IsBoosting) {
			dirtMaxSpeed *= m_BoostRatio;  // ブースト時も制限
		}

		float currentSpeed = sqrt(m_Velocity.x * m_Velocity.x +
			m_Velocity.z * m_Velocity.z);
		if (currentSpeed > dirtMaxSpeed) {
			float speedRatio = dirtMaxSpeed / currentSpeed;
			m_Velocity.x *= speedRatio;
			m_Velocity.z *= speedRatio;
		}

		// デバッグ表示（オプション）
		 printf("On DIRT road - Speed limited to %.2f\n", currentSpeed);
		break;
	}

	case RoadType::STRAIGHT:
	case RoadType::TURN_LEFT:
	case RoadType::TURN_RIGHT:
	case RoadType::SLOPE_UP:
	case RoadType::SLOPE_DOWN:
	case RoadType::START_LINE:
	case RoadType::GOAL_LINE:
	default:
		// 通常の道路では何もしない
		break;
	}
}

void Player::UpdateNormalMovement(float throttle, float steering, Vector3 forwardDir, Vector3 rightDir, float speedFactor)
{
	float timeScale = GameManager::Instance().GetTimeScale();

	m_Rotation.y += steering * m_TurnSpeed * speedFactor * timeScale;

	if (m_Rotation.y > PI) m_Rotation.y -= 2.0f * PI;
	if (m_Rotation.y < -PI) m_Rotation.y += 2.0f * PI;

	float accelerationMultiplier = m_IsBoosting ? m_BoostPower : 1.0f;
	Vector3 forceDir = forwardDir * throttle * m_Acceleration * accelerationMultiplier * timeScale;
	m_Velocity += forceDir;

	speed = sqrt(m_Velocity.x * m_Velocity.x + m_Velocity.z * m_Velocity.z);
	Vector3 velocityDir = Vector3(0.0f, 0.0f, 0.0f);
	if (speed > 0.01f) {
		velocityDir = m_Velocity * (1.0f / speed);
	}
	float alignment = forwardDir.x * velocityDir.x + forwardDir.z * velocityDir.z;
	Vector3 targetVelocity = forwardDir * speed * alignment;

	float adjustedGrip = 1.0f - pow(1.0f - m_GripFactor, timeScale);
	m_Velocity = m_Velocity * (1.0f - adjustedGrip) + targetVelocity * adjustedGrip;

	float decelerationMultiplier = m_IsBoosting ? 0.998f : m_Deceleration;
	m_Velocity *= pow(decelerationMultiplier, timeScale);

	// ★★★ 速度制限にハイブリッド速度倍率を適用 ★★★
	float speedMultiplier = GetCurrentSpeedMultiplier();
	float maxSpeed = m_MaxSpeed * speedMultiplier;

	if (m_IsBoosting)
	{
		maxSpeed *= m_BoostRatio;
	}

	speed = sqrt(m_Velocity.x * m_Velocity.x + m_Velocity.z * m_Velocity.z);
	if (speed > maxSpeed) {
		m_Velocity = m_Velocity * (maxSpeed / speed);
	}

	Vector3 backmovement = Vector3(-velocityDir.x, 0.0f, velocityDir.z);
	Vector3 dustPos = m_Position;
	dustPos.x += 1.0f;
	dustPos.y += 2.0f;

	m_sparkEmitter.Emit(dustPos, Vector3(0.0f, 1.0f, 0.0f));
	//float timeScale = GameManager::Instance().GetTimeScale();
	//// 通常のステアリング（速度に応じて回転量を調整）
	//m_Rotation.y += steering * m_TurnSpeed * speedFactor * timeScale;
	//// 角度の正規化
	//if (m_Rotation.y > PI) m_Rotation.y -= 2.0f * PI;
	//if (m_Rotation.y < -PI) m_Rotation.y += 2.0f * PI;

	//// ブースト時の加速力調整
	//float accelerationMultiplier = m_IsBoosting ? m_BoostPower : 1.0f;
	//// 前進方向の力を加える
	//Vector3 forceDir = forwardDir * throttle * m_Acceleration * accelerationMultiplier * timeScale;
	//m_Velocity += forceDir;

	//// 通常のグリップ効果を適用
	//speed = sqrt(m_Velocity.x * m_Velocity.x + m_Velocity.z * m_Velocity.z);
	//Vector3 velocityDir = Vector3(0.0f, 0.0f, 0.0f);
	//if (speed > 0.01f) {
	//	velocityDir = m_Velocity * (1.0f / speed); // 正規化
	//}
	//float alignment = forwardDir.x * velocityDir.x + forwardDir.z * velocityDir.z;
	//Vector3 targetVelocity = forwardDir * speed * alignment;
	//// グリップの補間（タイムスケールに応じて調整）
	//float adjustedGrip = 1.0f - pow(1.0f - m_GripFactor, timeScale);
	//m_Velocity = m_Velocity * (1.0f - adjustedGrip) + targetVelocity * adjustedGrip;

	//// 減速処理（ブースト時は減速を軽減）
	//float decelerationMultiplier = m_IsBoosting ? 0.998f : m_Deceleration;
	//m_Velocity *= pow(decelerationMultiplier, timeScale);

	//// ★速度制限を最後に移動★
	//float maxSpeed = m_IsBoosting ? m_MaxSpeed * m_BoostRatio : m_MaxSpeed;
	//speed = sqrt(m_Velocity.x * m_Velocity.x + m_Velocity.z * m_Velocity.z);
	//if (speed > maxSpeed) {
	//	m_Velocity = m_Velocity * (maxSpeed / speed);
	//	//最大速度を固定値にするならここで
	//	//speed = maxSpeed;  // ★制限後の速度を反映★
	//}

	//Vector3 backmovement = Vector3(-velocityDir.x, 0.0f, velocityDir.z);
	//Vector3 dustPos = m_Position;
	//dustPos.x += 1.0f;
	//dustPos.y += 2.0f;

	////printf("MangerEmit: m_ParticlePos=(%f, %f, %f)\n", dustPos.x, dustPos.y, dustPos.z);

	//// プリセットから砂煙を生成
	//m_sparkEmitter.Emit(dustPos, Vector3(0.0f, 1.0f, 0.0f));
}

// ドリフト移動処理にもブーストを適用
void Player::UpdateDriftMovement(float throttle, float steering, Vector3 forwardDir, Vector3 rightDir, float speedFactor)
{
	float timeScaleDrift = GameManager::Instance().GetTimeScale();

	float driftRotation = m_DriftDirection * m_DriftTurnSpeed * speedFactor;
	m_Rotation.y += driftRotation;

	m_Rotation.y += steering * m_TurnSpeed * speedFactor * 0.5f * timeScaleDrift;

	if (m_Rotation.y > PI) m_Rotation.y -= 2.0f * PI;
	if (m_Rotation.y < -PI) m_Rotation.y += 2.0f * PI;

	float accelerationMultiplier = m_IsBoosting ? m_BoostPower : 1.0f;

	Vector3 forceDir = forwardDir * throttle * m_Acceleration * 0.8f * accelerationMultiplier * timeScaleDrift;
	m_Velocity += forceDir;

	Vector3 lateralForce = rightDir * m_DriftDirection * 0.02f * speedFactor;
	m_Velocity += lateralForce;

	// ★★★ 速度制限にハイブリッド速度倍率を適用 ★★★
	float speedMultiplier = GetCurrentSpeedMultiplier();
	float maxSpeed = m_MaxSpeed * speedMultiplier;

	if (m_IsBoosting)
	{
		maxSpeed *= 1.2f;  // ドリフト中のブースト時はさらに1.2倍
	}

	speed = sqrt(m_Velocity.x * m_Velocity.x + m_Velocity.z * m_Velocity.z);
	if (speed > maxSpeed) {
		m_Velocity = m_Velocity * (maxSpeed / speed);
	}

	Vector3 velocityDir = Vector3(0.0f, 0.0f, 0.0f);
	if (speed > 0.01f) {
		velocityDir = m_Velocity * (1.0f / speed);
	}

	float alignment = forwardDir.x * velocityDir.x + forwardDir.z * velocityDir.z;
	Vector3 targetVelocity = forwardDir * speed * alignment;

	float adjustedGrip = 1.0f - pow(1.0f - m_GripFactor, timeScaleDrift);
	m_Velocity = m_Velocity * (1.0f - adjustedGrip) + targetVelocity * adjustedGrip;

	float decelerationMultiplier = m_IsBoosting ? 0.985f : (m_Deceleration * 0.98f);
	m_Velocity *= pow(decelerationMultiplier, timeScaleDrift);
}


void Player::Draw()
{
	// SRT情報作成
	SRT srt;
	srt.pos = m_Position;
	srt.rot = m_Rotation;
	srt.scale = m_Scale;
	Matrix4x4 worldmtx;
	worldmtx = srt.GetMatrix();
	Matrix rotationMatrix = Matrix::CreateRotationY(DirectX::XMConvertToRadians(90.0f)); // または -90.0f
	worldmtx = rotationMatrix * worldmtx;
	Renderer::SetWorldMatrix(&worldmtx);

	Matrix4x4 worldMatrix = DirectX::XMMatrixIdentity();

	//m_sparkEmitter.Render(Renderer::GetDeviceContext(), worldMatrix);
	auto mode = Renderer::GetRenderMode();

	if (mode == Renderer::RenderMode::SHADOW_MAP)
	{
		OutputDebugStringA("\n=== Player::Draw SHADOW_MAP mode ===\n");

		// シャドウマップ生成用シェーダー
		m_shadowMapShader.SetGPU();

		// シャドウマップパスではメッシュレンダラーを使わず、直接描画する必要があるかも
		// まずは通常通り呼んでみる
		m_meshrenderer.Draw();
	}
	else
	{
		OutputDebugStringA("\n=== Player::Draw NORMAL mode ===\n");

		// 通常描画
		if (Renderer::IsShadowMapEnabled())
		{
			m_shadowShader.SetGPU();
		}
		else
		{
			m_shader.SetGPU();
		}

		m_meshrenderer.Draw();
	}

	// デバッグ用
	g_position = m_Position;
	g_rotation = m_Rotation;
	g_scale = m_Scale;

	Color bscolor(1, 1, 1, 0.5f);
	SphereDrawerDraw(m_BoundingSphere.radius, bscolor, m_BoundingSphere.center.x, m_BoundingSphere.center.y, m_BoundingSphere.center.z);
	if (m_roadManager) {
		float terrainHeight;
		Vector3 terrainNormal;
		if (m_roadManager->GetTerrainHeight(m_Position, terrainHeight, terrainNormal)) {
			Color terrainColor(1, 0, 0, 0.8f); // 赤色
			SphereDrawerDraw(0.3f, terrainColor, m_Position.x, terrainHeight, m_Position.z);
		}
	}
	//m_sparkEmitter.Render(Renderer::GetDeviceContext(), viewmtx);//ここのViewMatrixが違う説を提唱します
}

void Player::Dispose()
{

}

// 敵との衝突時の処理
void Player::OnCollisionWithEnemy(Enemy& enemy)
{
	float timeScale = GameManager::Instance().GetTimeScale();
	// Player から 敵 への方向ベクトル（水平方向のみ）
	Vector3 knockbackDirection = enemy.GetPosition() - this->GetPosition();
	knockbackDirection.y = 0.0f;  // Y軸成分を削除

	// 方向ベクトルが0でない場合のみ処理
	if (knockbackDirection.Length() > 0.001f) {
		knockbackDirection.Normalize();	
	}
	else {
		// プレイヤーと敵が完全に同じ位置にいる場合は前方に飛ばす
		knockbackDirection = Vector3(sinf(this->GetRotation().y), 0.0f, cosf(this->GetRotation().y));
	}


	// カメラの前方向を取得(水平方向のみ)
	Vector3 cameraForward = SpringCamera::Instance().GetForward();
	cameraForward.y = 0.0f;
	cameraForward.Normalize();

	// カメラ方向に寄せる
	float cameraInfluence = 0.5f; // 0.0f(元の方向) ～ 1.0f(完全にカメラ方向)
	knockbackDirection = knockbackDirection * (1.0f - cameraInfluence) +
		cameraForward * cameraInfluence;
	knockbackDirection.y = 0.1f; // Y方向を復元
	knockbackDirection.Normalize();


	// ノックバック力を適用
	float knockbackForce = speed*50.0f; // 力を強くして確実に飛ぶようにする
	m_gameScore += knockbackForce;

	if (timeScale != 1.0f)
	{
		knockbackForce *=(1 / timeScale);//スローモーションでも同じ力で吹っ飛ぶように
	}

	// ★★★ ハイブリッド速度システムの適用 ★★★
	m_ConsecutiveHits++;

	// 【永続ボーナス】マイルストーン達成チェック
	if (m_ConsecutiveHits % m_HitsPerMilestone == 0 &&
		m_PermanentSpeedBonus < m_MaxPermanentBonus)
	{
		m_PermanentSpeedBonus += m_PermanentBonusPerMilestone;

		// 最大値を超えないように制限
		if (m_PermanentSpeedBonus > m_MaxPermanentBonus)
		{
			m_PermanentSpeedBonus = m_MaxPermanentBonus;
		}

		// マイルストーン達成時の特別な演出
		ApplyHitStop(0.025f, 0.005f);  // 通常より長めのヒットストップ
		SpringCamera::Instance().Shake(3.0f, 0.2f);  // 強めのカメラシェイク

		// ここでUI表示などを追加可能
		// 例: "SPEED LEVEL UP!" みたいな表示
		printf("★ MILESTONE! Speed Level: x%.2f (Hits: %d)\n",
			m_PermanentSpeedBonus, m_ConsecutiveHits);
	}

	// 【一時ボーナス】敵を倒すたびに加算
	m_TemporarySpeedBonus += m_TemporaryBonusPerHit;
	if (m_TemporarySpeedBonus > m_MaxTemporaryBonus)
	{
		m_TemporarySpeedBonus = m_MaxTemporaryBonus;
	}

	// 一時ボーナスのタイマーをリセット
	m_SpeedBoostTimer = m_TemporaryBoostDuration;

	// 現在の速度に応じてヒットストップと演出を調整
	float speedRatio = speed / m_MaxSpeed;
	if (speedRatio > 1.5f)
	{
		// 高速時はより派手な演出
		ApplyHitStop(0.015f, 0.005f);
		SpringCamera::Instance().Shake(2.0f, 0.15f);
	}
	else
	{
		// 通常の演出
		ApplyHitStop(0.01f, 0.0001f);
		SpringCamera::Instance().Shake(1.0f, 0.1f);
	}

	//敵を飛ばしたときに起きる効果
	//ApplyHitStop(0.01f, 0.01f);
	SpringCamera::Instance().Shake(1.0f, 0.1f);

	enemy.ApplyKnockback(knockbackDirection, knockbackForce,timeScale);
}

void Player::ApplyHitStop(float duration, float timeScale /*= 0.0f*/)//	スローどうしよ　ピンとこんなー
{
	m_HitStopTimer = duration;
	m_PreviousTimeScale = GameManager::Instance().GetTimeScale();//連続で吹っ飛ばすとバグるので廃止
	//GameManager::Instance().SetTimeScale(timeScale);
}



void Player::AddBoostGauge(float amount)
{
	m_BoostGauge += amount;
	if (m_BoostGauge > m_MaxBoostGauge)
	{
		m_BoostGauge = m_MaxBoostGauge;
	}

	// デバッグ用の出力（必要に応じてコメントアウト可能）
	//printf("Boost Gauge increased by %.1f. Current: %.1f/%.1f\n",
	//	amount, m_BoostGauge, m_MaxBoostGauge);
}
void Player::UpdatePositionWithCollisionCheck(float timeScale)
{
	// 現在の速度を取得
	float horizontalSpeed = sqrt(m_Velocity.x * m_Velocity.x + m_Velocity.z * m_Velocity.z);
	// 高速移動判定（ブースト使用時かつ一定速度以上の場合のみ）
	bool isHighSpeed = m_IsBoosting && horizontalSpeed > 2.0f;

	// 変更: m_road から m_roadManager に変更
	if (isHighSpeed && m_roadManager)
	{
		// 高速移動時は段階的に位置を更新（ただし高さ調整は最小限に）
		int steps = std::max(3, (int)(horizontalSpeed / 1.0f)); // 速度に応じてステップ数を動的調整
		steps = std::min(steps, 6); // 最大6ステップに制限（処理負荷考慮）
		Vector3 stepVelocity = m_Velocity * (timeScale / steps);

		for (int i = 0; i < steps; i++) {
			Vector3 nextPosition = m_Position;
			nextPosition.x += stepVelocity.x;
			nextPosition.z += stepVelocity.z;

			// 各ステップで突き抜けチェックのみ実行（細かい高さ調整はしない）
			float terrainHeight;
			Vector3 terrainNormal;

			// 変更: RoadManager経由で地形高度を取得
			bool terrainFound = m_roadManager->GetTerrainHeight(nextPosition, terrainHeight, terrainNormal);

			if (terrainFound)
			{
				// 地形の高さと比較して、大きな突き抜けのみチェック
				float carBottom = nextPosition.y - m_BoundingSphere.radius;
				float heightDifference = carBottom - terrainHeight;

				// 大きく地形を下回った場合のみ緊急修正
				if (heightDifference < -3.0f) {
					nextPosition.y = terrainHeight + m_BoundingSphere.radius + 1.0f; // 少し余裕を持たせる
					m_verticalVelocity = 0.0f;
					m_isGrounded = true;
					printf("Emergency collision correction at step %d\n", i);
				}
			}

			// 水平位置のみ更新（垂直位置は後の地形追従に任せる）
			m_Position.x = nextPosition.x;
			m_Position.z = nextPosition.z;
		}

		// 垂直方向の位置更新（重力のみ）
		if (!m_isGrounded) {
			m_Position.y += m_verticalVelocity;
		}
	}
	else {
		// 通常速度時は従来通り
		m_Position.x += m_Velocity.x*timeScale;
		m_Position.z += m_Velocity.z*timeScale;

		// 垂直方向の位置更新（重力のみ、地形追従は後で処理）
		if (!m_isGrounded) {
			m_Position.y += m_verticalVelocity;
		}
	}
}
void Player::UpdateCarRotationFromTerrain(const Vector3& terrainNormal)
{ // 地形がほぼ水平な場合は回転をリセット
	if (terrainNormal.y > 0.99f) { // さらに厳しく設定
		// 徐々に水平に戻す（非常に緩やかに）
		m_Rotation.x = Lerp(m_Rotation.x, 0.0f, m_terrainTiltLerpSpeed * 0.1f);
		m_Rotation.z = Lerp(m_Rotation.z, 0.0f, m_terrainTiltLerpSpeed * 0.1f);
		return;
	}

	// 車の向きベクトル（Y軸回転のみから計算）
	float carYaw = m_Rotation.y;
	Vector3 carForward = Vector3(sinf(carYaw), 0.0f, cosf(carYaw));
	Vector3 carRight = Vector3(cosf(carYaw), 0.0f, -sinf(carYaw));

	// 地形上での車の方向ベクトルを計算
	Vector3 projectedForward = carForward - terrainNormal * carForward.Dot(terrainNormal);
	if (projectedForward.Length() > 0.01f) {
		projectedForward.Normalize();
	}
	else {
		projectedForward = carForward;
	}

	Vector3 projectedRight = carRight - terrainNormal * carRight.Dot(terrainNormal);
	if (projectedRight.Length() > 0.01f) {
		projectedRight.Normalize();
	}
	else {
		projectedRight = carRight;
	}

	// ピッチ角度（前後の傾斜）を計算
	float pitch = asinf(projectedForward.y);

	// ロール角度（左右の傾斜）を計算
	float roll = asinf(projectedRight.y);

	// 角度制限（極端な傾斜を防ぐ）
	const float MAX_TILT = 10.0f * (3.14159f / 180.0f); // 25度から20度に制限を強化
	pitch = std::max(-MAX_TILT, std::min(MAX_TILT, pitch));
	roll = std::max(-MAX_TILT, std::min(MAX_TILT, roll));

	// 補間速度の調整
	Vector3 horizontalVelocity = Vector3(m_Velocity.x, 0.0f, m_Velocity.z);
	float horizontalSpeed = horizontalVelocity.Length();
	bool isNearlyStationary = horizontalSpeed < 0.05f;

	float lerpSpeed = m_terrainTiltLerpSpeed * 0.3f; // 全体的に遅く

	if (isNearlyStationary) {
		// 静止時
		lerpSpeed *= 0.5f;
	}
	else {
		// 移動時
		if (IsOnSlope()) {
			lerpSpeed *= 0.3f; // 坂で非常に慎重に
		}
		else {
			lerpSpeed *= 0.5f;
		}
	}


	// 地面に接触している場合のみ回転を適用
	if (m_isGrounded) {
		// 現在の角度との差をチェック
		float pitchDiff = abs(m_Rotation.x - pitch);
		float rollDiff = abs(m_Rotation.z - roll);

		// 角度の変化が大きすぎる場合は更に緩やかに
		if (pitchDiff > 0.2f || rollDiff > 0.2f) {
			lerpSpeed *= 0.2f; // 非常に緩やか
		}
		else if (pitchDiff < 0.02f && rollDiff < 0.02f) {
			lerpSpeed *= 2.0f; // 小さな差の場合のみ少し速く
		}

		m_Rotation.x = Lerp(m_Rotation.x, pitch, lerpSpeed);
		m_Rotation.z = Lerp(m_Rotation.z, roll, lerpSpeed);
	}
	}

