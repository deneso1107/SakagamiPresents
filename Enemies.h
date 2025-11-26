#pragma once
#include "system/commontypes.h"
#include "system/IScene.h"
#include "Enemy.h"
#include"Field.h"
constexpr int ENEMYMAX = 100;

enum class EnemyFormation
{
	LINE,//直線状
	CIRCLE,//円形
	RANDOM//今まで通り
};
struct FormationConfig
{
	EnemyFormation formation=EnemyFormation::RANDOM;
	Vector3 centerPos; // フォーメーションの中心位置
	float spacing;          // 敵同士の間隔（LINEの場合）
	float circleRadius;           // 円の半径（CIRCLEの場合）
};
void InitEnemies(IScene*,Field*);
void InitEnemiesWithFormation(IScene*,Field*, const FormationConfig&);
void UpdateEnemies(float);
void DrawEnemies();
void DisposeEnemies();

// メッシュ取得
CStaticMesh* GetEnemyMesh();

// 全RTS情報取得
std::vector<std::unique_ptr<Enemy>>& GetAllEnemys();
std::vector<SRT> GetAllRTS();
GM31::GE::Collision::BoundingSphere GetEnemyBoundingSphere(const Enemy& enemy);
