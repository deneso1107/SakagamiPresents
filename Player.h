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
class Player:public ObjectBase 
{
	PlayerStateManager m_PlayerStateManager;

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


	// 滑らかな地形追従用の変数
	float m_targetHeight = 0.0f;        // 目標の高さ
	float m_heightLerpSpeed = 0.2f;    // 高さの補間速度
	bool m_smoothHeightTransition = true; // 滑らかな高さ遷移を有効にするか
	float m_slopeThreshold = 0.995f;      // 急な坂の判定閾値

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

	void DebugPlayerMoveParameter_();
	void UpdateDriftMovement(float throttle, float steering, Vector3 forwardDir, Vector3 rightDir, float speedFactor/*,float deltatime*/);
	void UpdateNormalMovement(float throttle, float steering, Vector3 forwardDir, Vector3 rightDir, float speedFactor/*,float deltatime*/);

	// ★ここに追加★
	void UpdateBoostSystem(bool boostInput, float deltaSeconds);
	void UpdatePositionWithCollisionCheck(float deltaSeconds);

	Road* m_road;

	RoadManager* m_roadManager;  // 追加

	// 地形追従用の変数を追加
	Vector3 m_terrainNormal = Vector3(0.0f, 1.0f, 0.0f);  // 地形の法線
	float m_terrainTiltLerpSpeed = 0.1f;  // 傾斜の補間速度

	float testparticle_y = 0.0f;
	float testparticle_x = 0.0f;
	float testparticle_z = 0.0f;

	//牛用の変数
	float m_groundOffset = 0.0f; // 地面補正値

	std::function<void(bool, float)>m_PostProcessSetter;
public:
	void Init() override;
	void Update(float) override;
	void Draw() override;
	void Dispose() override;
	void SetTerrain(CTerrainMesh* terrain);

	void PlayerDriftUpdate(uint64_t deltatime) {
		m_physics.Update(deltatime);
		m_Position = m_physics.GetPosition();
		m_Rotation = m_physics.GetRotation();
		m_Velocity = m_physics.GetVelocity();
	}

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
	bool CheckGroundContact();
	void SetGrounded(bool grounded) { m_isGrounded = grounded; }
	bool IsGrounded() const { return m_isGrounded; }
	void ResetVerticalVelocity() { m_verticalVelocity = 0.0f; }
	float GetVerticalVelocity() const { return m_verticalVelocity; }
	void UpdateSmoothTerrainFollowing(uint64_t deltatime);
	bool IsOnSlope() const;

	// 路面効果を適用する関数（のちのち追加するならここから追加してください）
	void ApplyRoadSurfaceEffect(RoadType surfaceType, float deltatime);

	// アイテム取得時に呼び出す関数
	void AddBoostGauge(float amount);

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
	float GetSpeed() { return speed; }
	float GetMaxSpeed() { return m_MaxSpeed * m_BoostRatio; }
	
	void SetPostProcessSetter(std::function<void(bool, float)> setter) 
	{
		m_PostProcessSetter = setter;
	}

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