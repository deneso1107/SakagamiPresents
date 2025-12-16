#pragma once
#include "system/IScene.h"
#include "scenemanager.h"
#include <string>
#include"ScreenFixedBillboard.h"
#include"VideoPlayer.h"
class Title : public IScene
{
public:
	void update(float deltatime) override;
	void draw(uint64_t deltatime) override;
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
	ScreenFixedBillboard* m_turtrialBillboard;//ゲッターあるよ
	VideoPlayer m_videoPlayer;
	ScreenFixedBillboard* m_VideoBB;

};

