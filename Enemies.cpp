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


void InitEnemies(IScene* currentscene,Field* field)
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
		"shader/vertexLightingVS.hlsl",			// 頂点シェーダー
		"shader/vertexLightingPS.hlsl");		// ピクセルシェーダー

	for(int i = 0; i < ENEMYMAX; i++)
	{
		std::unique_ptr<Enemy> enemy = std::make_unique<Enemy>(currentscene);
		enemy->Init();
		enemy->SetPosition(Vector3(posdist(mt), 0.0f, posdist(mt)));
		enemy->SetRotation(Vector3(0.0, rotdist(mt), 0.0));
		enemy->SetMeshRenderer(&g_EnemyMeshRenderer);
		enemy->SetField(field); // Enemy に渡す
		g_Enemies.emplace_back(std::move(enemy));
	}

}

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