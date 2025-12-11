#include "Title.h"
#include "system/CDirectInput.h"

void Title::init()
{
	// スプライトの初期化
	m_screenBillboard = new ScreenFixedBillboard(Vector2(0.5f, 0.5f), 0.15f, 0.15f, L"assets/texture/scene/Title.png");
	m_turtrialBillboard = new ScreenFixedBillboard(Vector2(0.5f, 0.5f), 1.0f, 1.0f, L"assets/texture/scene/Turtrial.png");

	m_VideoBB = ScreenFixedBillboard::CreateFromVideo(
		Vector2(0.5f, 0.5f),
		1.0f, 1.0f,
		L"assets/video/無題の動画 ‐ Clipchampで作成.mp4"
	);
	if (m_VideoBB == nullptr) {
		MessageBoxA(nullptr, "動画の読み込みに失敗しました", "Error", MB_OK);
	}
	else {
		// 動画情報を確認
		VideoPlayer* player = m_VideoBB->GetVideoPlayer();
		if (player) {
			char msg[256];
			printf("動画サイズ: %dx%d\n長さ: %.2f秒",
				player->GetWidth(),
				player->GetHeight(),
				player->GetDurationSeconds());
			MessageBoxA(nullptr, "Video Info","OK", MB_OK);
		}
	}
	m_VideoBB->SetLooping(true);
	m_VideoBB->PlayVideo();
}

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
	m_VideoBB->Update();
	if (m_VideoBB && m_VideoBB->GetVideoPlayer()) {
		if (!m_VideoBB->GetVideoPlayer()->IsValid()) {
			OutputDebugStringA("VideoPlayer が無効です\n");
		}
	}
}
void Title::draw(uint64_t deltatime)
{
	m_VideoBB->Draw();
	//if(changepic)
//{
//	m_turtrialBillboard->Draw();
//	return;
//}
//else
//{
//	m_screenBillboard->Draw();
//}
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