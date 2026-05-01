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
#include"GoalCamera.h"
#include "TreeManager.h"

// 定数定義を関数外に移動（再計算を防ぐ）
namespace PlayerConstants {
	constexpr float VALUE_MOVE_MODEL = 2.0f;
	constexpr float VALUE_ROTATE_MODEL = PI * 0.02f;
	constexpr float RATE_ROTATE_MODEL = 0.40f;
	constexpr float RATE_MOVE_MODEL = 0.20f;
	constexpr float TWO_PI = PI * 2.0f;
	constexpr float MAX_FALL_SPEED = -10.0f;
	constexpr float SPIRAL_MIN_DURATION = 2.0f;
	constexpr float SPIRAL_MAX_DURATION = 6.0f;
}

// debug用
static Vector3 g_rotation = Vector3(0.0f, 0.0f, 0.0f);
static Vector3 g_position = Vector3(0.0f, 0.0f, 0.0f);
static Vector3 g_scale = Vector3(1.0f, 1.0f, 1.0f);



//最初の演出
void Player::StartRaceSequence(const Vector3& startPosition)
{
	// 全状態をクリアしてスパイラル降下状態に
	m_stateManager.ClearAllStates();
	m_stateManager.AddState(PlayerStateManager::State::SpiralDescending);

	m_spiralTime = 0.0f;
	m_spiralInitialYaw = m_Rotation.y;

	// 開始位置を設定
	m_spiralTargetPos = startPosition;
	m_spiralStartPos = startPosition;
	m_spiralStartPos.y += m_spiralHeight;

	// 実際の地形高度を取得
	if (m_roadManager) {
		float terrainHeight;
		Vector3 terrainNormal;
		if (m_roadManager->GetTerrainHeight(startPosition, terrainHeight, terrainNormal)) {
			float bottomOffsetY = m_mesh.GetBottomY();
			m_spiralTargetPos.y = terrainHeight - (bottomOffsetY * m_Scale.y);
			m_spiralStartPos.y = m_spiralTargetPos.y + m_spiralHeight;
		}
	}

	// 螺旋の長さから最適な時間を計算（sqrt呼び出しを1回に削減）
	const float verticalDistance = m_spiralHeight;
	const float spiralCircumference = PlayerConstants::TWO_PI * m_spiralRadius;
	const float horizontalDistance = spiralCircumference * m_spiralRotations;
	const float totalDistance = sqrtf(verticalDistance * verticalDistance +
		horizontalDistance * horizontalDistance);

	m_spiralDuration = totalDistance / m_spiralDesiredSpeed;
	m_spiralDuration = std::clamp(m_spiralDuration,
		PlayerConstants::SPIRAL_MIN_DURATION,
		PlayerConstants::SPIRAL_MAX_DURATION);

	// プレイヤーを開始位置に配置
	m_Position = m_spiralStartPos;
	m_velocity = Vector3::Zero; // ゼロベクトル定数使用
	m_Rotation.x = 0.0f;
	m_Rotation.z = 0.0f;

	// ブースト関連もリセット
	m_isBoosting = false;
	m_isDrifting = false;
	m_boostGauge = 0.0f;
}


void Player::UpdateStartSequence(float deltatime)//最初の降下演出
{
	// スパイラル降下中
	if (m_stateManager.IsSpiralDescending())
	{
		m_spiralTime += deltatime;

		// 進行度を計算（0.0 ～ 1.0）
		float progress = m_spiralTime / m_spiralDuration;

		//地面との衝突チェック
		bool shouldLand = false;

		if (progress >= 1.0f)
		{
			// 時間経過で着地
			shouldLand = true;
			progress = 1.0f;
		}
		else if (m_roadManager)
		{
			// 地形との距離をチェック
			float terrainHeight;
			Vector3 terrainNormal;
			if (m_roadManager->GetTerrainHeight(m_Position, terrainHeight, terrainNormal))
			{
				float bottomOffsetY = m_mesh.GetBottomY();
				float carBottomY = m_Position.y + (bottomOffsetY * m_Scale.y);
				float distanceToGround = carBottomY - terrainHeight;

				// 地面に接触または非常に近い場合は着地
				if (distanceToGround <= 0.5f)
				{
					shouldLand = true;
				}
			}
		}

		if (shouldLand)
		{
			// 螺旋降下終了、カウントダウンへ移行
			m_Position = m_spiralTargetPos;

			m_stateManager.RemoveState(PlayerStateManager::State::SpiralDescending);
			m_stateManager.AddState(PlayerStateManager::State::Countdown);
			m_stateManager.AddState(PlayerStateManager::State::OnGround);

			m_countdownTime = 0.0f;
			m_countdownNumber = 3;

			// 着地時は現在の回転をそのまま使う（補間済み）
			// すでに徐々に正面を向いているので、強制的に変更しない
			m_Rotation.x = 0.0f;  // ピッチだけ水平に
			m_Rotation.y = m_spiralInitialYaw;
			m_Rotation.z = 0.0f;  // ロールだけ水平に
		}
		else
		{
			//有機的な動きのための複数のイージング

			// 1. 高さ: 最初ゆっくり、途中加速、最後減速（S字カーブ）
			float heightEase;
			if (progress < 0.5f) {
				// 前半: ゆっくり降下開始
				float t = progress * 2.0f;
				heightEase = t * t * (3.0f - 2.0f * t); // Smoothstep
			}
			else {
				// 後半: 着地に向けて減速
				float t = (progress - 0.5f) * 2.0f;
				heightEase = 0.5f + 0.5f * (1.0f - pow(1.0f - t, 3.0f)); // EaseOutCubic
			}
			float height = Lerp(m_spiralStartPos.y, m_spiralTargetPos.y, heightEase);

			// 2. 回転速度: 途中で加速して最後は減速（生物的な動き）
			float rotationEase = progress + sinf(progress * PI) * 0.15f; // 波打つ
			float angle = rotationEase * PI * 2.0f * m_spiralRotations;

			// 3. 半径: 波打ちながら縮小（ふわふわ感）
			float radiusBase = 1.0f - progress * 0.7f; // 基本的な縮小
			float radiusWave = sinf(progress * PI * m_spiralRotations * 2.0f) * m_spiralWaveIntensity;
			float radiusPulse = sinf(progress * PI * 6.0f) * m_spiralPulseIntensity * (1.0f - progress);
			float currentRadius = m_spiralRadius * (radiusBase + radiusWave + radiusPulse);

			// 4. 上下の揺れ（ふわふわ感の強化）
			float verticalWave = sinf(m_spiralTime * 4.0f) * m_spiralVerticalWave * (1.0f - progress * 0.7f);
			float verticalBounce = cosf(progress * PI * 3.0f) * 0.5f * (1.0f - progress);

			// 5. 螺旋軌道の位置を計算
			float offsetX = cosf(angle) * currentRadius;
			float offsetZ = sinf(angle) * currentRadius;

			m_Position.x = m_spiralTargetPos.x + offsetX;
			m_Position.y = height + verticalWave + verticalBounce;
			m_Position.z = m_spiralTargetPos.z + offsetZ;

			// 6. 向きの計算（滑らかな方向転換）
			// 少し先の位置を見て自然な向きを作る
			float lookAheadAngle = angle + 0.2f;
			float lookAheadRadius = currentRadius * 0.95f;
			float lookX = m_spiralTargetPos.x + cosf(lookAheadAngle) * lookAheadRadius;
			float lookZ = m_spiralTargetPos.z + sinf(lookAheadAngle) * lookAheadRadius;

			Vector3 lookDirection = Vector3(lookX - m_Position.x, 0.0f, lookZ - m_Position.z);
			float targetYaw = atan2f(lookDirection.x, lookDirection.z);

			// 序盤から徐々に正面方向への補正を開始
			if (progress > 0.01f) {
				// 残り70%で初期方向に戻す
				float returnProgress = (progress - 0.01f) / 0.99f; // 0.0 ~ 1.0

				// イージング: 最初はゆっくり、最後は強く補正
				float returnStrength;
				if (returnProgress < 0.5f) {
					// 前半: 緩やかに（二次関数）
					returnStrength = 2.0f * returnProgress * returnProgress;
				}
				else {
					// 後半: 加速して補正（三次関数）
					float t = returnProgress - 0.5f;
					returnStrength = 0.5f + 2.0f * t * t;
				}

				// 螺旋の接線方向と初期方向をブレンド
				targetYaw = Lerp(targetYaw, m_spiralInitialYaw, returnStrength);
			}

			// 目標方向への滑らかな回転
			float yawDiff = targetYaw - m_Rotation.y;
			while (yawDiff > PI) yawDiff -= 2.0f * PI;
			while (yawDiff < -PI) yawDiff += 2.0f * PI;

			// 着地が近いほど回転速度を上げる（最後はしっかり正面を向く）
			float rotationSpeed = 0.15f;
			if (progress > 0.3f) {
				// 補正フェーズでは回転速度を徐々に上げる
				float speedBoost = (progress - 0.3f) / 0.7f;
				rotationSpeed = Lerp(0.15f, 0.35f, speedBoost);
			}

			m_Rotation.y += yawDiff * rotationSpeed;

			// 7. ピッチ角: 生き物のように揺れる
			float pitchWave = sinf(m_spiralTime * 3.5f) * m_spiralPitchWave;
			float pitchDescend = -0.15f * progress;

			// 着地直前はピッチを水平に戻す（より早く）
			if (progress > 0.5f) {
				float levelProgress = (progress - 0.5f) / 0.5f;
				levelProgress = levelProgress * levelProgress; // 二次曲線
				pitchDescend = Lerp(pitchDescend, 0.0f, levelProgress);
				pitchWave *= (1.0f - levelProgress);
			}

			m_Rotation.x = pitchDescend + pitchWave;

			// 8. ロール角: 旋回に合わせて傾く
			float rollFromTurn = -sinf(angle) * m_spiralRollIntensity * (1.0f - progress * 0.5f);
			float rollWave = cosf(m_spiralTime * 2.8f) * 0.08f;

			// 着地直前はロールを水平に戻す（より早く）
			if (progress > 0.5f) {
				float levelProgress = (progress - 0.5f) / 0.5f;
				levelProgress = levelProgress * levelProgress; // 二次曲線
				rollFromTurn *= (1.0f - levelProgress);
				rollWave *= (1.0f - levelProgress);
			}

			m_Rotation.z = rollFromTurn + rollWave;
		}
	}

	// カウントダウン中
	else if (m_stateManager.IsCountdown())
	{
		//カウントダウンエフェクトを最初の1回だけ開始
		if (!m_countdownStarted)
		{
			m_countdown->Start();
			m_countdownStarted = true;
		}

		// カウントダウンエフェクトの更新
		m_countdown->Update(deltatime);

		if (m_countdown->IsFinished())
		{
			// カウントダウン終了、レース開始
			m_stateManager.RemoveState(PlayerStateManager::State::Countdown);
			m_stateManager.AddState(PlayerStateManager::State::RaceReady);
			// フラグをリセット（次回のレースのため）GameSceneNormalbgm
			m_countdownStarted = false;
			//通常時BGM開始
			SoundManager::GetInstance().PlayBGM("GameSceneNormalbgm");
		}

		// カウントダウン中は静止
		m_velocity = Vector3(0.0f, 0.0f, 0.0f);
		m_Rotation.x = 0.0f; // 水平に戻す
	}

	// レース準備完了（この状態はすぐに解除される）
	else if (m_stateManager.IsRaceReady())
	{
		// 通常の操作を許可する
		m_stateManager.RemoveState(PlayerStateManager::State::RaceReady);
		// 特に状態を追加しない（通常のゲームプレイに戻る）
	}
}

void Player::OnGoal()//Goal時の演出をあいまいにするため、GoalSequence状態に遷移させるだけにして、実際の演出はUpdateGoalSequenceで行う
{
	m_postProcessSetter(false, 0.0f);
	m_hasGoaled = true;
	m_goalEffectStarted = false;

	// ゴール状態に遷移
	m_stateManager.ClearAllStates();
	m_stateManager.AddState(PlayerStateManager::State::GoalSequence);
	m_stateManager.AddState(PlayerStateManager::State::OnGround);

	// ゴールジャンプの初期化
	m_goalJumpTime = 0.0f;
	m_goalStartPos = m_Position;
	m_goalStartRotation = m_Rotation;

	// 進行方向を取得
	m_goalDirection = Vector3(sin(m_Rotation.y), 0.0f, cos(m_Rotation.y));
	m_goalDirection.Normalize();
	GoalCamera& cam = GoalCamera::Instance();
	cam.StartGoalSequence();
}
void Player::UpdateGoalSequence(float deltatime)
{
	// UI演出を最初の1回だけ開始
	if (!m_goalEffectStarted)
	{
		m_goalEffect->Start();
		SoundManager::GetInstance().StopBGM();
		SoundManager::GetInstance().PlaySE("Goal", 1.0f);

		m_goalEffectStarted = true;
	}

	// UI演出の更新
	m_goalEffect->Update(deltatime);

	//プレイヤーのロケット飛行アニメーション（螺旋上昇）
	m_goalJumpTime += deltatime;
	float t = m_goalJumpTime / m_goalJumpDuration;

	if (t <= 1.0f)
	{
		float forward, height;
		float spiralRadius = 5.0f;  // 螺旋の半径
		float spiralRotations = 3.0f;  // 螺旋の回転数

		if (t < 0.4f) {
			//フェーズ1 (0~0.4秒): ゆっくり上昇
			float t1 = t / 0.4f;

			// 緩やかな二次曲線
			forward = m_goalJumpDistance * 0.2f * t1 * t1;
			height = m_goalJumpHeight * 0.15f * t1 * t1;

			// 螺旋回転（ゆっくり）
			float spiralAngle = t1 * 3.14159f * 2.0f * spiralRotations * 0.3f;  // 前半は少しだけ回転
			spiralRadius *= t1;  // 徐々に半径を広げる

			// 螺旋オフセットを計算
			Vector3 rightDir = Vector3(-m_goalDirection.z, 0, m_goalDirection.x);
			rightDir.Normalize();
			Vector3 spiralOffset = rightDir * (cosf(spiralAngle) * spiralRadius);
			spiralOffset += m_goalDirection * (sinf(spiralAngle) * spiralRadius * 0.5f);  // 前後にも少し揺れる

			// 位置を計算
			Vector3 forwardOffset = m_goalDirection * forward;
			Vector3 upOffset = Vector3(0, height, 0);
			m_Position = m_goalStartPos + forwardOffset + upOffset + spiralOffset;

			//プレイヤーの向き：徐々に上を向く
			m_Rotation.y = m_goalStartRotation.y + spiralAngle;
			m_Rotation.x = -0.3f * t1;  // 徐々に上を向き始める（最大約-17度）

			// 速度も遅め
			float speed = (m_goalJumpDistance / m_goalJumpDuration) * 0.5f;
			m_velocity = m_goalDirection * speed;
			m_velocity.y = speed * 0.3f;

		}
		else {
			// フェーズ2 (0.4~1.0秒):螺旋
			float t2 = (t - 0.4f) / 0.6f;

			// 三次関数で急加速
			float accel = t2 * t2 * t2;

			forward = m_goalJumpDistance * 0.2f +
				m_goalJumpDistance * 0.8f * accel;
			height = m_goalJumpHeight * 0.15f +
				m_goalJumpHeight * 0.85f * accel;

			// 螺旋回転（速く）
			float spiralAngle = (0.3f + t2 * 0.7f) * 3.14159f * 2.0f * spiralRotations;  // 後半で激しく回転

			// 螺旋オフセットを計算
			Vector3 rightDir = Vector3(-m_goalDirection.z, 0, m_goalDirection.x);
			rightDir.Normalize();
			Vector3 spiralOffset = rightDir * (cosf(spiralAngle) * spiralRadius);
			spiralOffset += m_goalDirection * (sinf(spiralAngle) * spiralRadius * 0.5f);

			// 位置を計算
			Vector3 forwardOffset = m_goalDirection * forward;
			Vector3 upOffset = Vector3(0, height, 0);
			m_Position = m_goalStartPos + forwardOffset + upOffset + spiralOffset;

			//プレイヤーの向き：上向き（ロケットのように
			m_Rotation.y = m_goalStartRotation.y + spiralAngle;
			m_Rotation.x = Lerp(-0.3f, -1.4f, t2);  // -17度 → -80度（ほぼ真上）

			// ロール（螺旋の傾き）
			m_Rotation.z = sinf(spiralAngle) * 0.3f * t2;

			// 速度も急加速（最大4倍）
			float speedMult = 1.0f + t2 * 3.0f;
			float speed = (m_goalJumpDistance / m_goalJumpDuration) * speedMult;
			m_velocity = m_goalDirection * speed;
			m_velocity.y = speed * 1.5f * speedMult;  // 上昇速度も加速
		}
	}
	else
	{
		// ジャンプ終了後もさらに加速して飛び続ける
		float overTime = m_goalJumpTime - m_goalJumpDuration;

		// 螺旋は継続（徐々に半径が小さくなる）
		float spiralAngle = 3.14159f * 2.0f * 3.0f + overTime * 3.0f;  // 回転継続
		float spiralRadius = 5.0f * (1.0f - overTime * 0.5f);  // 徐々に縮小
		spiralRadius = std::max(spiralRadius, 1.0f);  // 最小1m

		Vector3 rightDir = Vector3(-m_goalDirection.z, 0, m_goalDirection.x);
		rightDir.Normalize();
		Vector3 spiralOffset = rightDir * (cosf(spiralAngle) * spiralRadius);

		// どんどん加速
		float extraHeight = m_goalJumpHeight * 3.0f * overTime;
		float extraDistance = m_goalJumpDistance * 2.0f * overTime;

		Vector3 forwardOffset = m_goalDirection * (m_goalJumpDistance + extraDistance);
		Vector3 upOffset = Vector3(0, m_goalJumpHeight + extraHeight, 0);

		m_Position = m_goalStartPos + forwardOffset + upOffset + spiralOffset;

		//回転は継続、上向き維持 
		m_Rotation.y = m_goalStartRotation.y + spiralAngle;
		m_Rotation.x = -1.4f;  // ほぼ真上を向く（約-80度）
		m_Rotation.z = sinf(spiralAngle) * 0.2f;

		// 速度も最大
		m_velocity = m_goalDirection * (m_goalJumpDistance / m_goalJumpDuration) * 5.0f;
		m_velocity.y = m_goalJumpHeight * 3.0f;
	}

	// UI演出が完全に終了したらリザルトへ
	if (m_goalEffect->IsFinished() && t > 1.0f)
	{
		// リザルト画面へ遷移
		SceneManager::ChangeScene("Ending");// ゴールに到達したらResultSceneへ遷移
	}
}


void Player::Init()
{
	// モデルの初期化
	m_mesh.Load(
		"assets/model/Cow_Rocket_Adventure_1112064202_texture.fbx",// モデル名
		"assets/model/");						// テクスチャのパス
	//レンダラ初期化
	m_meshrenderer.Init(m_mesh);

	//自動計算: BottomYが負の場合は補正が必要
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




	if (!m_sparkEmitter.Init(Renderer::GetDevice()))
	{
		OutputDebugStringA("サンプラーステート作成失敗\n");
	}
	m_Scale = Vector3(5.0f, 5.0f, 5.0f);

	m_stateManager.ClearAllStates();
	m_stateManager.AddState(PlayerStateManager::State::OnGround);

	// カウントダウンエフェクトの初期化
	m_countdown = std::make_unique <CountdownEffect>();
	m_countdown->Initialize(
		Vector2(0.5f, 0.4f),   // 数字位置（中央の白枠）
		Vector2(0.25f, 0.5f),  // GO位置（左の白枠）
		0.15f,                 // 数字サイズ
		0.12f,                 // GOサイズ
		0.25f                  // 背景サイズ
	);
	m_countdownStarted = false;

	//ゴール演出の初期化
	// ゴール演出の初期化
	m_goalEffect=std::make_unique <GoalEffect>();
	m_goalEffect->Initialize();
	m_hasGoaled = false;
	m_goalEffectStarted = false;

	// ゴールジャンプパラメータ
	m_goalJumpDuration = 3.0f;        // 3秒間飛び続ける
	m_goalJumpHeight = 500.0f;         // 最終的に50m上昇
	m_goalJumpDistance = 70.0f;       // 前方に30m進む
	m_goalRotationSpeed = 720.0f;     // 1回転
	m_goalAcceleration = 5.0f;        // 加速倍率


}
//まだちょいがたつく
void Player::Update(float deltatime)
{
	if (m_stateManager.IsGoalSequence())
	{
		UpdateGoalSequence(deltatime);
		// 地形追従は行う
		UpdateSmoothTerrainFollowing(deltatime);
		// バウンディングスフィアの更新
		float bottomOffsetY = m_mesh.GetBottomY();
		m_BoundingSphere = {
			Vector3(
				m_Position.x,
				m_Position.y + (bottomOffsetY * m_Scale.y) + (m_BoundingSphere.radius * 0.5f),
				m_Position.z
			),
			0.5f,
		};
		return; // 通常のUpdate処理をスキップ
	}

	// スタートシーケンス中は専用の処理
	if (m_stateManager.IsInStartSequence())
	{
		UpdateStartSequence(deltatime);

		// 地形追従は行うが、入力は受け付けない
		UpdateSmoothTerrainFollowing(deltatime);

		// バウンディングスフィアの更新
		float bottomOffsetY = m_mesh.GetBottomY();
		m_BoundingSphere = {
			Vector3(
				m_Position.x,
				m_Position.y + (bottomOffsetY * m_Scale.y) + (m_BoundingSphere.radius * 0.5f),
				m_Position.z
			),
			0.5f,
		};

		// スパークエフェクトの更新
		m_sparkEmitter.Update(deltatime);

		return; // 通常のUpdate処理をスキップ
	}

	// 通常のゲームプレイ時は状態を更新
	if (!m_stateManager.IsInStartSequence())
	{
		// 地面との接触状態を更新
		if (m_isGrounded)
		{
			if (!m_stateManager.IsOnGround())
			{
				m_stateManager.AddState(PlayerStateManager::State::OnGround);
			}
		}
		else
		{
			m_stateManager.RemoveState(PlayerStateManager::State::OnGround);
		}

		// ドリフト状態を更新
		if (m_isDrifting)
		{
			if (!m_stateManager.IsDrifting())
			{
				m_stateManager.AddState(PlayerStateManager::State::Drifting);
			}
		}
		else
		{
			m_stateManager.RemoveState(PlayerStateManager::State::Drifting);
		}

		// 走行状態を更新（一定速度以上で走行中とみなす）
		float currentSpeed = sqrt(m_velocity.x * m_velocity.x + m_velocity.z * m_velocity.z);
		if (currentSpeed > 0.5f)
		{
			if (!m_stateManager.IsRunning())
			{
				m_stateManager.AddState(PlayerStateManager::State::Running);
			}
		}
		else
		{
			m_stateManager.RemoveState(PlayerStateManager::State::Running);
		}
	}

	if (m_hitStopTimer > 0.0f)//ヒットストップがなんかしっくりこない
	{
		m_hitStopTimer -= deltatime;

		if (m_hitStopTimer <= 0.0f)
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


			m_hitStopTimer = 0.0f;
		}
	}

	float timeScale = GameManager::Instance().GetTimeScale();
    float scaledDeltaTime = deltatime * timeScale;  //ここで一度だけ計算

	float throttle = 0.0f;
	float steering = 0.0f;
	// ブースト入力チェック
	bool boostInput = CDirectInput::GetInstance().CheckKeyBuffer(DIK_SPACE)|| InputManager::GetInstance()->GetButton(SDL_CONTROLLER_BUTTON_B);

	// ブースト状態の更新
	UpdateBoostSystem(boostInput, deltatime);

	//速度システムの更新
	UpdateSpeedBonusSystem(deltatime);

	//if (CDirectInput::GetInstance().CheckKeyBuffer(DIK_O))
	//{
	//	AddBoostGauge(5.0f);
	//}




	// 前進・後退の入力
	throttle = 0.0f;

	// キーボード入力
	if (CDirectInput::GetInstance().CheckKeyBuffer(DIK_W)) {
		throttle = 1.0f;
	}
	else if (CDirectInput::GetInstance().CheckKeyBuffer(DIK_S)) {
		throttle = -0.5f;
	}

	// コントローラー入力（Aボタン = 前進）
	if (InputManager::GetInstance()->IsConnected())
	{
		if (InputManager::GetInstance()->GetButton(SDL_CONTROLLER_BUTTON_A))
		{
			throttle = 1.0f; // Aボタンで前進
		}
		else if (InputManager::GetInstance()->GetButton(SDL_CONTROLLER_BUTTON_B))
		{
			//throttle = -0.5f; // Bボタンで後退（必要なら）
		}
	}

	// ステアリング入力
	steering = 0.0f;

	// キーボードのステアリング
	if (CDirectInput::GetInstance().CheckKeyBuffer(DIK_A)) {
		steering = -1.0f; // 左
	}
	else if (CDirectInput::GetInstance().CheckKeyBuffer(DIK_D)) {
		steering = 1.0f;  // 右
	}

	// ドリフト入力チェック
	bool driftInput = false;
	m_driftDirection = 0.0f;

	// キーボードのドリフト
	if (CDirectInput::GetInstance().CheckKeyBuffer(DIK_Q)) {
		driftInput = true;
		m_driftDirection = -1.0f;
	}
	else if (CDirectInput::GetInstance().CheckKeyBuffer(DIK_E)) {
		driftInput = true;
		m_driftDirection = 1.0f;
	}

	// コントローラーの処理
	if (InputManager::GetInstance()->IsConnected())
	{
		float leftX = InputManager::GetInstance()->GetAxis(SDL_CONTROLLER_AXIS_LEFTX);
		const float THRESHOLD = 0.3f;

		// ZR/R ボタンが押されているかチェック（ドリフトボタン）
		bool isDriftButtonPressed =
			InputManager::GetInstance()->GetButton(SDL_CONTROLLER_BUTTON_RIGHTSHOULDER)|| InputManager::GetInstance()->GetButton(SDL_CONTROLLER_BUTTON_LEFTSHOULDER);

		if (abs(leftX) > THRESHOLD)
		{
			if (isDriftButtonPressed)
			{
				// Rボタン押下中 + スティック左右 = ドリフト
				driftInput = true;
				m_driftDirection = leftX;

				// またはキーボードと同じく-1.0/1.0にする場合
				// m_driftDirection = (leftX < 0.0f) ? -1.0f : 1.0f;
			}
			else
			{
				// Rボタンなし + スティック左右 = 通常ハンドル操作
				steering = leftX;

				// またはキーボードと同じく-1.0/1.0にする場合
				// steering = (leftX < 0.0f) ? -1.0f : 1.0f;
			}
		}
	}

	// 車の向きベクトルを計算
	Vector3 forwardDir = Vector3(sinf(m_Rotation.y), 0.0f, cosf(m_Rotation.y));
	Vector3 rightDir = Vector3(cosf(m_Rotation.y), 0.0f, -sinf(m_Rotation.y));

	float currentSpeed = sqrt(m_velocity.x * m_velocity.x + m_velocity.z * m_velocity.z);
	float speedFactor = std::min(currentSpeed / m_maxSpeed, 1.0f);

	// ドリフト状態の管理
	if (driftInput && currentSpeed > 0.5f) {
		m_isDrifting = true;
	}
	else if (!driftInput || currentSpeed < 0.2f) {
		m_isDrifting = false;
	}

	// 前フレームの位置を保存
	m_previousPosition = m_Position;

	// 重力を適用（地面に接触していない場合）
	ApplyGravity(deltatime );

	// ドリフト時とノーマル時で異なる処理
	if (m_isDrifting)
	{
		// ドリフト中の処理
		UpdateDriftMovement(throttle, steering, forwardDir, rightDir, speedFactor);
	}
	else {
		// 通常の処理
		UpdateNormalMovement(throttle, steering, forwardDir, rightDir, speedFactor);
	}

	UpdateLastGroundedRoad();

	// 高速移動時の位置更新
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
		m_collisionRadius,
	};

	// 滑らかな地形追従処理
	UpdateSmoothTerrainFollowing(deltatime);

	// 落下状態チェック
	CheckFallState(deltatime);


	// 敵との当たり判定
	for (auto& enemy : GetAllEnemys()) 
	{
		if (CollisionSphere(m_BoundingSphere, enemy->GetEnemyBoundingSphere())) 
		{
			if (!enemy->m_isKnockedBack) {

				OnCollisionWithEnemy(*enemy);
				AddBoostGauge(5.0f);//敵を吹っ飛ばした際に加速ゲージを取得　加速の最大値も少し上昇
			}
		}
	}

	for (auto& enemy : GetAllWeavingEnemies())
	{
		if (!enemy || !enemy->GetActive()) continue;
		if (CollisionSphere(m_BoundingSphere, enemy->GetEnemyBoundingSphere()))
		{
			if (!enemy->GetIsKnockedBack()) {
				OnCollisionWithEnemy(*enemy); // WeavingEnemy版が呼ばれる
				AddBoostGauge(5.0f);
			}
		}
	}

	m_sparkEmitter.Update(scaledDeltaTime);



	// 最高速度の時だけアフターイメージ
	if (m_isMaxSpeed)
	{
		UpdateAfterImage(deltatime); // deltaTimeは既存のものを使用

		if (!m_isBGM)
		{
			SoundManager::GetInstance().StopBGM();
			SoundManager::GetInstance().PlayBGM("GameSceneAccerationbgm");
			m_isBGM = true;
		}
	}
}

void Player::UpdateAfterImage(float deltaTime)
{
	m_ghostSpawnTimer += deltaTime;

	if (m_ghostSpawnTimer >= GHOST_SPAWN_INTERVAL) {
		// 現在のワールド行列を計算
		SRT srt;
		srt.pos = m_Position;
		srt.rot = m_Rotation;
		srt.scale = m_Scale;
		Matrix4x4 worldmtx = srt.GetMatrix();
		Matrix rotationMatrix = Matrix::CreateRotationY(DirectX::XMConvertToRadians(90.0f));
		worldmtx = rotationMatrix * worldmtx;

		GhostData ghost;
		ghost.worldMatrix = worldmtx;	
		ghost.alpha = 1.0f; // 濃いめの初期アルファ値
		ghost.lifetime = 0.0f;

		m_ghostTrail.push_back(ghost);
		m_ghostSpawnTimer = 0.0f;

		if (m_ghostTrail.size() > MAX_GHOST_COUNT) {
			m_ghostTrail.pop_front();
		}
	}

	// 既存の残像を更新
	for (auto& ghost : m_ghostTrail) {
		ghost.lifetime += deltaTime;
		ghost.alpha -= deltaTime * 5.5f; // フェードアウト速度
	}

	m_ghostTrail.erase(
		std::remove_if(m_ghostTrail.begin(), m_ghostTrail.end(),
			[](const GhostData& g) { return g.alpha <= 0.0f; }),
		m_ghostTrail.end()
	);
}


// ブーストシステムの更新処理
void Player::UpdateBoostSystem(bool boostInput, float deltaSeconds)
{
	if (boostInput && m_boostGauge > 0.0f)
	{
		// ブースト使用中
		m_isBoosting = true;
		m_boostGauge -= m_boostConsumption * deltaSeconds;
		// ゲージが0以下になったら停止
		if (m_boostGauge <= 0.0f) 
		{
			m_boostGauge = 0.0f;
			m_isBoosting = false;
		}
	}
	else {
		// ブースト未使用
		m_isBoosting = false;
		m_postProcessSetter(false, 0.0f);
		m_isMaxSpeed = false;

	}
}

void Player::UpdateSpeedBonusSystem(float deltatime)
{
	// 一時ボーナスのタイマー更新
	if (m_speedBoostTimer > 0.0f)
	{
		m_speedBoostTimer -= deltatime;

		if (m_speedBoostTimer <= 0.0f)
		{
			m_speedBoostTimer = 0.0f;
		}
	}

	// 一時ボーナスの減衰（タイマーが切れた後）
	if (m_speedBoostTimer <= 0.0f && m_temporarySpeedBonus > 1.0f)
	{
		// 緩やかに1.0に戻す
		m_temporarySpeedBonus = Lerp(m_temporarySpeedBonus, 1.0f, m_temporaryBonusDecaySpeed);

		// 1.0に十分近づいたら完全に1.0にする
		if (abs(m_temporarySpeedBonus - 1.0f) < 0.01f)
		{
			m_temporarySpeedBonus = 1.0f;
		}
	}

	// 速度に応じたポストプロセス効果（常時適用）
	float currentMultiplier = GetCurrentSpeedMultiplier();
	if (currentMultiplier > 1.5f)
	{
		// 速度倍率が1.5倍以上の時はモーションブラーを強化
		float blurIntensity = std::min((currentMultiplier - 1.0f) * 0.02f, 0.05f);
		m_postProcessSetter(true, blurIntensity);
		m_isMaxSpeed = true;

		//SpringCamera::Instance().PlusCurrentFOV();
	}
	else if (!m_isBoosting)
	{
		// ブーストもしていない、速度倍率も低い場合は無効化
		m_postProcessSetter(false, 0.0f);
		m_isMaxSpeed = false;

		if (m_isBGM)
		{///元のBGMに戻す
			SoundManager::GetInstance().StopBGM();
			SoundManager::GetInstance().PlayBGM("GameSceneNormalbgm");
			m_isBGM = false;
		}
	}
}


//現在の速度倍率を取得
float Player::GetCurrentSpeedMultiplier() const
{
	return m_permanentSpeedBonus * m_temporarySpeedBonus;
}

void Player::UpdateSmoothTerrainFollowing(float deltatime)
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

			float horizontalSpeed = sqrt(m_velocity.x * m_velocity.x + m_velocity.z * m_velocity.z);
			bool isHighSpeed = m_isBoosting && horizontalSpeed > 2.0f;

			float contactThreshold = isHighSpeed ? 1.5f : 2.0f;

			if (heightDifference <= contactThreshold) {
				onRoad = true;
				m_isGrounded = true;

				m_targetHeight = terrainHeight - (bottomOffsetY * m_Scale.y);

				float currentHeight = m_Position.y;
				float heightDiff = abs(m_targetHeight - currentHeight);// 高さの差分

				Vector3 horizontalVelocity = Vector3(m_velocity.x, 0.0f, m_velocity.z);
				float horizontalSpeedForLerp = horizontalVelocity.Length();
				bool isNearlyStationary = horizontalSpeedForLerp < 0.05f;

				//下り坂判定の追加
				bool isMovingDownhill = false;
				if (horizontalSpeedForLerp > 0.1f) 
				{
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
				if (isHighSpeed) 
				{
					if (heightDiff > 1.5f)
					{
						//// 即座に修正
						m_Position.y = m_targetHeight;

						//垂直速度をリセット
						m_verticalVelocity = 0.0f;
					}
					else 
					{
						//段差が細かい場所補間なのでゆっくり補間
						float lerpSpeed = isMovingDownhill ? 0.5f : 0.3f; // 下り坂ではより強く
						m_Position.y = Lerp(currentHeight, m_targetHeight, lerpSpeed);

						//下り坂では垂直速度を減衰
						if (isMovingDownhill && m_verticalVelocity < 0.0f) {
							m_verticalVelocity *= m_downhillVelocityDamping;
						}
					}
				}
				else {
					// 通常速度時の処理
					if (heightDiff > 3.0f) {
						m_Position.y = m_targetHeight;
						m_verticalVelocity = 0.0f; //リセット
					}
					else {
						float lerpSpeed = isNearlyStationary ? 0.25f : 0.1f;
						if (heightDiff < 0.05f) lerpSpeed = 0.5f;
						m_Position.y = Lerp(currentHeight, m_targetHeight, lerpSpeed);
					}
				}

				//地面接触時は垂直速度をリセット
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
						CDirectInput::GetInstance().CheckKeyBuffer(DIK_S)|| InputManager::GetInstance()->GetButton(SDL_CONTROLLER_BUTTON_RIGHTSHOULDER)|| InputManager::GetInstance()->GetButton(SDL_CONTROLLER_BUTTON_LEFTSHOULDER));

					if (slopeDot > 0 && hasThrottleInput)
					{
						// 下り坂 - 何もしない（バウンド防止のため加速を控える）
					}
					else {
						// 上り坂抵抗
						float resistanceFactor = 1.0f;
						m_velocity *= resistanceFactor;
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

	//前フレームの状態を保存
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
		m_verticalVelocity += m_gravity * (deltatime *0.1f) * 60.0f; // 60FPS基準で調整

		// 落下速度の制限（ターミナル速度）
		const float maxFallSpeed = -10.0f;
		if (m_verticalVelocity < maxFallSpeed) {
			m_verticalVelocity = maxFallSpeed;
		}
	}
}

void Player::ApplyRoadSurfaceEffect(RoadType surfaceType, float deltatime)//道路ごとに効果をつけられる
{
	float timeScale = GameManager::Instance().GetTimeScale();

	switch (surfaceType) {
	case RoadType::DIRT:
	{
		if (!m_isMaxSpeed)//最高速度時は影響を受けない
		{
		// ダートでは摩擦が大きく、速度が低下
		float dirtFriction = 0.96f;  // 通常より強い減速（0.98fが通常）
		m_velocity.x *= pow(dirtFriction, timeScale);
		m_velocity.z *= pow(dirtFriction, timeScale);

		// 最大速度も制限
		float dirtMaxSpeedRatio = 0.75f;  // 通常の75%の速度
		float dirtMaxSpeed = m_maxSpeed * dirtMaxSpeedRatio;

		if (m_isBoosting) {
			dirtMaxSpeed *= m_boostRatio;  // ブースト時も制限
		}

		float currentSpeed = sqrt(m_velocity.x * m_velocity.x +
			m_velocity.z * m_velocity.z);
		if (currentSpeed > dirtMaxSpeed) {
			float speedRatio = dirtMaxSpeed / currentSpeed;
			m_velocity.x *= speedRatio;
			m_velocity.z *= speedRatio;
		}

		}

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

	m_Rotation.y += steering * m_turnSpeed * speedFactor * timeScale;

	if (m_Rotation.y > PI) m_Rotation.y -= 2.0f * PI;
	if (m_Rotation.y < -PI) m_Rotation.y += 2.0f * PI;

	float accelerationMultiplier = m_isBoosting ? m_boostPower : 1.0f;
	Vector3 forceDir = forwardDir * throttle * m_acceleration * accelerationMultiplier * timeScale;
	m_velocity += forceDir;

	m_speed = sqrt(m_velocity.x * m_velocity.x + m_velocity.z * m_velocity.z);
	Vector3 velocityDir = Vector3(0.0f, 0.0f, 0.0f);
	if (m_speed > 0.01f) {
		velocityDir = m_velocity * (1.0f / m_speed);
	}
	float alignment = forwardDir.x * velocityDir.x + forwardDir.z * velocityDir.z;
	Vector3 targetVelocity = forwardDir * m_speed * alignment;

	float adjustedGrip = 1.0f - pow(1.0f - m_gripFactor, timeScale);
	m_velocity = m_velocity * (1.0f - adjustedGrip) + targetVelocity * adjustedGrip;

	float decelerationMultiplier = m_isBoosting ? 0.998f : m_deceleration;
	m_velocity *= pow(decelerationMultiplier, timeScale);

	//  速度制限にハイブリッド速度倍率を適用
	float speedMultiplier = GetCurrentSpeedMultiplier();
	float maxSpeed = m_maxSpeed * speedMultiplier;

	if (m_isBoosting)
	{
		maxSpeed *= m_boostRatio;
	}

	m_speed = sqrt(m_velocity.x * m_velocity.x + m_velocity.z * m_velocity.z);
	if (m_speed > maxSpeed) {
		m_velocity = m_velocity * (maxSpeed / m_speed);
	}

	Vector3 backmovement = Vector3(-velocityDir.x, 0.0f, velocityDir.z);
	Vector3 dustPos = m_Position;
	dustPos.x += 1.0f;
	dustPos.y += 2.0f;

	m_sparkEmitter.Emit(dustPos, Vector3(0.0f, 1.0f, 0.0f));
}

// ドリフト移動処理にもブーストを適用
void Player::UpdateDriftMovement(float throttle, float steering, Vector3 forwardDir, Vector3 rightDir, float speedFactor)
{
	float timeScaleDrift = GameManager::Instance().GetTimeScale();

	float driftRotation = m_driftDirection * m_driftTurnSpeed * speedFactor;
	m_Rotation.y += driftRotation;

	m_Rotation.y += steering * m_turnSpeed * speedFactor * 0.5f * timeScaleDrift;

	if (m_Rotation.y > PI) m_Rotation.y -= 2.0f * PI;
	if (m_Rotation.y < -PI) m_Rotation.y += 2.0f * PI;

	float accelerationMultiplier = m_isBoosting ? m_boostPower : 1.0f;

	Vector3 forceDir = forwardDir * throttle * m_acceleration * 0.8f * accelerationMultiplier * timeScaleDrift;
	m_velocity += forceDir;

	Vector3 lateralForce = rightDir * m_driftDirection * 0.02f * speedFactor;
	m_velocity += lateralForce;

	//速度制限にハイブリッド速度倍率を適用
	float speedMultiplier = GetCurrentSpeedMultiplier();
	float maxSpeed = m_maxSpeed * speedMultiplier;

	if (m_isBoosting)
	{
		maxSpeed *= 1.2f;  // ドリフト中のブースト時はさらに1.2倍
	}

	m_speed = sqrt(m_velocity.x * m_velocity.x + m_velocity.z * m_velocity.z);
	if (m_speed > maxSpeed) {
		m_velocity = m_velocity * (maxSpeed / m_speed);
	}

	Vector3 velocityDir = Vector3(0.0f, 0.0f, 0.0f);
	if (m_speed > 0.01f) {
		velocityDir = m_velocity * (1.0f / m_speed);
	}

	float alignment = forwardDir.x * velocityDir.x + forwardDir.z * velocityDir.z;
	Vector3 targetVelocity = forwardDir * m_speed * alignment;

	float adjustedGrip = 1.0f - pow(1.0f - m_gripFactor, timeScaleDrift);
	m_velocity = m_velocity * (1.0f - adjustedGrip) + targetVelocity * adjustedGrip;

	float decelerationMultiplier = m_isBoosting ? 0.985f : (m_deceleration * 0.98f);
	m_velocity *= pow(decelerationMultiplier, timeScaleDrift);
}


void Player::Draw()
{

	auto mode = Renderer::GetRenderMode();
	// SRT情報作成
	SRT srt;
	srt.pos = m_Position;
	srt.rot = m_Rotation;
	srt.scale = m_Scale;
	Matrix4x4 worldmtx;
	worldmtx = srt.GetMatrix();
	Matrix rotationMatrix = Matrix::CreateRotationY(DirectX::XMConvertToRadians(90.0f)); // または -90.0f
	if (m_isResultMode)
	{
		rotationMatrix = Matrix::CreateRotationY(DirectX::XMConvertToRadians(145.0f));
		worldmtx = rotationMatrix * worldmtx;
	}
	worldmtx = rotationMatrix * worldmtx;
	Renderer::SetWorldMatrix(&worldmtx);

	Matrix4x4 worldMatrix = DirectX::XMMatrixIdentity();

	//m_sparkEmitter.Render(Renderer::GetDeviceContext(), worldMatrix);

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

	// カウントダウン中の場合、エフェクトを描画
	if (m_stateManager.IsCountdown() && m_countdown) {
		m_countdown->Draw();
	}

	if (m_stateManager.IsGoalSequence() && m_goalEffect)
	{
		m_goalEffect->Draw();
	}



	// デバッグ用
	g_position = m_Position;
	g_rotation = m_Rotation;
	g_scale = m_Scale;


	if (m_roadManager) {
		float terrainHeight;
		Vector3 terrainNormal;
		if (m_roadManager->GetTerrainHeight(m_Position, terrainHeight, terrainNormal)) {
			Color terrainColor(1, 0, 0, 0.8f); // 赤色
			SphereDrawerDraw(0.3f, terrainColor, m_Position.x, terrainHeight, m_Position.z);
		}
	}

	// 通常描画モードのみアフターイメージを描画
	if (mode != Renderer::RenderMode::SHADOW_MAP &&m_isMaxSpeed)
	{
		DrawAfterImage();
	}
}

void Player::DrawAfterImage()
{// ブレンドステートを半透明に設定
	Renderer::SetBlendState(EBlendState::BS_ADDITIVE);
	// デプステストは有効、デプス書き込みは無効
	Renderer::SetDepthEnable(true);   // 深度テストON
	// シェーダー設定
	m_shader.SetGPU();

	const float AFTER_IMAGE_OPACITY = 0.8f;
	const float GRADATION = 0.6f;

	// 速度ベースの透明度調整パラメータ
	const float MIN_SPEED_THRESHOLD = 0.1f;
	const float MAX_SPEED_THRESHOLD = 5.0f;
	float currentSpeed = m_velocity.Length();
	float speedAlphaFactor = std::clamp(
		(currentSpeed - MIN_SPEED_THRESHOLD) / (MAX_SPEED_THRESHOLD - MIN_SPEED_THRESHOLD),
		0.0f,
		1.0f
	);

	int index = 0;
	int totalGhosts = m_ghostTrail.size();

	// マテリアルテンプレートを事前取得(参照で)
	const std::vector<MATERIAL>& materialTemplates = m_mesh.GetMaterials();

	// 残像を古い順に描画
	for (const auto& ghost : m_ghostTrail) {
		// ワールド行列設定
		Renderer::SetWorldMatrix(const_cast<Matrix4x4*>(&ghost.worldMatrix));

		// グラデーション係数を計算(ゴーストごとに1回)
		float fadeRatio = (float)index / (float)totalGhosts;

		// マテリアルごとに描画
		for (const auto& matTemplate : materialTemplates)
		{
			MATERIAL mat = matTemplate;  // コピーして編集

			float displayAlpha = ghost.alpha * AFTER_IMAGE_OPACITY * (1.0f - fadeRatio * GRADATION) * speedAlphaFactor;
			float intensity = displayAlpha * 2.0f;

			//ショックウェーブに寄せた水色に
			mat.Diffuse.x = 0.2f * displayAlpha;
			mat.Diffuse.y = 0.8f * displayAlpha;
			mat.Diffuse.z = 1.0f * displayAlpha;
			mat.Diffuse.w = displayAlpha;
			mat.Emission.x = 0.3f * intensity;
			mat.Emission.y = 1.0f * intensity;
			mat.Emission.z = 1.5f * intensity;
			mat.TextureEnable = FALSE;

			m_meshrenderer.DrawWithCustomMaterial(mat);
		}

		index++;  // ゴーストごとにインクリメント
	}

	// ブレンドステートを元に戻す
	Renderer::SetBlendState(EBlendState::BS_ALPHABLEND);
}

void Player::Dispose()
{
	// unique_ptrは自動的に解放されるため、明示的な処理は不要
	// ただし、念のためリセット
	m_countdown.reset();
	m_goalEffect.reset();

	// dequeのクリア
	m_ghostTrail.clear();
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
	float knockbackForce = m_speed * KNOCKBACK_FORCE_MULTIPLIER;
	m_gameScore += knockbackForce / SCORE_DIVISOR;

	if (timeScale != 1.0f)
	{
		knockbackForce *=(1 / timeScale);//スローモーションでも同じ力で吹っ飛ぶように
	}

	//ハイブリッド速度システムの適用
	m_consecutiveHits++;

	// 【永続ボーナス】マイルストーン達成チェック
	if (m_consecutiveHits % m_hitsPerMilestone == 0 &&
		m_permanentSpeedBonus < m_maxPermanentBonus)
	{
		m_permanentSpeedBonus += m_permanentBonusPerMilestone;

		// 最大値を超えないように制限
		if (m_permanentSpeedBonus > m_maxPermanentBonus)
		{
			m_permanentSpeedBonus = m_maxPermanentBonus;
		}

		// マイルストーン達成時の特別な演出
		ApplyHitStop(0.025f, 0.005f);  // 通常より長めのヒットストップ
		SpringCamera::Instance().Shake(3.0f, 0.2f);  // 強めのカメラシェイク

		// ここでUI表示などを追加可能
	}

	// 【一時ボーナス】敵を倒すたびに加算
	m_temporarySpeedBonus += m_temporaryBonusPerHit;
	if (m_temporarySpeedBonus > m_maxTemporaryBonus)
	{
		m_temporarySpeedBonus = m_maxTemporaryBonus;
	}

	// 一時ボーナスのタイマーをリセット
	m_speedBoostTimer = m_temporaryBoostDuration;

	// 現在の速度に応じてヒットストップと演出を調整
	float speedRatio = m_speed / m_maxSpeed;
	if (speedRatio > 1.5f)
	{
		// 高速時はより派手な演出
		ApplyHitStop(0.1f, 0.5f);
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
	SoundManager::GetInstance().PlaySE("fly_away");

	enemy.ApplyKnockback(knockbackDirection, knockbackForce,timeScale);
}

void Player::OnCollisionWithEnemy(WeavingEnemy& enemy)
{
	using namespace DirectX::SimpleMath;
	// knockbackDirectionの計算は既存と全く同じ
	float timeScale = GameManager::Instance().GetTimeScale();
	Vector3 knockbackDirection = enemy.GetPosition() - this->GetPosition();
	knockbackDirection.y = 0.0f;
	if (knockbackDirection.Length() > 0.001f) 
	{
		knockbackDirection.Normalize();
	}
	else 
	{
		knockbackDirection = Vector3(sinf(this->GetRotation().y), 0.0f, cosf(this->GetRotation().y));
	}
	Vector3 cameraForward = SpringCamera::Instance().GetForward();
	cameraForward.y = 0.0f;
	cameraForward.Normalize();
	float cameraInfluence = 0.5f;
	knockbackDirection = knockbackDirection * (1.0f - cameraInfluence) +
		cameraForward * cameraInfluence;
	knockbackDirection.y = 0.1f;
	knockbackDirection.Normalize();
	float knockbackForce = m_speed * KNOCKBACK_FORCE_MULTIPLIER;
	// WeavingEnemyはスコアボーナスが高いので、加算量を増やす
	m_gameScore += (knockbackForce / SCORE_DIVISOR) ;
	if (timeScale != 1.0f) knockbackForce *= (1 / timeScale);

	// スコア・演出系は既存と同じ
	m_consecutiveHits++;
	if (m_consecutiveHits % m_hitsPerMilestone == 0 &&
		m_permanentSpeedBonus < m_maxPermanentBonus)
	{
		m_permanentSpeedBonus += m_permanentBonusPerMilestone;
		if (m_permanentSpeedBonus > m_maxPermanentBonus)
			m_permanentSpeedBonus = m_maxPermanentBonus;
		ApplyHitStop(0.025f, 0.005f);
		SpringCamera::Instance().Shake(3.0f, 0.2f);
	}
	m_temporarySpeedBonus += m_temporaryBonusPerHit;
	if (m_temporarySpeedBonus > m_maxTemporaryBonus)
		m_temporarySpeedBonus = m_maxTemporaryBonus;
	m_speedBoostTimer = m_temporaryBoostDuration;

	float speedRatio = m_speed / m_maxSpeed;
	if (speedRatio > 1.5f) {
		ApplyHitStop(0.1f, 0.5f);
		SpringCamera::Instance().Shake(2.0f, 0.15f);
	}
	else {
		ApplyHitStop(0.01f, 0.0001f);
		SpringCamera::Instance().Shake(1.0f, 0.1f);
	}
	SpringCamera::Instance().Shake(1.0f, 0.1f);
	SoundManager::GetInstance().PlaySE("fly_away");

	enemy.ApplyKnockback(knockbackDirection, knockbackForce, timeScale); // ← WeavingEnemyのApplyKnockback
}


void Player::ApplyHitStop(float duration, float timeScale /*= 0.0f*/)//	スローどうしよ　ピンとこんなー
{
	m_hitStopTimer = duration;
	m_PreviousTimeScale = GameManager::Instance().GetTimeScale();//連続で吹っ飛ばすとバグるので廃止
	//GameManager::Instance().SetTimeScale(timeScale);
}



void Player::AddBoostGauge(float amount)
{
	m_boostGauge += amount;
	if (m_boostGauge > m_maxBoostGauge)
	{
		m_boostGauge = m_maxBoostGauge;
	}
}
void Player::UpdatePositionWithCollisionCheck(float timeScale)
{
	// 現在の速度を取得
	float horizontalSpeed = sqrt(m_velocity.x * m_velocity.x + m_velocity.z * m_velocity.z);
	// 高速移動判定（ブースト使用時かつ一定速度以上の場合のみ）
	bool isHighSpeed = m_isBoosting && horizontalSpeed > 2.0f;

	// 変更: m_road から m_roadManager に変更
	if (isHighSpeed && m_roadManager)
	{
		// 高速移動時は段階的に位置を更新（ただし高さ調整は最小限に）
		int steps = std::max(3, (int)(horizontalSpeed / 1.0f)); // 速度に応じてステップ数を動的調整
		steps = std::min(steps, 6); // 最大6ステップに制限（処理負荷考慮）
		Vector3 stepVelocity = m_velocity * (timeScale / steps);

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
		m_Position.x += m_velocity.x*timeScale;
		m_Position.z += m_velocity.z*timeScale;

		// 垂直方向の位置更新（重力のみ、地形追従は後で処理）
		if (!m_isGrounded) {
			m_Position.y += m_verticalVelocity;
		}
	}

	if (m_treeManager)
	{
		auto result = m_treeManager->CheckCollision(m_Position, m_collisionRadius);
		if (result.hit)
		{
			// めり込み解消
			m_Position.x += result.normal.x * result.penetration;
			m_Position.z += result.normal.z * result.penetration;

			// 速度を反射・減衰
			m_velocity = TreeManager::ResolveCollision(result, m_velocity, 0.4f, 0.3f);

			// カメラシェイク（速度に応じて）
			float speed = sqrt(m_velocity.x * m_velocity.x + m_velocity.z * m_velocity.z);
			SpringCamera::Instance().Shake(speed * 0.3f, 0.1f);
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
	Vector3 horizontalVelocity = Vector3(m_velocity.x, 0.0f, m_velocity.z);
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
void Player::UpdateLastGroundedRoad()
{
	// 地面に接触している場合のみ記録
	if (m_isGrounded && m_roadManager) {
		BaseRoad* currentRoad = m_roadManager->GetRoadAtPosition(m_Position);

		if (currentRoad) {
			// 新しい道路に乗ったら更新
			if (currentRoad != m_lastGroundedRoad) {
				m_lastGroundedRoad = currentRoad;
			}

			// 最後に接地していた位置と向きを記録
			m_lastGroundedPosition = m_Position;
			m_lastGroundedRotation = m_Rotation;
			m_lastGroundedTime = 0.0f; // タイマーリセット
		}
	}
	else {
		m_lastGroundedTime += 0.016f; // 約60FPS想定
	}
}
void Player::CheckFallState(float deltatime)
{
	bool isFallingNow = false;

	// 空中時間を計測
	if (!m_isGrounded) {
		m_airTime += deltatime;
	}
	else {
		m_airTime = 0.0f;
	}

	// 落下判定の条件
	if (m_Position.y < FALL_THRESHOLD_Y) {
		isFallingNow = true;
	}

	if (!m_isGrounded && m_verticalVelocity < FALL_VELOCITY_THRESHOLD) {
		isFallingNow = true;
	}

	if (m_airTime > MAX_AIR_TIME) {
		isFallingNow = true;
	}

	//  落下状態になったらカメラ演出を開始（リスポーンは待機）
	if (isFallingNow && !m_isFalling && !m_isWaitingForRespawn) {
		m_isFalling = true;
		m_isWaitingForRespawn = true;
		m_fallTimer = 0.0f;

		// カメラを落下モードに切り替え
		SpringCamera::Instance().StartFallingMode();
	}

	// カメラ演出中はタイマーを進める
	if (m_isWaitingForRespawn) {
		m_fallTimer += deltatime;

		// カメラ演出が完了したらリスポーン実行
		if (m_fallTimer >= FALL_CAMERA_DURATION) {
			// リスポーン処理を実行
			RespawnToLastRoad();

			// カメラを通常モードに戻す
			SpringCamera::Instance().EndFallingMode();

			// フラグをリセット
			m_isWaitingForRespawn = false;
			m_fallTimer = 0.0f;
		}
	}

	//リスポーン後に接地したら落下状態を完全に解除
	if (m_isFalling && m_isGrounded && !m_isWaitingForRespawn) {
		m_isFalling = false;
		m_airTime = 0.0f;
	}
}

void Player::RespawnToLastRoad()
{
	Vector3 respawnPos;
	Vector3 respawnRot;
	bool foundRespawn = false;
	BaseRoad* targetRoad = nullptr;

	// 最後に接地していた道路からSTRAIGHTの道路を探す
	if (m_lastGroundedRoad) {
		targetRoad = m_roadManager->FindNearestStraightRoad(m_lastGroundedRoad);

		if (targetRoad) {
			foundRespawn = m_roadManager->GetSafePositionOnRoad(
				targetRoad,
				m_lastGroundedPosition,
				respawnPos,
				respawnRot  //STRAIGHTの道路の向きを取得
			);

		}
		else {
			// STRAIGHTが見つからない場合は元の道路を使用
			targetRoad = m_lastGroundedRoad;
			foundRespawn = m_roadManager->GetSafePositionOnRoad(
				targetRoad,
				m_lastGroundedPosition,
				respawnPos,
				respawnRot
			);
		}
	}

	// 記録された道路がない場合
	if (!foundRespawn) {
		auto startPos = m_roadManager->GetStartPos();
		if (startPos.has_value()) {
			respawnPos = startPos.value();
			respawnPos.y += 2.0f;
			respawnRot = Vector3(0.0f, 0.0f, 0.0f);
			foundRespawn = true;
		}
	}

	// リスポーン実行
	if (foundRespawn) {
		m_Position = respawnPos;

		//道路の向きに合わせる（Y軸回転のみ適用）
		m_Rotation.y = respawnRot.y;  // STRAIGHTの道路の向き
		m_Rotation.x = 0.0f;           // ピッチはリセット
		m_Rotation.z = 0.0f;           // ロールはリセット

		// 速度とステータスをリセット
		m_velocity *= 0.15f;
		m_verticalVelocity = 0.0f;
		m_isDrifting = false;
		m_isBoosting = false;

		// ペナルティの適用
		m_boostGauge *= 0.6f;

		if (m_temporarySpeedBonus > 1.0f) {
			m_temporarySpeedBonus = 1.0f;
			m_speedBoostTimer = 0.0f;
		}

		SpringCamera::Instance().Shake(2.0f, 0.25f);

	}
	else 
	{
		printf("どこにもリスポーン出来なかった\n");
	}
}