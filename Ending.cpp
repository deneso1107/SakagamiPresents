#include "Ending.h"
#include "system/CDirectInput.h"
void Ending::update(float deltatime)
{
	if (CDirectInput::GetInstance().CheckKeyBuffer(DIK_SPACE))
	{
		SceneManager::ChangeScene("Title");
	}
}
void Ending::draw(uint64_t deltatime)
{
	
	m_screenBillboard->Draw();
}
void Ending::init()
{
	// スプライトの初期化
	m_screenBillboard = new ScreenFixedBillboard(Vector2(0.5f, 0.5f), 1.15f, 1.15f, L"assets/texture/scene/Ending.png");
}
void Ending::loadAsync()
{

}
void  Ending::dispose()
{
	if (m_screenBillboard)
	{
		delete m_screenBillboard;
		m_screenBillboard = nullptr;
	}
}