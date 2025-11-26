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