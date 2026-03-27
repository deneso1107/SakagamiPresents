#pragma once

#include "system/commontypes.h"
#include "system/CShader.h"
#include "system/CStaticMesh.h"
#include "system/CStaticMeshRenderer.h"
#include "system/collision.h"
#include"system/SphereDrawer.h"
#include "system/DebugUI.h"

class ObjectBase {
protected:
	// SRT情報（姿勢情報）ここ小文字にしたら違和感あるから大文字のままに
    Vector3 m_Position = Vector3(0.0f, 0.0f, 0.0f);
    Vector3 m_Rotation = Vector3(0.0f, 0.0f, 0.0f);
    Vector3 m_Scale = Vector3(1.0f, 1.0f, 1.0f);

    // 当たり判定　ここ小文字にしたら違和感あるから大文字のままに
    GM31::GE::Collision::BoundingSphere m_BoundingSphere;

public:
    // コンストラクタ・デストラクタ
    ObjectBase() = default;
    virtual ~ObjectBase() = default;

    // 基本的なライフサイクル関数（純粋仮想関数）
    virtual void Init() = 0;
    virtual void Update(float deltaTime) = 0;
    virtual void Draw() = 0;
    virtual void Dispose() = 0;

    // Position操作
    virtual void SetPosition(Vector3 pos) { m_Position = pos; }
    virtual Vector3 GetPosition() const { return m_Position; }

    // Rotation操作
    virtual void SetRotation(Vector3 rot) { m_Rotation = rot; }
    virtual Vector3 GetRotation() const { return m_Rotation; }

    // Scale操作
    virtual void SetScale(Vector3 scale) { m_Scale = scale; }
    virtual Vector3 GetScale() const { return m_Scale; }

    // 当たり判定取得
    virtual GM31::GE::Collision::BoundingSphere GetCollision() const { return m_BoundingSphere; }

    // 重力とグランド判定用のメソッドを追加
    //void ApplyGravity(uint64_t deltatime);

    // SRT情報をまとめて取得
    virtual SRT GetSRT() const {
        SRT srt;
        srt.pos = m_Position;
        srt.rot = m_Rotation;
        srt.scale = m_Scale;
        return srt;
    }

    // 前方ベクトル取得（Y軸回転基準）
    virtual Vector3 GetForwardVector() const
    {
        //Y軸で回転する
       //仮に90f回転した場合、前方ベクトルは(1, 0, 0)になる()右向き
        return Vector3(sinf(m_Rotation.y), 0.0f, cosf(m_Rotation.y));
    }

 
};
