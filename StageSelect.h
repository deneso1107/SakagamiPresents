#pragma once
#include "system/IScene.h"
#include "StageManager.h"
#include"StageSelectCamera.h"
#include "system/CDirectInput.h"
#include"InputManager.h"
#include"ScreenFixedBillboard.h"
#include"StageUIAnimation.h"
#include"Skydome.h"
#include"ArrowButtonAnimator.h"
#include "StageDecideEffect.h"
#include"Player.h"
class StageSelect : public IScene
{
private:
	void update(float deltatime) override;
	void draw(float deltatime) override;
	void init() override;
	void loadAsync() override;
	void dispose() override;

	StageManager m_stageManager;
	StageSelectCamera m_camera;


	ArrowButtonAnimator m_leftArrow;
	ArrowButtonAnimator m_rightArrow;

	// 矢印ビルボード
	std::unique_ptr<ScreenFixedBillboard> m_leftArrowBillBoard;
	std::unique_ptr<ScreenFixedBillboard> m_rightArrowBillBoard;

	std::unique_ptr<ScreenFixedBillboard> m_stage1BillBoard; // ステージ説明
	std::unique_ptr<ScreenFixedBillboard> m_stage2BillBoard; // ステージ説明
	std::unique_ptr<ScreenFixedBillboard> m_stage3BillBoard; // ステージ説明
	std::unique_ptr<ScreenFixedBillboard> m_selectIconBillBoard; // セレクトアイコン
	std::unique_ptr<SparkEmitter> m_sparkEmitter;//火花エミッタ
	std::unique_ptr<Skydome> m_skydome;//スカイドーム

	StageUIAnimator m_uiAnimator;
	int m_currentBillboardIndex = 0; // 現在表示中のビルボード番号

	StageDecideEffect m_decideEffect;
	bool m_isDeciding = false;
	std::unique_ptr<Player> m_player;// プレイヤー
};