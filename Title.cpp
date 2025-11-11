#include "Title.h"
#include "system/CDirectInput.h"
void Title::update(float deltatime)
{
	if (CDirectInput::GetInstance().CheckKeyBuffer(DIK_SPACE)&&changepic)
	{
		if (CDirectInput::GetInstance().CheckKeyBuffer(DIK_SPACE) && changepic)
		{
			SceneManager::ChangeScene("CarDriveScene",true);
		}
	}
	else
	{
		if (CDirectInput::GetInstance().CheckKeyBuffer(DIK_SPACE) && !changepic)
		{
			changepic = true;
		}
	}
}
void Title::draw(uint64_t deltatime)
{
	if(changepic)
	{
		m_turtrialBillboard->Draw();
		return;
	}
	else
	{
		m_screenBillboard->Draw();
	}
}
void Title::init()
{
	// スプライトの初期化
	m_screenBillboard = new ScreenFixedBillboard(Vector2(0.5f, 0.5f), 1.15f, 1.15f, L"assets/texture/scene/Title.png");
	m_turtrialBillboard = new ScreenFixedBillboard(Vector2(0.5f, 0.5f), 1.0f, 1.0f, L"assets/texture/scene/Turtrial.png");
}
void  Title::dispose()
{
	if (m_screenBillboard)
	{
		delete m_screenBillboard;
		m_screenBillboard = nullptr;
	}
	changepic = false;
}