#include "system/commontypes.h"
#include "system/CStaticMesh.h"
#include "system/CStaticMeshRenderer.h"
#include "WeavingEnemy.h"
#include "SpringCamera.h"

// ──────────────────────────────────────────────
//  方向ベクトルの取得
// ──────────────────────────────────────────────

Vector3 WeavingEnemy::GetForwardVector() const
{
    switch (m_direction)
    {
    case MoveDirection::NORTH: return Vector3(0.0f, 0.0f, 1.0f);
    case MoveDirection::SOUTH: return Vector3(0.0f, 0.0f, -1.0f);
    case MoveDirection::EAST:  return Vector3(1.0f, 0.0f, 0.0f);
    case MoveDirection::WEST:  return Vector3(-1.0f, 0.0f, 0.0f);
    default:                   return Vector3(0.0f, 0.0f, 1.0f);
    }
}

// 前進方向に対して右90度の横ベクトル（サイン波の揺れ方向）
Vector3 WeavingEnemy::GetSideVector() const
{
    switch (m_direction)
    {
    case MoveDirection::NORTH: return Vector3(1.0f, 0.0f, 0.0f); // +X
    case MoveDirection::SOUTH: return Vector3(-1.0f, 0.0f, 0.0f); // -X
    case MoveDirection::EAST:  return Vector3(0.0f, 0.0f, -1.0f); // -Z
    case MoveDirection::WEST:  return Vector3(0.0f, 0.0f, 1.0f); // +Z
    default:                   return Vector3(1.0f, 0.0f, 0.0f);
    }
}

// ──────────────────────────────────────────────
//  Init
// ──────────────────────────────────────────────

void WeavingEnemy::Init()
{
    m_Rotation = Vector3(0.0f, 0.0f, 0.0f);
    m_Scale = Vector3(MODEL_SIZE, MODEL_SIZE, MODEL_SIZE);

    m_startPosition = m_Position;  // SetPosition後にInitを呼ぶこと
    m_phase = Phase::WEAVING;
    m_waveTimer = 0.0f;
    m_elapsedTime = 0.0f;
    m_traveledDistance = 0.0f;
    m_isActive = true;
    m_isKnockedBack = false;
    m_verticalVelocity = 0.0f;
    m_onField = false;

    m_BoundingSphere =
    {
        m_Position,
       COLLISION_SIZE ,
    };
}

// ──────────────────────────────────────────────
//  Update
// ──────────────────────────────────────────────

void WeavingEnemy::Update(float deltaTime)
{
    if (!m_isActive) return;

    // ── 起動チェック（未起動の場合はここで止まる） ──
    if (!m_isActivated) return;  // 起動されるまで何もしない

    // ── ノックバック中は既存Enemyと同じ処理 ──
    if (m_isKnockedBack)
    {
        KnockBack(deltaTime);
        return;
    }

    // ── フィールド衝突 & 着地補正 ──
    if (m_field && GM31::GE::Collision::CollisionSphereAABB(
        this->GetCollision(), m_field->GetFieldCollision()))
    {
        Vector3 pos = GetPosition();
        float fieldTop = m_field->GetFieldCollision().max.y;
        float enemyBottom = pos.y - GetCollision().radius;

        if (enemyBottom <= fieldTop)
        {
            pos.y = fieldTop + GetCollision().radius;
            SetPosition(pos);
            m_onField = true;
            if (m_verticalVelocity < 0.0f)
            {
                m_verticalVelocity = 0.0f;
            }
        }
    }

    // ── フェーズ別処理 ──
    switch (m_phase)
    {
        // ────────────── WEAVING フェーズ ──────────────
    case Phase::WEAVING:
    {
        m_elapsedTime += deltaTime;
        m_waveTimer += deltaTime;

        Vector3 forward = GetForwardVector();
        Vector3 side = GetSideVector();

        // 前進
        m_Position += forward * m_moveSpeed * deltaTime;

        // 左右の揺れ（控えめな振れ幅）
        float swaySpeed = 5.0f;  // 横揺れの速さ
        float swayAmplitude = 15.0f;  // 横揺れの幅
        float swayVelocity = swayAmplitude * swaySpeed
            * cosf(m_waveTimer * swaySpeed);
        m_Position += side * swayVelocity * deltaTime;

        // 上下ピストン（既存のまま）
        float bounceSpeed = 50.0f;
        float bounceHeight = 5.0f;
        float baseY = m_startPosition.y;
        m_Position.y = baseY + std::max(0.0f,
            sinf(m_waveTimer * bounceSpeed) * bounceHeight);

        // 移動距離の累積
        m_traveledDistance += (forward * m_moveSpeed * deltaTime).Length();

        // 進行方向に向ける
        float targetYaw = 0.0f;
        switch (m_direction)
        {
        case MoveDirection::NORTH: targetYaw = 0.0f; break;
        case MoveDirection::SOUTH: targetYaw = 180.0f; break;
        case MoveDirection::EAST:  targetYaw = 90.0f; break;
        case MoveDirection::WEST:  targetYaw = 270.0f; break;
        }
        m_Rotation.y = targetYaw;

        // 上昇トリガー判定
        bool timeTrigger = (m_elapsedTime >= m_timeLimit);
        bool distanceTrigger = (m_traveledDistance >= m_travelLimit);
        if (timeTrigger || distanceTrigger)
        {
            m_phase = Phase::RISING;
            m_verticalVelocity = m_riseSpeed;
        }
        break;
    }

    // ────────────── RISING フェーズ ──────────────
    case Phase::RISING:
    {
        m_verticalVelocity += m_gravity * deltaTime;
        m_Position.y += m_verticalVelocity * deltaTime;

        // X軸→Y軸に変更
        m_Rotation.y += m_rotateSpeed * deltaTime;

        Vector3 cameraPos = SpringCamera::Instance().GetPosition();
        float distance = (m_Position - cameraPos).Length();

        if (distance > 600.0f || m_Position.y < -100.0f)
        {
            if (!m_disappearEffectSpawned)
            {
                SpawnDisappearEffect();
                m_disappearEffectSpawned = true;
            }
            m_isActive = false;
        }
        break;
    }
    }

    // 当たり判定位置を更新
    m_BoundingSphere.center = m_Position;
}

// ──────────────────────────────────────────────
//  Draw
// ──────────────────────────────────────────────

void WeavingEnemy::Draw()
{
    SRT srt;
    srt.pos = m_Position;
    srt.rot = m_Rotation;
    srt.scale = m_Scale;

    Matrix4x4 worldmtx = srt.GetMatrix();
    Renderer::SetWorldMatrix(&worldmtx);
    m_StaticMeshRenderer->Draw();
}

// ──────────────────────────────────────────────
//  Dispose
// ──────────────────────────────────────────────

void WeavingEnemy::Dispose()
{
    m_disappearEffectSpawned = false;
}

// ──────────────────────────────────────────────
//  重力
// ──────────────────────────────────────────────

void WeavingEnemy::ApplyGravity(uint64_t deltatime)
{
    m_verticalVelocity += m_gravity * deltatime;

    const float maxFallSpeed = -15.0f;
    if (m_verticalVelocity < maxFallSpeed)
    {
        m_verticalVelocity = maxFallSpeed;
    }
}

// ──────────────────────────────────────────────
//  ノックバック（既存Enemyと同仕様）
// ──────────────────────────────────────────────

void WeavingEnemy::ApplyKnockback(Vector3 direction, float force, float timeScale)
{
    m_knockbackMove += direction * force;
    m_verticalVelocity = 5.0f;
    m_isKnockedBack = true;
    m_knockbackTimer = 0.5f;
    m_onField = false;
}

void WeavingEnemy::KnockBack(float deltaTime)
{
    float timeScale = GameManager::Instance().GetTimeScale();

    m_knockbackMove *= 0.95f;
    m_knockbackTimer -= deltaTime * timeScale;
    m_Rotation.x += m_rotateSpeed * deltaTime * timeScale;

    Vector3 cameraPos = SpringCamera::Instance().GetPosition();
    float distance = (m_Position - cameraPos).Length();

    if (m_knockbackTimer <= 0.0f || distance > 600.0f)
    {
        if (!m_disappearEffectSpawned)
        {
            SpawnDisappearEffect();
            m_disappearEffectSpawned = true;
        }
        m_isActive = false;
        return;
    }

    m_Position += m_knockbackMove * timeScale;
}

// ──────────────────────────────────────────────
//  エフェクト
// ──────────────────────────────────────────────

void WeavingEnemy::SpawnDisappearEffect()
{
    //EffectManager::Instance().SpawnEffect("Star", m_position, Vector3(450.0f, 450.0f, 0));
    //EffectManager::Instance().SpawnEffect("SparkleParticle", m_position, Vector3(0, 1, 0));
}

// ──────────────────────────────────────────────
//  当たり判定
// ──────────────────────────────────────────────

GM31::GE::Collision::BoundingSphere WeavingEnemy::GetEnemyBoundingSphere()
{
    GM31::GE::Collision::BoundingSphere sphere;
    sphere.center = m_Position;
    sphere.radius = m_BoundingSphereRadius;
    return sphere;
}
