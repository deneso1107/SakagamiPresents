#pragma once
#include <d3d11.h>
#include "system/commontypes.h"
#include "system/CMaterial.h"
//9月14日　角度とUV座標追加

class ScreenFixedBillboard////////////////////デストラクタが必要
{
private:
    Vector2 m_screenPosition;    // スクリーン座標での位置（0.0f-1.0f）
    float m_width;               // ビルボードの幅（スクリーン比率）
    float m_height;              // ビルボードの高さ（スクリーン比率）
    float m_angle;               // 回転角度（度数法）
    ID3D11ShaderResourceView* m_texture;

    struct ScreenBillboardVertex
    {
        Vector3 position;    // スクリーン空間座標
        Vector2 texcoord;    // テクスチャ座標
    };

    ID3D11Buffer* m_vertexBuffer;
    ID3D11Buffer* m_indexBuffer;
    CMaterial m_Material;

    Vector2 m_uvOffset;      // UV座標のオフセット（左上）
    Vector2 m_uvSize;        // UV座標のサイズ（幅、高さ）


public:
    // コピー/ムーブを禁止
    ScreenFixedBillboard(const ScreenFixedBillboard&) = delete;
    ScreenFixedBillboard& operator=(const ScreenFixedBillboard&) = delete;
    ScreenFixedBillboard(ScreenFixedBillboard&&) = delete;
    ScreenFixedBillboard& operator=(ScreenFixedBillboard&&) = delete;
    ScreenFixedBillboard(const Vector2& screenPos, float width, float height, const wchar_t* texturePath);
    // 既存コンストラクタに加えて、UV指定版を追加
   /* ScreenFixedBillboard(const Vector2& screenPos, float width, float height,
        const wchar_t* texturePath,
        const Vector2& uvOffset = Vector2(0.0f, 0.0f),
        const Vector2& uvSize = Vector2(1.0f, 1.0f));*/
    //~ScreenFixedBillboard();

    void Init(const Vector2& screenPos, float width, float height, const wchar_t* texturePath);
    void Dispose();
    void Update();  // ビューマトリックス不要
    void Draw();

    void SetScreenPosition(const Vector2& pos) { m_screenPosition = pos; }
    void SetSize(float width, float height) { m_width = width; m_height = height; }
    void SetAngle(float angle) { m_angle = angle; } 
    void SetUVRange(float u1, float v1, float u2, float v2);
    void SetUVOffset(const Vector2& offset) { m_uvOffset = offset; }
    void SetUVSize(const Vector2& size) { m_uvSize = size; }
    Vector2 RotatePoint(const Vector2& point, const Vector2& center, float angle); 

private:
    void CreateBuffers();
    void UpdateVertexBuffer();
    void CreateDefaultTexture();
    // UV座標を動的に変更するメソッド
};
