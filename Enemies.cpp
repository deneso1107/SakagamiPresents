#include "system/commontypes.h"
#include <vector>
#include <memory>
#include <random>
#include "system/CStaticMesh.h"
#include "system/CStaticMeshRenderer.h"
#include "system/CShader.h"
#include "Enemies.h"

// 敵多数
std::vector<std::unique_ptr<Enemy>> g_Enemies;

// 敵のメッシュデータ
static CStaticMesh g_EnemyMesh{};
static CStaticMeshRenderer g_EnemyMeshRenderer{};

// シェーダー
static CShader g_Shader{};	

// メッシュ
CStaticMesh* GetEnemyMesh() 
{
	return &g_EnemyMesh;
}

// 既存のランダム配置版（互換性維持）
void InitEnemies(IScene* currentscene, Field* field)
{
	FormationConfig config;
	config.formation = EnemyFormation::RANDOM;
	InitEnemiesWithFormation(currentscene, field, config);
}

// 配置パターン指定版
void InitEnemiesWithFormation(IScene* currentscene, Field* field, const FormationConfig& config)
{
	std::mt19937 mt{ std::random_device{}() };
	std::uniform_real_distribution<float> posdist{ -500.0f, 500.0f };
	std::uniform_real_distribution<float> rotdist{ 0.0f, PI };

	// モデルの初期化
	g_EnemyMesh.Load(
		"assets/model/car001.x",
		"assets/model/");
	// レンダラ初期化
	g_EnemyMeshRenderer.Init(g_EnemyMesh);
	// シェーダーの初期化
	g_Shader.Create(
		"shader/vertexLightingVS.hlsl",
		"shader/vertexLightingPS.hlsl");

	for (int i = 0; i < ENEMYMAX; i++)
	{
		std::unique_ptr<Enemy> enemy = std::make_unique<Enemy>(currentscene);
		enemy->Init();

		Vector3 pos;
		Vector3 rot(0.0f, 0.0f, 0.0f);

		// 配置パターンによって位置を決定
		switch (config.formation)
		{
		case EnemyFormation::RANDOM:
			// ランダム配置
			pos = Vector3(posdist(mt), 0.0f, posdist(mt));
			rot.y = rotdist(mt);
			break;

		case EnemyFormation::LINE:
			// 縦列配置（Z軸方向に並べる）
			pos = config.centerPos;
			pos.z += i * config.spacing;
			// プレイヤー方向を向くように（仮にZ+方向）
			rot.y = 0.0f;
			break;

		case EnemyFormation::CIRCLE:
		{
			// 円形配置
			float angle = (2.0f * PI * i) / ENEMYMAX;
			pos = config.centerPos;
			pos.x += cos(angle) * config.circleRadius;
			pos.z += sin(angle) * config.circleRadius;
			// 中心を向くように回転
			rot.y = angle + PI;
		}
		break;

		//case EnemyFormation::GRID:
		//{
		//	// グリッド配置
		//	int row = i / config.columns;
		//	int col = i % config.columns;
		//	pos = config.centerPos;
		//	pos.x += (col - config.columns / 2.0f) * config.spacing;
		//	pos.z += row * config.spacing;
		//	rot.y = 0.0f;
		//}
		//break;

		//case EnemyFormation::DOUBLE_LINE:
		//{
		//	// 2列配置
		//	int row = i % 2;  // 0 or 1
		//	int index = i / 2;
		//	pos = config.centerPos;
		//	pos.x += (row == 0 ? -config.spacing * 0.5f : config.spacing * 0.5f);
		//	pos.z += index * config.spacing;
		//	rot.y = 0.0f;
		//}
		//break;
		}

		enemy->SetPosition(pos);
		enemy->SetRotation(rot);
		enemy->SetMeshRenderer(&g_EnemyMeshRenderer);
		enemy->SetField(field);
		g_Enemies.emplace_back(std::move(enemy));
	}
}

//void InitEnemies(IScene* currentscene,Field* field)
//{
//	std::mt19937 mt{ std::random_device{}() };
//	std::uniform_real_distribution<float> posdist{ -500.0f, 500.0f };
//
//	std::uniform_real_distribution<float> rotdist{ 0.0f, PI };
//
//	// モデルの初期化
//	g_EnemyMesh.Load(
//		"assets/model/car001.x",
//		"assets/model/");
//
//	// レンダラ初期化
//	g_EnemyMeshRenderer.Init(g_EnemyMesh);
//
//	// シェーダーの初期化
//	g_Shader.Create(
//		"shader/vertexLightingVS.hlsl",			// 頂点シェーダー
//		"shader/vertexLightingPS.hlsl");		// ピクセルシェーダー
//
//	for(int i = 0; i < ENEMYMAX; i++)
//	{
//		std::unique_ptr<Enemy> enemy = std::make_unique<Enemy>(currentscene);
//		enemy->Init();
//		enemy->SetPosition(Vector3(posdist(mt), 0.0f, posdist(mt)));
//		enemy->SetRotation(Vector3(0.0, rotdist(mt), 0.0));
//		enemy->SetMeshRenderer(&g_EnemyMeshRenderer);
//		enemy->SetField(field); // Enemy に渡す
//		g_Enemies.emplace_back(std::move(enemy));
//	}
//
//}

void UpdateEnemies(float deltaTime)
{
	for (auto& e : g_Enemies)
	{
		e->Update(deltaTime);
	}
}

void DrawEnemies() 
{

	g_Shader.SetGPU();

	for (auto& e:g_Enemies)
	{
		if (e->GetActive())
		{
			e->Draw();
		}
	}
}

void DisposeEnemies() 
{

}

std::vector<SRT> GetAllRTS() 
{
	std::vector<SRT> allrts;
	for (auto& e : g_Enemies)
	{
		SRT r;
		r.pos = e->GetPosition();
		r.rot = e->GetRotation();
		r.scale = e->GetScale();
		allrts.emplace_back(r);
	}
	return allrts;
}

std::vector<std::unique_ptr<Enemy>>& GetAllEnemys()
{
	return g_Enemies; // コピーではなく参照を返すのでOK
}

GM31::GE::Collision::BoundingSphere GetEnemyBoundingSphere(const Enemy& enemy)//Enemyの当たり判定を取得する関数
{
	GM31::GE::Collision::BoundingSphere sphere;
	sphere.center = enemy.GetPosition();
	sphere.radius = sphere.center.x; // X座標を半径として使用(カスコード)
	return sphere;
}