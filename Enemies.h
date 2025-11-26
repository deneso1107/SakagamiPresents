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
    int enemyCount = 0;          // この配置パターンで生成する敵の数（0なら残り全部）


    FormationConfig() = default;

    // コンストラクタ
    FormationConfig(EnemyFormation form, int count, Vector3 center = Vector3(0.0f, 0.0f, 0.0f))
        : formation(form), enemyCount(count), centerPos(center) {
    }
};

// 複数配置パターンをまとめた構造体
struct MultiFormationConfig
{
    std::vector<FormationConfig> formations;
    int totalEnemyCount = ENEMYMAX;  // 生成する敵の総数

    // 配置パターンを追加
    void AddFormation(const FormationConfig& config)
    {
        formations.push_back(config);
    }

    // バリデーション（配置数の合計チェック）
    bool Validate() const
    {
        int specified = 0;
        for (const auto& config : formations)
        {
            specified += config.enemyCount;
        }
        // 指定された数が総数を超えていないかチェック
        return specified <= totalEnemyCount;
    }
};

void InitEnemies(IScene*,Field*);
void InitEnemiesWithFormation(IScene*,Field*, const FormationConfig&);
void UpdateEnemies(float);
void DrawEnemies();
void DisposeEnemies();
void InitEnemiesWithMultiFormation(IScene*, Field*, const MultiFormationConfig&);

// メッシュ取得
CStaticMesh* GetEnemyMesh();

// 全RTS情報取得
std::vector<std::unique_ptr<Enemy>>& GetAllEnemys();
std::vector<SRT> GetAllRTS();
GM31::GE::Collision::BoundingSphere GetEnemyBoundingSphere(const Enemy& enemy);
