#pragma once
#include"ObjectBase.h"
#include "system/IScene.h"
#include"Player.h"
class BoostItem:public ObjectBase
{
public:
	// 描画の為の情報（メッシュに関わる情報）
	CStaticMeshRenderer m_meshrenderer;
	CStaticMesh         m_mesh;                         // メッシュデータ
	// 描画の為の情報（見た目に関わる部分）
	CShader m_shader;   // シェーダ

	// アイテムの状態管理
	bool m_isActive = true;        // アクティブ状態
	bool m_isCollected = false;    // 取得済みフラグ

	void Init() override;
	void Draw() override;
	void Dispose() override {}
	void Update(float deltaTime) override;

	void CollectItem();  // アイテム取得処理
	bool IsActive() const { return m_isActive; }
	bool IsCollected() const { return m_isCollected; }

	void PlayerBoostGauge(Player*);
	// オーナーSCENE
	IScene* m_ownerscene = nullptr;
};

