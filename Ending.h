#pragma once
#include "system/IScene.h"
#include "scenemanager.h"
#include <string>
#include"ScreenFixedBillboard.h"
class Ending : public IScene
{
public:
	void update(float deltatime) override;
	void draw(uint64_t deltatime) override;
	void init() override;
	void loadAsync() override;
	void dispose() override;
private:
	ScreenFixedBillboard* m_screenBillboard;//ÉQÉbÉ^Å[Ç†ÇÈÇÊ
};
