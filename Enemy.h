#pragma once

#include "system/IScene.h"
#include"ObjectBase.h"
#include"Field.h"
class Enemy :public ObjectBase 
{
private:
	static constexpr	float RATE_ROTATE_ENEMY = 0.01f;		// 回転係数
	static constexpr	float RATE_MOVE_MODEL = 0.20f;			// 移動慣性係数

	// 描画の為の情報（メッシュに関わる情報）
	CStaticMeshRenderer* m_StaticMeshRenderer;

	// 目標回転角度
	Vector3	m_Destrot = { 0.0f,0.0f,0.0f };

	// オーナーSCENE
	IScene* m_ownerscene = nullptr;

	// 移動量
	Vector3 m_Move = Vector3(0.0f, 0.0f, 0.0f);

	// スピード
	float m_speed = 0.1f;	// 移動速度

	float m_KnockbackTimer=0.0f;    // ノックバック残り時間
	float m_IsKnockbackTimer=0.0f;//当たった瞬間の時間を取得

	float m_gravity = -9.8f;          // 重力の強さ（負の値）
	float m_verticalVelocity = 0.0f;  // Y軸方向の速度
	float m_RotateSpeed = 100.0f;

	Field* m_field = nullptr;

	bool onField = false;
	bool m_EffectSpawned = false;
	bool m_IsActive=true;//敵が生きているかどうか
	bool m_DisappearEffectSpawned = false;//エフェクト生成済みフラグ
	void SpawnDisappearEffect();

public:

	bool m_IsKnockedBack;      // ノックバック中かどうか
	Enemy(IScene* currentscene)
		: m_StaticMeshRenderer(nullptr),
		m_ownerscene(currentscene) {
	}


	void Init() override;
	void Update(float) override;
	void Draw()override;
	void Dispose()override;

	void ApplyKnockback(Vector3, float, float);
	void KnockBack(float);


	void SetMeshRenderer(CStaticMeshRenderer* renderer) { m_StaticMeshRenderer = renderer; }

	void ApplyGravity(uint64_t deltatime);
	void  SetActive(bool isActive) { m_IsActive = isActive; }
	bool  GetActive() const { return m_IsActive; }

	// Fieldのポインタを受け取る（nullptrチェックは呼び出し元が責任を持つ）
	void SetField( Field* field) {m_field = field;}

	GM31::GE::Collision::BoundingSphere GetEnemyBoundingSphere();///当たり判定を取得する関数
};

//class Enemy :public ObjectBase
//{
//	static constexpr	float RATE_ROTATE_ENEMY = 0.01f;		// 回転係数
//	static constexpr	float RATE_MOVE_MODEL = 0.20f;			// 移動慣性係数
//
//	// SRT情報（姿勢情報）
//	Vector3	m_Position = Vector3(0.0f, 0.0f, 0.0f);
//	Vector3	m_Rotation = Vector3(0.0f, 0.0f, 0.0f);
//	Vector3	m_Scale = Vector3(1.0f, 1.0f, 1.0f);
//
//	// 描画の為の情報（メッシュに関わる情報）
//	CStaticMeshRenderer* m_StaticMeshRenderer;
//
//	// 目標回転角度
//	Vector3	m_Destrot = { 0.0f,0.0f,0.0f };
//
//	// オーナーSCENE
//	IScene* m_ownerscene = nullptr;
//
//	// 移動量
//	Vector3 m_Move = Vector3(0.0f, 0.0f, 0.0f);
//
//	// スピード
//	float m_speed = 0.1f;	// 移動速度
//
//
//	float m_KnockbackTimer;    // ノックバック残り時間
//	float m_IsKnockbackTimer;//当たった瞬間の時間を取得
//
//	GM31::GE::Collision::BoundingSphere m_EnemySquare;//敵の当たり判定
//public:
//
//	bool m_IsKnockedBack;      // ノックバック中かどうか
//
//	Enemy(IScene* currentscene)
//		: m_StaticMeshRenderer(nullptr),
//		m_ownerscene(currentscene) {
//	}
//
//	void Init();
//	void Update(uint64_t);
//	void Draw();
//	void Dispose();
//
//
//	void ApplyKnockback(Vector3, float, uint64_t);
//	void KnockBack(float);
//
//	void SetPosition(Vector3 m_ParticlePos) { m_Position = m_ParticlePos; }
//	void SetRotation(Vector3 rot) { m_Rotation = rot; }
//	void SetScale(Vector3 scale) { m_Scale = scale; }
//
//	void SetMeshRenderer(CStaticMeshRenderer* renderer) { m_StaticMeshRenderer = renderer; }
//
//	Vector3 GetPosition() const { return m_Position; }
//	Vector3 GetRotation() const { return m_Rotation; }
//	Vector3 GetScale() const { return m_Scale; }
//	GM31::GE::Collision::BoundingSphere GetEnemyBoundingSphere();///当たり判定を取得する関数
//};