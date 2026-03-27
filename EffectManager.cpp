#include"EffeectManager.h"
#include <algorithm>

// ========================================
// BillboardEffect
// ========================================
BillboardEffect::BillboardEffect(const Vector3& position, float width, float height,
    const wchar_t* texturePath, float duration)
    : m_fadeOut(true), m_scale(1.0f), m_initialWidth(width), m_initialHeight(height)
{
    m_maxLifeTime = duration;
    m_lifeTime = 0.0f;
    m_billboard.Init(position, width, height, texturePath);
    m_originalColor = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
}
BillboardEffect::~BillboardEffect()   // デストラクタを追加
{
    m_billboard.Dispose();
    OutputDebugStringA("BillboardEffect デストラクタ呼ばれた\n");
}

void BillboardEffect::Update(float deltaTime)
{
    m_lifeTime += deltaTime;

    if (m_lifeTime >= m_maxLifeTime)
    {
        m_isActive = false;
        return;
    }

    if (m_fadeOut)
    {
        float lifeRatio = m_lifeTime / m_maxLifeTime;
        float alpha = 1.0f - lifeRatio;
        // m_billboard.SetColor(Vector4(1, 1, 1, alpha)); // 実装があれば
    }

    if (m_scale != 1.0f)
    {
        float newWidth = m_initialWidth * m_scale;
        float newHeight = m_initialHeight * m_scale;
        // m_billboard.SetSize(newWidth, newHeight); // 実装があれば
    }
}

void BillboardEffect::Draw(const Matrix4x4& viewMatrix)
{
    if (!m_isActive) return;
    m_billboard.Update(viewMatrix);
    m_billboard.Draw();
}


void ParticleEffect::Update(float deltaTime)
{
    m_lifeTime += deltaTime;

    if (!m_hasSetup && m_emitter)
    {
        m_emitter->SetColorRange(m_config.startColor, m_config.endColor);
        m_emitter->SetSpeedRange(m_config.minSpeed, m_config.maxSpeed);
        m_emitter->SetSpreadAngle(m_config.spreadAngle);
        m_emitter->SetGravity(m_config.gravity);
        m_emitter->SetParticleSize(m_config.particleSize);
        m_emitter->SetBehaviorType(m_config.behaviorType);  // 設定
        m_hasSetup = true;
    }

    if (m_hasSetup && m_emitter && m_lifeTime < m_emitDuration)
    {
        DirectX::XMFLOAT3 pos = { m_position.x, m_position.y, m_position.z };
        DirectX::XMFLOAT3 dir = { m_direction.x, m_direction.y, m_direction.z };

        //behaviorTypeを渡して放出
        m_emitter->Emit(pos, dir, m_config.behaviorType);
    }

    if (m_lifeTime >= m_maxLifeTime)
    {
        m_isActive = false;
    }
}

// ========================================
// EffectManager - 改良版
// ========================================
void EffectManager::Initialize()
{
    m_effects.clear();
    m_presets.clear();
    m_emitters.clear();
    m_currentEmitterIndex = 0;

    //エミッタ数を確認
    for (int i = 0; i < 5; ++i)
    {
        auto emitter = std::make_unique<SparkEmitter>();
        if (emitter->Init(Renderer::GetDevice()))
        {
            m_emitters.push_back(std::move(emitter));

            char buffer[128];
            sprintf_s(buffer, "エミッタ %d 初期化成功\n", i);
            OutputDebugStringA(buffer);
        }
        else
        {
            char buffer[128];
            sprintf_s(buffer, "エミッタ %d 初期化失敗\n", i);
            OutputDebugStringA(buffer);
        }
    }

    char buffer[128];
    printf( "合計 %zu 個のエミッタが作成されました\n", m_emitters.size());

    EffectPresets::RegisterDefaultPresets();
}

void EffectManager::Finalize()
{
    ClearAllEffects();

    //すべてのエミッタを解放
    for (auto& emitter : m_emitters)
    {
        if (emitter)
        {
            emitter->Uninit();
            emitter.reset();
        }
    }
    m_emitters.clear();
    m_presets.clear();

}

void EffectManager::Update(float deltaTime)
{
    // すべてのエフェクトを更新
    for (auto& effect : m_effects)
    {
        if (effect && effect->IsActive())
        {
            effect->Update(deltaTime);
        }
    }

    // 非アクティブなエフェクトを削除
    RemoveInactiveEffects();

    //すべてのエミッタを更新
    for (auto& emitter : m_emitters)
    {
        if (emitter)
        {
            emitter->Update(deltaTime);
        }
    }
}

void EffectManager::Draw(ID3D11DeviceContext* context,const Matrix4x4& viewMatrix)
{
    // BillboardEffect を描画
    for (auto& effect : m_effects)
    {
        if (effect && effect->IsActive())
        {
            effect->Draw(viewMatrix);
        }
    }

    for (auto& emitter : m_emitters)
    {
        if(emitter)
        {
            //viewProjを正しく作る
            DirectX::XMMATRIX view = Renderer::GetViewMatrix();
            DirectX::XMMATRIX proj = Renderer::GetProjectionMatrix();
            DirectX::XMMATRIX viewProj = view * proj;

            emitter->Render(context, view);
        }
    }
}

void EffectManager::RegisterPreset(const std::string& name, const EffectPreset& preset)
{
    m_presets[name] = preset;
}

void EffectManager::SpawnEffect(const std::string& presetName, const Vector3& position,
    const Vector3& direction)
{
    auto it = m_presets.find(presetName);
    if (it == m_presets.end())
        return;

    const EffectPreset& preset = it->second;

    if (preset.type == EffectPreset::Type::Billboard)
    {
        auto effect = std::make_unique<BillboardEffect>(
            position, preset.width, preset.height,
            preset.texturePath.c_str(), preset.duration);
        effect->SetFadeOut(preset.fadeOut);
        m_effects.push_back(std::move(effect));
    }
    else if (preset.type == EffectPreset::Type::Particle)
    {
        SparkEmitter* availableEmitter = GetAvailableEmitter();
        if (!availableEmitter)
        {
            OutputDebugStringA("警告: 利用可能なエミッタがありません\n");
            return;
        }

        auto effect = std::make_unique<ParticleEffect>(
            availableEmitter, position, direction, preset.particleConfig);
        
        m_effects.push_back(std::move(effect));
    }
}

void EffectManager::SpawnBillboardEffect(const Vector3& position, float width, float height,
    const wchar_t* texturePath, float duration, bool fadeOut)
{
    auto effect = std::make_unique<BillboardEffect>(
        position, width, height, texturePath, duration);
    effect->SetFadeOut(fadeOut);
    m_effects.push_back(std::move(effect));
}

void EffectManager::SpawnParticleEffect(const Vector3& position, const Vector3& direction,
    const ParticleEmitterGroup& config)
{
    SparkEmitter* availableEmitter = GetAvailableEmitter();
    if (!availableEmitter)
    {
        OutputDebugStringA("警告: 利用可能なエミッタがありません\n");
        return;
    }

    auto effect = std::make_unique<ParticleEffect>(
        availableEmitter, position, direction, config);
    m_effects.push_back(std::move(effect));
}

void EffectManager::ClearAllEffects()
{
    m_effects.clear();
}

int EffectManager::GetActiveEffectCount() const
{
    int count = 0;
    for (const auto& effect : m_effects)
    {
        if (effect && effect->IsActive())
            ++count;
    }
    return count;
}

void EffectManager::RemoveInactiveEffects()
{
    m_effects.erase(
        std::remove_if(m_effects.begin(), m_effects.end(),
            [](const std::unique_ptr<Effect>& effect) {
                return !effect || !effect->IsActive();
            }),
        m_effects.end()
    );
}

//利用可能なエミッタを返す
SparkEmitter* EffectManager::GetAvailableEmitter()
{
    if (m_emitters.empty())
    {
        OutputDebugStringA("エラー: エミッタリストが空です\n");
        return nullptr;
    }

    SparkEmitter* emitter = m_emitters[m_currentEmitterIndex].get();
    m_currentEmitterIndex = (m_currentEmitterIndex + 1) % m_emitters.size();

    return emitter;
}

// ========================================
// デフォルトプリセット登録
// ========================================
namespace EffectPresets
{
    void RegisterDefaultPresets()//メモ参照！！！！！！！！！！！！！！！
    {
        EffectManager& mgr = EffectManager::Instance();

        //星エフェクト（敵が吹っ飛ぶ時）
        {
            EffectPreset preset;
            preset.type = EffectPreset::Type::Billboard;
            preset.texturePath = L"assets/texture/white_star_big.png";  // 星の画像
            preset.width = 15.5f;
            preset.height = 15.5f;
            preset.duration = 0.8f;
            preset.fadeOut = true;
            mgr.RegisterPreset("Star", preset);
        }

        // キラキラパーティクル
        {
            EffectPreset preset;
            preset.type = EffectPreset::Type::Particle;
            preset.particleConfig.startColor = { 1.0f, 1.0f, 0.3f };  // 黄色
            preset.particleConfig.endColor = { 1.0f, 1.0f, 1.0f };    // 白
            preset.particleConfig.minSpeed = 5.0f;
            preset.particleConfig.maxSpeed = 10.0f;
            preset.particleConfig.spreadAngle = 360.0f;
            preset.particleConfig.gravity = -5.0f;  // 上昇
            preset.particleConfig.particleSize = 10.2f;
            preset.particleConfig.duration = 10.5f;
           // preset.particleConfig.emitCount = 100;  // Emit を2回呼ぶ = 200個
            mgr.RegisterPreset("SparkleParticle", preset);
        }

        // フラッシュ（Billboard）
        {
            EffectPreset preset;
            preset.type = EffectPreset::Type::Billboard;
            preset.texturePath = L"assets/texture/space_.png";
            preset.width = 8.0f;
            preset.height = 8.0f;
            preset.duration = 0.3f;
            preset.fadeOut = true;
            mgr.RegisterPreset("Flash", preset);
        }

        // 爆発パーティクル
        {
            EffectPreset preset;
            preset.type = EffectPreset::Type::Particle;
            preset.particleConfig.startColor = { 1.0f, 0.8f, 0.2f };  // オレンジ
            preset.particleConfig.endColor = { 1.0f, 0.2f, 0.0f };    // 赤
            preset.particleConfig.minSpeed = 10.0f;
            preset.particleConfig.maxSpeed = 20.0f;
            preset.particleConfig.spreadAngle = 360.0f;
            preset.particleConfig.gravity = 0.0f;
            preset.particleConfig.particleSize = 0.3f;
            preset.particleConfig.duration = 1.0f;
            //preset.particleConfig.emitCount = 1;  // Emit を3回呼ぶ = 300個
            mgr.RegisterPreset("Explosion", preset);
        }
        //砂煙エフェクト
        {
            EffectPreset preset;
            preset.type = EffectPreset::Type::Particle;
            preset.particleConfig.behaviorType = ParticleBehaviorType::Continuous;
            preset.particleConfig.startColor = { 0.9f, 0.9f, 0.9f };
            preset.particleConfig.endColor = { 1.0f, 1.0f, 1.0f };
            preset.particleConfig.minSpeed = 0.5f;
            preset.particleConfig.maxSpeed = 1.5f;
            preset.particleConfig.spreadAngle = 90.0f;
            preset.particleConfig.gravity = -0.5f;
            preset.particleConfig.particleSize = 30.0f;
            preset.particleConfig.duration = 2.0f;
            preset.particleConfig.emitCountPerCall = 3;
            mgr.RegisterPreset("DustCloud", preset);
        }

        // ヒットエフェクト
        {
            EffectPreset preset;
            preset.type = EffectPreset::Type::Particle;
            preset.particleConfig.startColor = { 0.2f, 0.5f, 1.0f };  // 青
            preset.particleConfig.endColor = { 0.3f, 1.0f, 1.0f };    // シアン
            preset.particleConfig.minSpeed = 8.0f;
            preset.particleConfig.maxSpeed = 12.0f;
            preset.particleConfig.spreadAngle = 180.0f;  // 半球状
            preset.particleConfig.gravity = 0.0f;
            preset.particleConfig.particleSize = 0.15f;
            preset.particleConfig.duration = 1.0f;
            //preset.particleConfig.emitCount = 1;  // Emit を1回 = 100個
            mgr.RegisterPreset("HitEffect", preset);
        }
    }
}