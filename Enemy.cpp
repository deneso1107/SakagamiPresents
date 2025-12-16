#include "system/commontypes.h"
#include "system/CStaticMesh.h"
#include "system/CStaticMeshRenderer.h"
#include "system/CDirectInput.h"
#include "Enemy.h"
#include "Player.h"
#include "CarDriveScene.h"

void Enemy::Init()
{
	m_Position = Vector3(0.0f, 0.0f, 0.0f);
	m_Rotation = Vector3(0.0f, 0.0f, 0.0f);
	m_Scale = Vector3(1.0f, 1.0f, 1.0f);

	m_BoundingSphere=
	{
		m_Position,
		5.0f,//カスコード
	};
	m_IsKnockedBack = false;	// ノックバック状態を初期化
}

void Enemy::Update(float deltaTime)
{
    //m_BoundingSphere =
    //{
    //    m_Position,
    //    5.0f,//カスコード
    //};
    // フィールドとの衝突判定
    if (GM31::GE::Collision::CollisionSphereAABB(this->GetCollision(), m_field->GetFieldCollision()))
    {
        // フィールドに接触している場合
        Vector3 enemyPos = GetPosition();
        float fieldTop = m_field->GetFieldCollision().max.y;
        float enemyBottom = enemyPos.y - GetCollision().radius;

        // 敵が地面より下に落ちた場合のみ補正
        if (enemyBottom <= fieldTop) {
            // フィールドの上に配置
            enemyPos.y = fieldTop + GetCollision().radius;
            SetPosition(enemyPos);
            onField = true;
            // 地面に着地したら垂直速度をリセット（下向きの速度のみ）
            if (!m_IsKnockedBack && m_verticalVelocity < 0.0f) {
                m_verticalVelocity = 0.0f;
            }
        }
        else {
            // フィールドの範囲内だが、地面から離れている（空中にいる）
            onField = false;
        }

        if (m_IsKnockedBack)
        {
            KnockBack(deltaTime);    // ノックバック中の処理
        }
        else
        {
            //Player* p;
            //p = ((CarDriveScene*)(m_ownerscene))->GetPlayer();    // プレイヤの取得
            //// プレイヤの座標を取得
            //Vector3 playerpos = p->GetPosition();
            //// atan2を使用して角度を求める
            //m_Destrot.y = atan2f(-(playerpos.x - m_Position.x), -(playerpos.z - m_Position.z));
            //// 現在の向きとの差分を計算する
            //float fDiffRotY = m_Destrot.y - m_Rotation.y;
            //// 補正（－１８０～１８０の範囲）
            //if (fDiffRotY > PI)
            //{
            //    fDiffRotY -= PI * 2.0f;
            //}
            //if (fDiffRotY < -PI)
            //{
            //    fDiffRotY += PI * 2.0f;
            //}
            //// 回転角度計算
            //m_Rotation.y += fDiffRotY * RATE_ROTATE_ENEMY;
            //if (m_Rotation.y > PI)
            //{
            //    m_Rotation.y -= PI * 2.0f;
            //}
            //if (m_Rotation.y < -PI)
            //{
            //    m_Rotation.y += PI * 2.0f;
            //}
            //Matrix4x4 mtx = Matrix4x4::CreateRotationY(m_Rotation.y);    // 目標角度を現在の角度に更新
            //Vector3 forward = mtx.Forward();    // 前方ベクトルを取得
            //m_Move = forward * m_speed;    // 前方ベクトルにスピードを掛けて移動量を計算
            ///// 位置移動（X、Z軸のみ。Y軸は重力で制御済み）
            //m_Position.x += m_Move.x;
            //m_Position.z += m_Move.z;
            //// 移動量に慣性をかける(減速率)
            //m_Move += -m_Move * RATE_MOVE_MODEL;
        }
    }
    //else
    //{
    //    // フィールドから離れている場合
    //    onField = false;


    //    // 常に重力を適用
    //    ApplyGravity(deltaTime);//重力がえぐい　しかもPLayerが動いている時だけめちゃdeltatimeが大きくなる一旦保留((重力を進める場合は25行目のNoundingSquareのコメントを解除)
    //    m_Position.y += m_verticalVelocity * deltaTime;
    //    if (m_IsKnockedBack)
    //    {
    //        KnockBack(deltaTime);
    //    }
    //}
}

void Enemy::ApplyGravity(uint64_t deltatime)
{
    // 常に重力を適用（地面での補正は後で行う）
    // 重力を垂直速度に加算
    m_verticalVelocity += m_gravity * deltatime /** 60.0f*/; // 60FPS基準で調整

    // 落下速度の制限（ターミナル速度）
    const float maxFallSpeed = -15.0f;
    if (m_verticalVelocity < maxFallSpeed)
    {
        m_verticalVelocity = maxFallSpeed;
    }
}

void Enemy::Draw()
{
    SRT srt;

    // SRT情報作成
    srt.pos = m_Position;            // 位置
    srt.rot = m_Rotation;            // 姿勢
    srt.scale = m_Scale;            // 拡縮

    Matrix4x4 worldmtx;

    worldmtx = srt.GetMatrix();

    Renderer::SetWorldMatrix(&worldmtx);        // GPUにセット

    m_StaticMeshRenderer->Draw();

    Color bscolor(1, 1, 1, 0.5f);

    SphereDrawerDraw(m_BoundingSphere.radius, bscolor, m_Position.x, m_Position.y, m_Position.z);//球体を描画
}

void Enemy::Dispose()
{
    m_DisappearEffectSpawned = false;
}

void Enemy::ApplyKnockback(Vector3 direction, float force, float timeScale)
{
    // ノックバックの適用
    m_Move += direction * force;

    // 上向きの力も追加（車が浮く効果）
    m_verticalVelocity = 5.0f; // 上向きの初期速度を与える

    // 初期位置は変更しない（KnockBack関数で更新される）

    m_IsKnockedBack = true;
    m_KnockbackTimer = 10.5f;  // 3秒間飛行
    onField = false; // ノックバックで地面から離れる
}

void Enemy::KnockBack(float deltaTime)
{

    float timeScale = GameManager::Instance().GetTimeScale();
    // deltaTimeは既にfloat型なので、そのまま使用
    // 元のコードに戻す

    // 摩擦で減速（徐々に遅くなる）
    m_Move *= 0.95f;

    // タイマー更新
    m_KnockbackTimer -= deltaTime* timeScale;
    m_Rotation.x += m_RotateSpeed * deltaTime * timeScale; // 回転速度は調整可能
	Vector3 cameraPos = SpringCamera::Instance().GetPosition();
    float distance = (m_Position - cameraPos).Length();


    // 一定距離以上、または時間経過でエフェクト発生
    if ((/*distance > 80.0f ||*/ m_KnockbackTimer <= 0.2f) && !m_EffectSpawned)//ここから 
    {
        m_EffectSpawned = true;
        GameManager::Instance().SetTimeScale(1.0f);//スローモーションの調整
    }

    // 完全に消す
    if (m_KnockbackTimer <= 0.0f|| distance > 1000.0f)
    {
        SetActive(false);
        // まだエフェクトを出していなければ出す
        if (!m_DisappearEffectSpawned)
        {
            SpawnDisappearEffect();
            m_DisappearEffectSpawned = true;
        }
        return;;
    }

    if (m_KnockbackTimer <= 0.0f)
    {
        m_IsKnockedBack = false;
        m_Move = Vector3(0, 0, 0);
    }

    
    m_Position += m_Move* timeScale;//ここなぜかスローにならない
}

void Enemy::SpawnDisappearEffect()
{
    // テスト1: 同じ位置で両方生成
    EffectManager::Instance().SpawnEffect("Star", m_Position, Vector3(450.0f, 450.0f, 0));
    printf("Star生成完了\n");

    EffectManager::Instance().SpawnEffect("SparkleParticle", m_Position, Vector3(0, 1, 0));
    //printf("SparkleParticle生成完了\n");
}

GM31::GE::Collision::BoundingSphere Enemy::GetEnemyBoundingSphere()//Enemyの当たり判定を取得する関数
{
    GM31::GE::Collision::BoundingSphere sphere;
    sphere.center = m_Position;//敵のデフォルトのBoundingSquareとは別のやつを使っているのでどっちも変更尾する必要あり
	sphere.radius = 10.0f; // X座標を半径として使用(カスコード)→ここの当たり判定だけ大きくする
    return sphere;
}