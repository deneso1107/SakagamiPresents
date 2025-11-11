#pragma once
#include <DirectXMath.h>

// ★ パーティクルの動作タイプ（どこからでも参照可能）
enum class ParticleBehaviorType
{
    Burst,        // 一度に大量放出（爆発など）
    Continuous,   // 連続的に少量放出（砂煙、炎など）
    Trail         // 軌跡（剣の軌跡など）
};

// ★ パーティクルエミッターの設定構造体
struct ParticleEmitterGroup
{
    DirectX::XMFLOAT3 startColor;
    DirectX::XMFLOAT3 endColor;
    float minSpeed;
    float maxSpeed;
    float spreadAngle;
    float gravity;
    float particleSize;
    float duration;

    ParticleBehaviorType behaviorType;
    int emitCountPerCall;
    float emitInterval;
    bool useAirResistance;
    bool expandOverTime;
    float sizeGrowthRate;

    ParticleEmitterGroup()
        : startColor{ 1, 1, 0 }, endColor{ 1, 0, 0 },
        minSpeed(1.0f), maxSpeed(3.0f),
        spreadAngle(360.0f), gravity(0.0f),
        particleSize(0.1f), duration(1.0f),
        behaviorType(ParticleBehaviorType::Burst),
        emitCountPerCall(100),
        emitInterval(0.0f),
        useAirResistance(false),
        expandOverTime(false),
        sizeGrowthRate(1.0f)
    {
    }
};