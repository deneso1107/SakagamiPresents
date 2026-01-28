#pragma once
#include <memory>
#include <string>
#include <unordered_map>
#include "Billboard.h"
#include "SparkEmitter.h"
#include"ParticleTypes.h"
using namespace DirectX::SimpleMath;
// エフェクトの基底クラス
class Effect
{
public:
    Effect() : m_IsActive(true), m_LifeTime(0.0f), m_MaxLifeTime(1.0f) {}
    virtual ~Effect() = default;

    virtual void Update(float deltaTime) = 0;
    virtual void Draw(const Matrix4x4& viewMatrix) = 0;

    bool IsActive() const { return m_IsActive; }
    void SetActive(bool active) { m_IsActive = active; }

protected:
    bool m_IsActive;
    float m_LifeTime;
    float m_MaxLifeTime;
};

//既存Billboardクラスを使ったエフェクト
class BillboardEffect : public Effect
{
public:
    BillboardEffect(const Vector3& position, float width, float height,
        const wchar_t* texturePath, float duration);
    ~BillboardEffect() override;

    void Update(float deltaTime) override;
    void Draw(const Matrix4x4& viewMatrix) override;

    void SetFadeOut(bool fade) { m_FadeOut = fade; }
    void SetScale(float scale) { m_Scale = scale; }

private:
    Billboard m_Billboard;
    Vector4 m_OriginalColor;
    bool m_FadeOut;
    float m_Scale;
    float m_InitialWidth;
    float m_InitialHeight;
};

class ParticleEffect : public Effect
{
public:
    ParticleEffect(SparkEmitter* sharedEmitter,
        const Vector3& position, const Vector3& direction,
        const ParticleEmitterGroup& config)
        : m_Emitter(sharedEmitter),
        m_Config(config),
        m_Position(position),
        m_Direction(direction),
        m_ParticleCountToEmit(config.emitCountPerCall),
        m_HasSetup(false),
        m_EmitDuration(0.3f)
    {
        m_MaxLifeTime = config.duration;
        m_LifeTime = 0.0f;
    }

    ~ParticleEffect() override = default;
    void Update(float deltaTime) override;
    void Draw(const Matrix4x4& viewMatrix) override {}

    const ParticleEmitterGroup& GetConfig() const { return m_Config; }
    Vector3 GetPosition() const { return m_Position; }
    Vector3 GetDirection() const { return m_Direction; }

private:
    SparkEmitter* m_Emitter;
    ParticleEmitterGroup m_Config;
    Vector3 m_Position;
    Vector3 m_Direction;
    int m_ParticleCountToEmit;
    bool m_HasSetup;
    float m_EmitDuration;
};



// ========================================
// エフェクトプリセット
// ========================================
struct EffectPreset
{
    enum class Type {
        Billboard,
        Particle
    };

    Type type;
    std::wstring texturePath;
    float width;
    float height;
    float duration;

    // パーティクル設定
    ParticleEmitterGroup particleConfig;
    bool fadeOut;

    EffectPreset()
        : type(Type::Billboard), texturePath(L""),
        width(1.0f), height(1.0f), duration(1.0f),
        fadeOut(true)
    {
    }
};

// ========================================
// EffectManager - 改良版
// ========================================
class EffectManager
{
public:
    static EffectManager& Instance()
    {
        static EffectManager instance;
        return instance;
    }

    void Initialize();
    void Finalize();

    void Update(float deltaTime);
    void Draw(ID3D11DeviceContext*,const Matrix4x4& viewMatrix);

    // プリセット登録
    void RegisterPreset(const std::string& name, const EffectPreset& preset);

    // エフェクト生成
    void SpawnEffect(const std::string& presetName, const Vector3& position,
        const Vector3& direction = Vector3(0, 1, 0));

    void SpawnBillboardEffect(const Vector3& position, float width, float height,
        const wchar_t* texturePath, float duration, bool fadeOut = true);

    void SpawnParticleEffect(const Vector3& position, const Vector3& direction,
        const ParticleEmitterGroup& config);

    void ClearAllEffects();
    int GetActiveEffectCount() const;

private:
    EffectManager() = default;
    ~EffectManager() { Finalize(); }
    EffectManager(const EffectManager&) = delete;
    EffectManager& operator=(const EffectManager&) = delete;

    std::vector<std::unique_ptr<Effect>> m_Effects;
    std::unordered_map<std::string, EffectPreset> m_Presets;
    size_t m_CurrentEmitterIndex = 0;

    //BillBoard抑制
    bool m_BillboardSpawnedThisFrame = false;
    int m_CurrentFrame = 0;//現在のフレーム数を取得する

    //複数のエミッタを保持
    std::vector<std::unique_ptr<SparkEmitter>> m_Emitters;

    void RemoveInactiveEffects();
    SparkEmitter* GetAvailableEmitter();
};

// ========================================
// ヘルパー関数
// ========================================
namespace EffectPresets
{
    void RegisterDefaultPresets();
}