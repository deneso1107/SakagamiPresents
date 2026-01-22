#pragma once
#include "system/IScene.h"
#include "scenemanager.h"
#include <string>
#include"ScreenFixedBillboard.h"
#include"VideoPlayer.h"

#include"Player.h"
#include"TitleMove.h"
#include"Skydome.h"
#include"SparkEmitter.h"
class Title : public IScene
{
public:
	void update(float deltatime) override;
	void draw(float deltatime) override;
	void init() override;
	void loadAsync() override;	
	void dispose() override;
	bool changepic=false;//カスコード
	//void ChangeScene(const std::string& sceneName,bool)
	//{
	//	SceneManager::ChangeScene(sceneName);
	//}
private:
	ScreenFixedBillboard* m_screenBillboard;//ゲッターあるよ
	ScreenFixedBillboard* m_TitleBillboard;//ゲッターあるよ
	VideoPlayer m_videoPlayer;
	ScreenFixedBillboard* m_VideoBB;

	std::unique_ptr<Player> m_player;// プレイヤー
	std::unique_ptr<TitleSpiralEffect> m_spiralEffect;// 螺旋エフェクト
	std::unique_ptr<Skydome> m_skydome;//スカイドーム
	std::unique_ptr<SparkEmitter> m_sparkEmitter;//火花エミッタ

	float m_titlePosY;          // タイトルの現在のY座標
	float m_targetPosY;         // タイトルの目標Y座標
	float m_titleVelocityY;     // タイトルの落下速度
	bool m_isAnimating;         // アニメーション中かどうか

};

