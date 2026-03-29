#pragma once

#include "system/IScene.h"
#include "ObjectBase.h"
#include "Field.h"
#include "BaseRoad.h"

// 移動方向の列挙型
enum class MoveDirection
{
    NORTH,  // +Z方向
    SOUTH,  // -Z方向
    EAST,   // +X方向
    WEST    // -X方向
};

class WeavingEnemy : public ObjectBase
{
public:
    // ────────────── 調整パラメータ ──────────────
    static constexpr float DEFAULT_MOVE_SPEED = 200.0f;   // 前進速度
    static constexpr float DEFAULT_WEAVE_AMPLITUDE = 100.0f;  // 左右の振れ幅
    static constexpr float DEFAULT_WEAVE_FREQUENCY = 1.5f;   // 左右の周波数（Hz）
    static constexpr float DEFAULT_RISE_SPEED = 200.0f;  // 上昇速度
    static constexpr float DEFAULT_TRAVEL_LIMIT = 2000.0f; // 上昇トリガーとなる移動距離
    static constexpr float DEFAULT_TIME_LIMIT = 1.5f;   // 上昇トリガーとなる時間（秒）
    static constexpr float DEFAULT_BOUNDING_RADIUS = 10.0f;  // 当たり判定半径

private:
    // ────────────── 内部状態 ──────────────

    // 移動フェーズ
    enum class Phase
    {
        WEAVING,   // 左右に揺れながら前進
        RISING     // 上昇して消える
    };

    CStaticMeshRenderer* m_StaticMeshRenderer = nullptr;
    IScene* m_ownerscene = nullptr;
    Field* m_field = nullptr;

    MoveDirection m_direction = MoveDirection::NORTH;
    Phase         m_phase = Phase::WEAVING;

    // サイン波用タイマー
    float m_waveTimer = 0.0f;

    // 経過時間 / 移動距離トラッキング
    float m_elapsedTime = 0.0f;
    float m_traveledDistance = 0.0f;
    Vector3 m_startPosition = { 0.0f, 0.0f, 0.0f };

    // パラメータ（実行時変更可能）
    float m_moveSpeed = DEFAULT_MOVE_SPEED;
    float m_weaveAmplitude = DEFAULT_WEAVE_AMPLITUDE;
    float m_weaveFrequency = DEFAULT_WEAVE_FREQUENCY;
    float m_riseSpeed = DEFAULT_RISE_SPEED;
    float m_travelLimit = DEFAULT_TRAVEL_LIMIT;
    float m_timeLimit = DEFAULT_TIME_LIMIT;

    float m_gravity = -9.8f;
    float m_verticalVelocity = 0.0f;
    bool  m_onField = false;
    bool  m_isActive = true;

    float m_BoundingSphereRadius = DEFAULT_BOUNDING_RADIUS;

    // ノックバック（既存Enemyと同仕様）
    bool  m_isKnockedBack = false;
    float m_knockbackTimer = 0.0f;
    float m_rotateSpeed = 100.0f;
    Vector3 m_knockbackMove = { 0.0f, 0.0f, 0.0f };

    bool m_disappearEffectSpawned = false;

    bool       m_isActivated = false;  // 動き出したかどうか
    BaseRoad* m_linkedRoad = nullptr; // 紐づいている道路

    // ────────────── 内部ヘルパー ──────────────

    // 方向ベクトルを取得（前進方向）
    Vector3 GetForwardVector() const;

    // 方向に対して垂直な横ベクトルを取得（左右揺れ用）
    Vector3 GetSideVector() const;

    void SpawnDisappearEffect();
    void KnockBack(float deltaTime);

	static constexpr float COLLISION_SIZE = 20.0f; // 当たり判定のサイズ（半径）
	static constexpr float MODEL_SIZE = 15.0f; // 当たり判定のサイズ（半径）

public:
    WeavingEnemy(IScene* currentscene)
        : m_ownerscene(currentscene)
    {
    }

    void Init()    override;
    void Update(float deltaTime) override;
    void Draw()    override;
    void Dispose() override;

    // ────────────── Setter / Getter ──────────────

    void SetMeshRenderer(CStaticMeshRenderer* renderer) { m_StaticMeshRenderer = renderer; }
    void SetField(Field* field) { m_field = field; }

    void SetMoveDirection(MoveDirection dir) { m_direction = dir; }
    void SetMoveSpeed(float speed) { m_moveSpeed = speed; }
    void SetWeaveAmplitude(float amp) { m_weaveAmplitude = amp; }
    void SetWeaveFrequency(float freq) { m_weaveFrequency = freq; }
    void SetRiseSpeed(float speed) { m_riseSpeed = speed; }
    void SetTravelLimit(float dist) { m_travelLimit = dist; }
    void SetTimeLimit(float time) { m_timeLimit = time; }

    void SetActive(bool active) { m_isActive = active; }
    bool GetActive() const { return m_isActive; }
    BaseRoad* GetLinkedRoad() const { return m_linkedRoad; }

    void ApplyGravity(uint64_t deltatime);
    void ApplyKnockback(Vector3 direction, float force, float timeScale);

    void SetLinkedRoad(BaseRoad* road) { m_linkedRoad = road; }
    void ActivateMovement() { m_isActivated = true; }
    bool IsActivated() const { return m_isActivated; }
    bool GetIsKnockedBack() const { return m_isKnockedBack; }


    GM31::GE::Collision::BoundingSphere GetEnemyBoundingSphere();
};