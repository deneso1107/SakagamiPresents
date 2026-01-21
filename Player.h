#pragma once

#include"CarPhysics.h"
#include"SparkEmitter.h"
#include"Enemy.h"
#include"ObjectBase.h"
#include"PlayerStateManager.h"
#include"CTerrainMesh.h"
#include"Road.h"
#include"scenemanager.h"
#include"RoadManager.h"
#include"GameManager.h"
#include"CountdownEffect.h"
#include"GoalEffect.h"
class Player:public ObjectBase 
{
	PlayerStateManager m_stateManager;

	// 描画の為の情報（メッシュに関わる情報）
	CStaticMeshRenderer	m_meshrenderer;
	CStaticMesh			m_mesh;							// メッシュデータ

	// 移動量
	Vector3	m_Move = { 0.0f,0.0f,0.0f };

	// 目標回転角度
	Vector3	m_Destrot = { 0.0f,0.0f,0.0f };

	// 描画の為の情報（見た目に関わる部分）
	CShader	m_shader;	// シェーダ
	CShader m_shadowShader;        // 通常描画用（影あり）
	CShader m_shadowMapShader;     // シャドウマップ生成用

	CarPhysics m_physics;

	float speed;

	// スパイラル降下用の変数
	float m_spiralTime = 0.0f;          // 螺旋アニメーションの経過時間
	float m_countdownTime = 0.0f;       // カウントダウンの経過時間
	int m_countdownNumber = 3;          // 表示するカウント数

	Vector3 m_spiralStartPos;           // 螺旋開始位置
	Vector3 m_spiralTargetPos;          // 螺旋終了位置（スタート地点）
	float m_spiralInitialYaw = 0.0f;    // ★開始時の正面方向（Y軸回転）

	float m_spiralRadius = 15.0f;       // 螺旋の半径
	float m_spiralHeight = 50.0f;       // 螺旋の高さ
	float m_spiralDuration = 2.0f;      // 螺旋降下にかかる時間（秒）
	float m_spiralRotations = 2.0f;     // 螺旋の回転数

	// 少し機械っぽいので追加の揺れパラメータ
	float m_spiralWaveIntensity = 0.08f;    // 半径の波打ち強度
	float m_spiralPulseIntensity = 0.05f;   // 脈動の強度
	float m_spiralVerticalWave = 0.8f;      // 上下の揺れ幅
	float m_spiralPitchWave = 0.1f;         // ピッチの揺れ幅
	float m_spiralRollIntensity = 0.2f;     // ロールの傾き強度
	float m_spiralDesiredSpeed = 25.0f; // ★降下速度（調整可能）


	std::unique_ptr<CountdownEffect> m_countdown;
	bool m_countdownStarted;  // カウントダウンが開始されたかのフラグ

	//ゴール演出用
	std::unique_ptr<GoalEffect> m_goalEffect;
	bool m_hasGoaled;
	bool m_goalEffectStarted;
	// ゴールジャンプパラメータ
	float m_goalJumpTime;
	float m_goalJumpDuration;
	Vector3 m_goalStartPos;
	Vector3 m_goalStartRotation;
	Vector3 m_goalDirection;
	float m_goalJumpHeight;      // 最大高度
	float m_goalJumpDistance;    // 前方距離
	float m_goalRotationSpeed;   // 回転速度
	float m_goalAcceleration;    // 加速度（空へ飛んでいく）




	// 重力関連の変数
	float m_gravity = -0.5f;          // 重力の強さ（負の値）
	float m_verticalVelocity = 0.0f;  // Y軸方向の速度
	bool m_isGrounded = false;        // 地面に接触しているか
	float m_groundCheckDistance = 2.0f; // 地面チェックの距離

	// 車の物理パラメータ
	float m_Acceleration = 0.1f;      // 加速度
	float m_MaxSpeed = 2.0f;          // 最高速度(ブースト時は2.6)
	float m_BoostRatio = 1.3f;          // ブースト時の最高速度倍率
	float m_Deceleration = 0.95f;     // 減速率
	float m_TurnSpeed = 0.02f;        // 旋回速度
	float m_GripFactor = 0.9f;        // グリップ力（1.0で完全グリップ、0.0で完全ドリフト）
	float m_DriftGripFactor = 0.3f;   // ドリフト時のグリップ力
	float m_DriftTurnSpeed = 0.05f;   // ドリフト時の旋回速度

	// 加速ゲージ関連
	float m_BoostGauge = 0.0f;           // 加速ゲージ（0-100）
	bool m_IsBoosting = false;           // ブースト中かどうか
	float m_BoostConsumption = 15.0f;    // ブーストの消費量（per second）
	float m_BoostPower = 1.5f;           // ブーストの力
	float m_MaxBoostGauge = 100.0f;      // ゲージの最大値

	bool m_IsMaxSpeed=false;


	// 滑らかな地形追従用の変数
	float m_targetHeight = 0.0f;        // 目標の高さ
	float m_heightLerpSpeed = 0.2f;    // 高さの補間速度
	bool m_smoothHeightTransition = true; // 滑らかな高さ遷移を有効にするか
	float m_slopeThreshold = 0.995f;      // 急な坂の判定閾値

	// 速度システム
	float m_PermanentSpeedBonus = 1.0f;     // 永続的な速度ボーナス（初期値1.0 = 100%）
	float m_TemporarySpeedBonus = 1.0f;     // 一時的な速度ボーナス（初期値1.0 = 100%）
	float m_SpeedBoostTimer = 0.0f;         // 一時ブーストの残り時間
	int m_ConsecutiveHits = 0;              // 連続ヒット数（敵を倒した総数）

	// 調整用パラメータ
	const float m_PermanentBonusPerMilestone = 0.05f;  // マイルストーン達成ごとに5%アップ
	const int m_HitsPerMilestone = 10;                 // 10体ごとにマイルストーン
	const float m_TemporaryBonusPerHit = 0.15f;        // 1体倒すごとに15%アップ（一時）
	const float m_MaxTemporaryBonus = 2.0f;            // 一時ボーナスの最大値（2.0 = 200%）
	const float m_TemporaryBoostDuration = 5.0f;       // 一時ブーストの持続時間（秒）
	const float m_TemporaryBonusDecaySpeed = 0.05f;    // 一時ボーナスの減衰速度
	const float m_MaxPermanentBonus = 2.0f;            // 永続ボーナスの最大値（2.0 = 200%）

	// 前フレームの情報
	Vector3 m_previousPosition;
	Vector3 m_previousTerrainNormal;
	float m_PreviousTimeScale = 1.0f;

	//ヒットストップ
	float m_HitStopTimer = 0.0f;

	// 速度ベクトル（前回の移動方向を保持）
	Vector3 m_Velocity = Vector3(0.0f, 0.0f, 0.0f);

	// ドリフト状態
	bool m_IsDrifting = false;
	float m_DriftDirection = 0.0f;    // ドリフト方向（-1: 左、1: 右）

	bool m_OnTheGround = false;

	void UpdateDriftMovement(float throttle, float steering, Vector3 forwardDir, Vector3 rightDir, float speedFactor/*,float deltatime*/);
	void UpdateNormalMovement(float throttle, float steering, Vector3 forwardDir, Vector3 rightDir, float speedFactor/*,float deltatime*/);

	void UpdateBoostSystem(bool boostInput, float deltaSeconds);
	void UpdatePositionWithCollisionCheck(float deltaSeconds);

	//速度システム用
	void UpdateSpeedBonusSystem(float deltatime);  // 速度ボーナスシステムの更新
	float GetCurrentSpeedMultiplier() const;        // 現在の速度倍率を取得

	Road* m_road;

	RoadManager* m_roadManager;  // 追加

	// 地形追従用の変数を追加
	Vector3 m_terrainNormal = Vector3(0.0f, 1.0f, 0.0f);  // 地形の法線
	float m_terrainTiltLerpSpeed = 0.1f;  // 傾斜の補間速度

	float testparticle_y = 0.0f;
	float testparticle_x = 0.0f;
	float testparticle_z = 0.0f;

	//道の埋め込み防止
	float m_groundOffset = 0.0f; // 地面補正値

	//坂判定の取得
	float m_slopeDot;           // 坂の方向と移動方向の内積
	float m_slopeAngle;         // 実際の傾斜角度（ラジアン）
	bool m_isOnSlope;           // 坂の上にいるか

	bool m_wasGroundedLastFrame;     // 前フレームで接地していたか
	float m_downhillVelocityDamping = 0.7f; // 下り坂での垂直速度減衰係数
	float m_groundStickForce = 2.0f;        // 地面への吸着力

	std::function<void(bool, float)>m_PostProcessSetter;
	bool m_PlusFov = false;

	//当たり判定の半径
	float m_CollisionRadius = 0.5f;

	bool m_isResultMode = false;



public:
	void Init() override;
	void Update(float) override;
	void Draw() override;
	void SetResultMode(bool isResultMode) { m_isResultMode = isResultMode; }
	void Dispose() override;

	void PlayerDriftUpdate(uint64_t deltatime) {
		m_physics.Update(deltatime);
		m_Position = m_physics.GetPosition();
		m_Rotation = m_physics.GetRotation();
		m_Velocity = m_physics.GetVelocity();
	}

	//最初の演出
	void StartRaceSequence(const Vector3& startPosition);
	void UpdateStartSequence(float deltatime);

	//最後の演出
	void UpdateGoalSequence(float deltatime);

	const CStaticMesh& GetMesh() 
	{
		return m_mesh;
	}
	void SetRoad(Road* road)
	{
		m_road = road;
	};
	void SetRoadManager(RoadManager* roadManager) { m_roadManager = roadManager; }
	RoadManager* GetRoadManager() const { return m_roadManager; };
	//追加
	void OnCollisionWithEnemy(Enemy&);//敵を取得してベクトルを計算する


	// 地形の法線から車の回転を計算するメソッドを追加
	void UpdateCarRotationFromTerrain(const Vector3& terrainNormal);

	// 重力とグランド判定用のメソッドを追加
	void ApplyGravity(float deltatime);
	void SetGrounded(bool grounded) { m_isGrounded = grounded; }
	bool IsGrounded() const { return m_isGrounded; }
	void ResetVerticalVelocity() { m_verticalVelocity = 0.0f; }
	float GetVerticalVelocity() const { return m_verticalVelocity; }
	void UpdateSmoothTerrainFollowing(float deltatime);
	bool IsOnSlope() const;

	// 路面効果を適用する関数（のちのち追加するならここから追加してください）
	void ApplyRoadSurfaceEffect(RoadType surfaceType, float deltatime);

	// アイテム取得時に呼び出す関数
	void AddBoostGauge(float amount);

	float GetSpiralTime() const { return m_spiralTime; }
    float GetSpiralDuration() const { return m_spiralDuration; }

	// ブーストゲージ関連のゲッター
	float GetBoostGauge() const { return m_BoostGauge; }
	void SetBoostGauge(float value)
	{
		m_BoostGauge += value;
		if (m_BoostGauge > m_MaxBoostGauge) {
			m_BoostGauge = m_MaxBoostGauge;
		}
	}
	bool IsBoosting() const { return m_IsBoosting; }
	Vector3 GetVelocity() const { return m_Velocity; }
	void SetVelocity(Vector3 num) { m_Velocity = num; }
	void ApplyHitStop(float,float);

	//ゴール関連のメソッドを追加
	void OnGoal();                      // ゴール時に呼ばれる


	float GetGroundSlope() const;

	// 車の物理パラメータを取得
	SparkEmitter m_sparkEmitter;

	// Lerp関数が存在しない場合は追加
	float Lerp(float a, float b, float t) {
		return a + (b - a) * t;
	}
	Vector3 Lerp3(const Vector3& a, const Vector3& b, float t)
	{
		return Vector3(
			a.x + (b.x - a.x) * t,
			a.y + (b.y - a.y) * t,
			a.z + (b.z - a.z) * t
		);
	}

	float EaseInOutCubic(float t)
	{
           return t < 0.5f ? 4.0f * t * t * t : 1.0f - pow(-2.0f * t + 2.0f, 3.0f) / 2.0f;
	};

	float GetSpeed() { return speed; }
	float GetMaxSpeed() { return m_MaxSpeed * m_BoostRatio; }
	float GetNormalSpeed() { return m_MaxSpeed; }

	bool GetIsMaxSpeed() { return m_IsMaxSpeed; }

	const PlayerStateManager& GetStateManager() const { return m_stateManager; }

	//最初の演出系
	int GetCountdownNumber() const { return m_countdownNumber; }
	float GetCountdownProgress() const { return m_countdownTime; }
	
	void SetPostProcessSetter(std::function<void(bool, float)> setter) 
	{
		m_PostProcessSetter = setter;
	}

	bool GetOnGoal() { return m_hasGoaled; }

	void DebugGravitySystem_()
	{
		if (ImGui::TreeNode("Gravity System")) {
			ImGui::SliderFloat("Gravity", &m_gravity, -2.0f, 0.0f);
			ImGui::Text("Vertical Velocity: %.3f", m_verticalVelocity);
			ImGui::Text("Is Grounded: %s", m_isGrounded ? "Yes" : "No");
			ImGui::Text("Position Y: %.3f", m_Position.y);

			if (ImGui::Button("Reset Position")) {
				m_Position = Vector3(0.0f, 10.0f, 0.0f); // 空中に配置
				m_verticalVelocity = 0.0f;
				m_isGrounded = false;
			}

			if (ImGui::Button("Jump")) {
				if (m_isGrounded) {	
					m_verticalVelocity = 3.0f; // ジャンプ力
					m_isGrounded = false;
				}
			}

			ImGui::TreePop();
		}
	}
};	