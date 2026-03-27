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
    Effect() : m_isActive(true), m_lifeTime(0.0f), m_maxLifeTime(1.0f) {}
    virtual ~Effect() = default;

    virtual void Update(float deltaTime) = 0;
    virtual void Draw(const Matrix4x4& viewMatrix) = 0;

    bool IsActive() const { return m_isActive; }
    void SetActive(bool active) { m_isActive = active; }

protected:
    bool m_isActive;
    float m_lifeTime;
    float m_maxLifeTime;
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

    void SetFadeOut(bool fade) { m_fadeOut = fade; }
    void SetScale(float scale) { m_scale = scale; }

private:
    Billboard m_billboard;
    Vector4 m_originalColor;
    bool m_fadeOut;
    float m_scale;
    float m_initialWidth;
    float m_initialHeight;
};

class ParticleEffect : public Effect
{
public:
    ParticleEffect(SparkEmitter* sharedEmitter,
        const Vector3& position, const Vector3& direction,
        const ParticleEmitterGroup& config)
        : m_emitter(sharedEmitter),
        m_config(config),
        m_position(position),
        m_direction(direction),
        m_particleCountToEmit(config.emitCountPerCall),
        m_hasSetup(false),
        m_emitDuration(0.3f)
    {
        m_maxLifeTime = config.duration;
        m_lifeTime = 0.0f;
    }

    ~ParticleEffect() override = default;
    void Update(float deltaTime) override;
    void Draw(const Matrix4x4& viewMatrix) override {}

    const ParticleEmitterGroup& GetConfig() const { return m_config; }
    Vector3 GetPosition() const { return m_position; }
    Vector3 GetDirection() const { return m_direction; }

private:
    SparkEmitter* m_emitter;
    ParticleEmitterGroup m_config;
    Vector3 m_position;
    Vector3 m_direction;
    int m_particleCountToEmit;
    bool m_hasSetup;
    float m_emitDuration;
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

    std::vector<std::unique_ptr<Effect>> m_effects;
    std::unordered_map<std::string, EffectPreset> m_presets;
    size_t m_currentEmitterIndex = 0;

    //BillBoard抑制
    bool m_billboardSpawnedThisFrame = false;
    int m_currentFrame = 0;//現在のフレーム数を取得する

    //複数のエミッタを保持
    std::vector<std::unique_ptr<SparkEmitter>> m_emitters;

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