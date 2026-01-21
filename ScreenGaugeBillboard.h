#pragma once
#include "ScreenFixedBillboard.h"
#include <d3d11.h>
#include <DirectXMath.h>

// 頂点構造体の定義（ScreenFixedBillboardと同じ構造を想定）
struct ScreenBillboardVertex {
    Vector3 position;
    Vector2 texCoord;
};

//ゲージクラス 
class ScreenGaugeBillboard
{
private:
    // プロパティ
    Vector2 m_screenPosition;
    float m_width;
    float m_height;

    // テクスチャ
    ID3D11ShaderResourceView* m_frameTexture;    // 外枠
    ID3D11ShaderResourceView* m_fillTexture;     // 内容
    ID3D11ShaderResourceView* m_textTexture;     // バーの上の文字

    // バッファ
    ID3D11Buffer* m_frameVertexBuffer;
    ID3D11Buffer* m_fillVertexBuffer;
    ID3D11Buffer* m_textVertexBuffer;
    ID3D11Buffer* m_indexBuffer;

    // ゲージ制御
    float m_currentValue;        // 現在値 (0.0f〜1.0f)
    float m_targetValue;         // 目標値
    float m_animationSpeed;      // アニメーション速度

    // 表示設定
    Vector2 m_fillMargin;        // 内容の余白
    Vector4 m_tintColor;         // 色調整用

    // シェーダー・マテリアル
    CMaterial m_material;

public:
    ScreenGaugeBillboard();
    ~ScreenGaugeBillboard();

    // 初期化
    bool Init(const Vector2& screenPos, float width, float height,
        const wchar_t* frameTexturePath, const wchar_t* fillTexturePath,
        const Vector2& fillMargin = Vector2(0.05f, 0.05f));

    // ゲージ制御
    void SetValue(float value, bool animate = true);
    void SetAnimationSpeed(float speed) { m_animationSpeed = speed; }
    void SetTintColor(const Vector4& color) { m_tintColor = color; }

    // 取得
    float GetCurrentValue() const { return m_currentValue; }
    float GetTargetValue() const { return m_targetValue; }
    bool IsAnimating() const { return abs(m_currentValue - m_targetValue) > 0.001f; }

    // 更新・描画
    void Update(float deltaTime);
    void Draw();

private:
    void CreateBuffers();
    void UpdateFrameVertexBuffer();
    void UpdateTextVertexBuffer();
    void UpdateFillVertexBuffer();
    void SetupRenderState();
    void CleanupRenderState();
};