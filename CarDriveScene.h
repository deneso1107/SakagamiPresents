#pragma once
#include <memory>
#include <array>
#include "system/IScene.h"
#include "system/C3DShape.h"
#include "system/DebugUI.h"
#include "system/CStaticMesh.h"
#include "system/CStaticMeshRenderer.h"
#include "system/CShader.h"
#include "system/collision.h"
#include "camera.h"
#include "Field.h"
#include "Player.h"
#include"Road.h"
#include "Skydome.h"
#include"Start.h"
#include"Goal.h"
#include "system/SphereDrawer.h"
#include"Billboard.h"
#include"ScreenFixedBillBoard.h"
#include"ChaseCamera.h"
#include"ScreenGaugeBillboard.h"
#include"BoostItem.h"
#include"SpeedMator.h"
#include"NumberRenderer.h"
#include"RoadManager.h"
#include"EffeectManager.h"
#include"PostProcessManager.h"
class CarDriveScene : public IScene
{
public:
	explicit CarDriveScene();
	void update(float deltatime) override;//継承！継承
	//void update(float deltatime) override;//継承！継承
	void draw(uint64_t deltatime) override;
	void init() override;
	void dispose() override;

	//IMGUIデバッグ形
	void debugFreeCamera();
	void debugDirectionalLight();
	void debugChangeCamera();
	void debugChangeScene();
	void debugParticlePos();
	Vector3 pos;//パーティクル位置保存用


	void InitPostProcess();// ポストプロセス初期化
	void ApplyPostProcess();
	void ApplyMotionBlur(ID3D11DeviceContext*, ID3D11ShaderResourceView*, ID3D11RenderTargetView*);
	void ApplyChromaticAberration(ID3D11DeviceContext*, ID3D11ShaderResourceView*, ID3D11RenderTargetView*);
	void CreateIntermediateTexture();
	void ChangeScene(const std::string& sceneName)
	{
		SceneManager::ChangeScene(sceneName);
	}

	bool m_enableMotionBlur = true;
	bool m_enableChromaticAberration = false;
	bool m_enableShockwave = false;
	Player* GetPlayer() const { return m_player.get(); }		// プレイヤの取得
private:

	// ポストプロセス用メンバ変数を追加
	ID3D11Texture2D* m_sceneTexture = nullptr;
	ID3D11RenderTargetView* m_sceneRTV = nullptr;
	ID3D11ShaderResourceView* m_sceneSRV = nullptr;

	ID3D11Texture2D* m_sceneDepthTexture = nullptr;
	ID3D11DepthStencilView* m_sceneDSV = nullptr;
	ID3D11RenderTargetView* m_originalBackBuffer = nullptr;

	ID3D11SamplerState* m_sampler = nullptr;

	ID3D11DepthStencilState* m_defaultDepthState = nullptr;


	// クロマティックアバレーション用
	CShader  m_chromaticAberrationShader;
	CShader m_motionBlurShader;

	// 中間テクスチャ（パス間のやり取り用）
	ID3D11Texture2D* m_intermediateTexture;
	ID3D11RenderTargetView* m_intermediateRTV;
	ID3D11ShaderResourceView* m_intermediateSRV;




	//MeshRenderer m_fullscreenQuad;
	float m_blurStrength = 0.0f;
	bool m_usePostProcess = false;
	bool m_NowCamera = true;//初手追尾カメラか全体カメラか
	bool m_Debug = false;
	float m_aberrationStrength = 0.0f;
	float m_LineTime = 0.0f;//シェーダに渡す用
	float m_LineSpeed = 0.0f;//同じく
	float m_ShockWave = 0.0f;//同じく

	float m_shockwaveTime = -1.0f; // 発動した時間（-1は無効）
	float m_shockwaveDuration = 0.25f; // ショックウェーブ継続時間（秒）
	float m_shockwaveIntensity = 0.0f;
	float m_shockwaveProgress = 0.0f;


	float m_PrevBoostGauge = 0.0f;
	float m_time = 0.0f;

	float m_RemainingTime;//タイム三校用
	//ChaseCamera m_CheeseCamera;										// フリーカメラ
	FreeCamera m_FreeCamera;
	ScreenFixedBillboard* m_screenBillboard;//ゲッターあるよ
	ScreenGaugeBillboard* m_Gauge;
	std::array<std::unique_ptr<::Segment>, 3> m_segments;			// ローカル軸表示用線分
	std::unique_ptr<Field> m_field;								// フィールド
	std::unique_ptr<Player> m_player;							// フィールド
	std::unique_ptr<Skydome> m_skydome;							// スカイドーム
	std::unique_ptr<Start> m_start;							// スタート
	std::unique_ptr<Goal> m_goal;							//　ゴール
	std::unique_ptr<BoostItem> m_item;							//　ゴール
	Road* m_road;
	SpeedMator* m_speedMator;
	NumberRenderer m_timeRenderer;
	NumberRenderer m_scoreRenderer;
	SparkEmitter m_sparkEmitter;
	RoadManager roadManager; // 道路サイズを18に設定
};
//このカメラにビルボードを対応させよう