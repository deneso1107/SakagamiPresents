#pragma once
#include "ObjectBase.h"
#include "CTerrainMesh.h"

class BaseStageModel : public ObjectBase 
{
private:
    CStaticMeshRenderer m_meshRenderer;
    CStaticMesh m_mesh;
    CShader m_shader;
    CShader m_shadowShader;
    bool m_isInitialized = false;

    virtual std::string GetModelFileName() const = 0;

    // ふわふわ回転
    float m_rotationY = 0.0f;
    float m_floatTimer = 0.0f;
    float m_baseY = 0.0f;   // 初期Y座標

    // スライド慣性による傾き
    float m_tiltZ = 0.0f;   // 現在の傾き
    float m_targetTiltZ = 0.0f;   // 目標傾き

    // ハイライト
    bool  m_isSelected = false;
    float m_highlight = 0.0f;   // 0.0?1.0

    // 左右入力時のスピン
    float m_spinVelocity = 0.0f;  // 入力時に加算される回転速度

    // ハイライト用定数バッファ
    struct HighlightBuffer {
        DirectX::XMFLOAT4 highlightColor; // rgb=色, a=強度
    };
    ID3D11Buffer* m_highlightCBuffer = nullptr;


public:
    BaseStageModel() = default;
    virtual ~BaseStageModel() {
        if (m_isInitialized) Dispose();
    }

    void Init() override;
    void Update(float deltatime) override;
    void Draw() override;
    void Dispose() override;

    bool IsInitialized() const { return m_isInitialized; }

    float m_carouselScale = 1.0f;  // カルーセル用スケール（個別スケールとは別管理）
    Vector3 m_baseScale = Vector3(1.0f, 1.0f, 1.0f); // 個別設定のスケール

    void SetSelected(bool selected) { m_isSelected = selected; }
    void AddSpin(float velocity) { m_spinVelocity += velocity; } // 左右入力時に呼ぶ
    void SetTilt(float direction) { m_targetTiltZ = direction; } // スライド時に呼ぶ (-1:左, 1:右)
    void ResetTilt() { m_targetTiltZ = 0.0f; }
    float GetHighlight() const { return m_highlight; }
    void SetCarouselScale(float scale) { m_carouselScale = scale; }
    // 既存のSetScaleをオーバーライドしてbaseScaleに保存
    void SetBaseScale(const Vector3& scale) { m_baseScale = scale; }
    void SetBaseY(float y) { m_baseY = y; }

};
