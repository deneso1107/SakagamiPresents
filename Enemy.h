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
	CStaticMeshRenderer* m_staticMeshRenderer;

	// 目標回転角度
	Vector3	m_destrot = { 0.0f,0.0f,0.0f };

	// オーナーSCENE
	IScene* m_ownerscene = nullptr;

	// 移動量
	Vector3 m_move = Vector3(0.0f, 0.0f, 0.0f);

	// スピード
	float m_speed = 0.1f;	// 移動速度

	float m_knockbackTimer=0.0f;    // ノックバック残り時間
	float m_isKnockbackTimer=0.0f;//当たった瞬間の時間を取得

	float m_gravity = -9.8f;          // 重力の強さ（負の値）
	float m_verticalVelocity = 0.0f;  // Y軸方向の速度
	float m_rotateSpeed = 100.0f;

	float m_boundingSphereRadius = 15.0f; // 当たり判定の半径

	Field* m_field = nullptr;

	bool m_onField = false;
	bool m_effectSpawned = false;
	bool m_isActive=true;//敵が生きているかどうか
	bool m_disappearEffectSpawned = false;//エフェクト生成済みフラグ
	void SpawnDisappearEffect();

public:

	bool m_isKnockedBack=false;      // ノックバック中かどうか
	Enemy(IScene* currentscene)
		: m_staticMeshRenderer(nullptr),
		m_ownerscene(currentscene)
	{
	}


	void Init() override;
	void Update(float) override;
	void Draw()override;
	void Dispose()override;

	void ApplyKnockback(Vector3, float, float);
	void KnockBack(float);


	void SetMeshRenderer(CStaticMeshRenderer* renderer) { m_staticMeshRenderer = renderer; }

	void ApplyGravity(uint64_t deltatime);
	void  SetActive(bool isActive) { m_isActive = isActive; }
	bool  GetActive() const { return m_isActive; }

	// Fieldのポインタを受け取る（nullptrチェックは呼び出し元が責任を持つ）
	void SetField( Field* field) {m_field = field;}

	GM31::GE::Collision::BoundingSphere GetEnemyBoundingSphere();///当たり判定を取得する関数
};