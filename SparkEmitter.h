#pragma once
#include <d3d11.h>
#include <DirectXMath.h>
#include <vector>
#include <wrl/client.h>
#include "system/commontypes.h"
#include "system/CShader.h"
#include "system/CMaterial.h"
#include "ParticleTypes.h" 
using Microsoft::WRL::ComPtr;
using namespace DirectX::SimpleMath;

class SparkEmitter
{

    struct Vertex
    {
        DirectX::XMFLOAT3 pos;
        DirectX::XMFLOAT2 uv;
    };

    struct InstanceData
    {
        DirectX::XMFLOAT4X4 world;
        DirectX::XMFLOAT4 color;
    };

    struct Particle
    {
        DirectX::XMFLOAT3 pos = { 0.0f,0.0f,0.0f };
        DirectX::XMFLOAT3 velocity{ 0.0f,0.0f,0.0f };
        DirectX::XMFLOAT4 color{ 0.0f,0.0f,0.0f,1.0f };
        float size = 100.1f;
        float life = 10.0f;
        float lifespan = 30.0f;//
    };

    struct ConstantBuffer
    {
        DirectX::XMMATRIX viewProj;
    };
    struct WindTrail
    {
        std::vector<DirectX::XMFLOAT3> points;  // トレイルの点列
        DirectX::XMFLOAT3 direction;            // 放射方向
        float life;
        float lifespan;
        DirectX::XMFLOAT4 color;
    };

    

public:
    ~SparkEmitter(); // デストラクタを追加
    bool Init(ID3D11Device* device);
	// 基本のEmit関数(後方互換性のため残す)
   // void Emit(const DirectX::XMFLOAT3& position, const DirectX::XMFLOAT3& direction);
   
    // 動作タイプ別のEmit（統合版）
    void Emit(const DirectX::XMFLOAT3& pos, const DirectX::XMFLOAT3& dir,
        ParticleBehaviorType type = ParticleBehaviorType::Burst);

    //動作タイプ拡張用関数
    // 動作タイプを設定
    void SetBehaviorType(ParticleBehaviorType type) { m_BehaviorType = type; }
    void Update(float deltaTime);
    void Uninit(); //明示的な解放関数も追加
    void Render(ID3D11DeviceContext*, const DirectX::XMMATRIX&);
    void CreateBuffers();
    void SetupRenderState();
    DirectX::XMFLOAT3 LerpColor(DirectX::XMFLOAT3, DirectX::XMFLOAT3, float);

    // 色の範囲を設定できるようにする
    void SetColorRange(DirectX::XMFLOAT3 startColor, DirectX::XMFLOAT3 endColor) { m_StartColor = startColor; m_EndColor = endColor; };


    void SetSparkleMode(bool isGold, float area = 50.0f);

    // 速度の範囲を設定
    void SetSpeedRange(float minSpeed, float maxSpeed) { m_MinSpeed = minSpeed; m_MaxSpeed = maxSpeed; };

    // 拡散角度を設定
    void SetSpreadAngle(float angle) { m_SpreadAngle = angle; };

    // 重力の影響を設定
    void SetGravity(float gravity) { m_Gravity = gravity; };

    // パーティクルのサイズを設定
    void SetParticleSize(float size) { m_ParticleSize = size; };

    float GetParticleCount() const { return static_cast<float>(m_particles.size()); }

    float LerpFloat(float a, float b, float t)
    {
        return a + (b - a) * t;
    }

    DirectX::XMFLOAT4X4 m_WorldMatrix; // エミッタのワールド行列
    void SetWorldMatrix(const DirectX::XMMATRIX& world) {
        XMStoreFloat4x4(&m_WorldMatrix, world);
    }

    Vector3 m_Position = { 0, 0, 0 }; // ワールド上の位置 
    void SetPosition(const Vector3& pos) {
        m_Position = pos;
        XMStoreFloat4x4(&m_WorldMatrix, DirectX::XMMatrixTranslation(pos.x, pos.y, pos.z));
    }
    Vector3 GetPosition() const { return m_Position; }

    void UpdateMeteorShower(Particle& p, float deltaTime);
    void EmitMeteorShower(const DirectX::XMFLOAT3& centerPos, const DirectX::XMFLOAT3& dir);

    void SetMeteorShowerMode(float radius = 50.0f, float distance = 60.0f, float height = 30.0f,
        float speed = 20.0f, int spawnRate = 3);

    void SetPlayerForward(const DirectX::XMFLOAT3& forward) {
        m_playerForward = forward;
    }

private:
    struct Vertex;
    struct InstanceData;

    DirectX::XMFLOAT3 m_StartColor = { 1, 1, 0 };  // 開始色（黄色）
    DirectX::XMFLOAT3 m_EndColor = { 1, 0, 0 };    // 終了色（赤）
    float m_MinSpeed = 1.0f;
    float m_MaxSpeed = 3.0f;
    float m_SpreadAngle = 360.0f;  // 360度 = 全方向
    float m_Gravity = 0.0f;
    float m_ParticleSize = 10.1f;

    ParticleBehaviorType m_BehaviorType = ParticleBehaviorType::Burst;

    CShader m_shader;   // シェーダ
    ComPtr<ID3D11ShaderResourceView> m_fillTexture;
    ComPtr<ID3D11Buffer> m_indexBuffer;
    CMaterial m_material;

    int m_maxParticles = 1000;
    int m_BufferSize = 10000; // バッファサイズ
    
    //動作タイプ拡張用
   // ParticleEmitterGroup::BehaviorType m_BehaviorType = ParticleEmitterGroup::BehaviorType::Burst;
    // 動作タイプ別のUpdate処理
    void UpdateBurst(Particle& p, float deltaTime);
    void UpdateContinuous(Particle& p, float deltaTime);
    void UpdateTrail(Particle& p, float deltaTime);

    void EmitBurst(const DirectX::XMFLOAT3& pos, const DirectX::XMFLOAT3& dir, int count);
    void EmitContinuous(const DirectX::XMFLOAT3& pos, const DirectX::XMFLOAT3& dir);
    void EmitTrail(const DirectX::XMFLOAT3& pos, const DirectX::XMFLOAT3& dir);

    void UpdateSparkle(Particle& p, float deltaTime);
    void EmitSparkle(const DirectX::XMFLOAT3& centerPos, const DirectX::XMFLOAT3& dir);

    // キラキラ設定
    bool m_isGoldSparkle = false;  // 金色かどうか
    float m_sparkleArea = 1280.0f;   // 発生範囲

    //風
    int m_radialRayCount = 12;        // 放射線の本数
    float m_radialSpeed = 1.0f;       // 放射速度
    float m_radialSpreadAngle = 0.0f; // 現在の角度オフセット（回転用
    int m_maxTrails = 50;

    // 流星群用パラメータ
    float m_meteorSpawnRadius = 50.0f;      // プレイヤー周囲の生成半径
    float m_meteorSpawnDistance = 60.0f;    // プレイヤーから横方向の距離
    float m_meteorSpawnHeight = 15.0f;      // 生成高さ（やや低め）
    float m_meteorSpeed = 25.0f;            // 流星の速度
    DirectX::XMFLOAT3 m_playerForward = { 0, 0, 1 };  // プレイヤーの向き
    int m_meteorSpawnRate = 4;              // フレームあたりの生成数
    float m_meteorInnerRadius = 15.0f;   // 中心の避ける範囲（内側の半径）
    float m_meteorOuterRadius = 50.0f;   // 外側の半径
    float m_meteorFrontBackExcludeAngle = 45.0f;  // 前後を避ける角度（度）

    // 流星のトレイル用
    struct MeteorTrail {
        std::vector<DirectX::XMFLOAT3> positions;
        DirectX::XMFLOAT4 color;
        float fadeTime;
    };
    std::vector<MeteorTrail> m_meteorTrails;


    std::vector<Particle> m_particles;
    std::vector<WindTrail> m_windTrails;
    ComPtr<ID3D11ShaderResourceView> m_texture;
    ComPtr<ID3D11ShaderResourceView> m_windStreakTexture;
    ComPtr<ID3D11VertexShader> m_vertexShader;
    ComPtr<ID3D11PixelShader> m_pixelShader;
    ComPtr<ID3D11Buffer> m_vertexBuffer;
    ComPtr<ID3D11Buffer> m_instanceBuffer;
    ComPtr<ID3D11SamplerState> m_samplerState; // サンプラーステートのメンバ変数を追加 
    ComPtr<ID3D11BlendState>  m_blendState = nullptr; // 定数バッファのメンバ変数を追加
    ComPtr<ID3D11Buffer> m_constantBuffer;
    ComPtr<ID3D11DepthStencilState> m_depthStencilState;


    ComPtr<ID3D11InputLayout> m_inputLayout;

    //サンプラー作成
};