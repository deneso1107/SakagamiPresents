#pragma once
#include "system/commontypes.h"
#include "system/IScene.h"
#include "Enemy.h"
#include"Field.h"
constexpr int ENEMYMAX = 100;

void InitEnemies(IScene*,Field*);
void UpdateEnemies(float);
void DrawEnemies();
void DisposeEnemies();

// ƒƒbƒVƒ…æ“¾
CStaticMesh* GetEnemyMesh();

// ‘SRTSî•ñæ“¾
std::vector<std::unique_ptr<Enemy>>& GetAllEnemys();
std::vector<SRT> GetAllRTS();
GM31::GE::Collision::BoundingSphere GetEnemyBoundingSphere(const Enemy& enemy);
