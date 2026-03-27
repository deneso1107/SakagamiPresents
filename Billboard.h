#pragma once
//#include <SimpleMath.h>
#include <d3d11.h>
#include "system/commontypes.h"
#include "system/CMaterial.h"
using namespace DirectX::SimpleMath;

enum class BillboardBlendType
{
    Opaque,
    Additive
};
class TextureCache
{
public:
    static TextureCache& Instance()
    {
        static TextureCache instance;
        return instance;
    }

    ComPtr<ID3D11ShaderResourceView> GetTexture(ID3D11Device* device, const wchar_t* path);

    void Clear()
    {
        m_Cache.clear();
        OutputDebugStringA("テクスチャキャッシュをクリア\n");
    }

private:
    TextureCache() = default;
    std::unordered_map<std::wstring, ComPtr<ID3D11ShaderResourceView>> m_Cache;
};


class Billboard
{
private:
    Vector3 m_position;
    float m_width;
    float m_height;

    // ComPtr に変更
    ComPtr<ID3D11ShaderResourceView> m_texture;
    ComPtr<ID3D11Buffer> m_vertexBuffer;
    ComPtr<ID3D11Buffer> m_indexBuffer;

    //CShader m_shader;  // シェーダーメンバを追加

    struct BillboardVertex
    {
        Vector3 position;
        Vector3 normal;
        Vector4 diffuse;
        Vector2 texcoord;
    };

    CMaterial m_material;
    bool m_isInitialized = false;  //初期化済みフラグ
    bool m_needsUpdate = true;     //更新が必要か

public:
    Billboard();
    ~Billboard();


    BillboardBlendType m_blendType = BillboardBlendType::Opaque;

    void Init(const Vector3& position, float width, float height, const wchar_t* texturePath);
    void Dispose();
    void Update(const Matrix4x4& viewMatrix);
    void Draw();

    void SetPosition(const Vector3& position)
    {
        if (m_position.x != position.x ||
            m_position.y != position.y ||
            m_position.z != position.z)
        {
            m_position = position;
            m_needsUpdate = true;  //位置が変わったら更新フラグON
        }
    }

    void SetSize(float width, float height)
    {
        if (m_width != width || m_height != height)
        {
            m_width = width;
            m_height = height;
            m_needsUpdate = true;  //サイズが変わったら更新フラグON
        }
    }

private:
    void CreateBuffers();
    void UpdateVertexBuffer(const Matrix4x4& viewMatrix);
    void CreateDefaultTexture();
};
