#include"EffeectManager.h"
#include <algorithm>

// ========================================
// BillboardEffect
// ========================================
BillboardEffect::BillboardEffect(const Vector3& position, float width, float height,
    const wchar_t* texturePath, float duration)
    : m_FadeOut(true), m_Scale(1.0f), m_InitialWidth(width), m_InitialHeight(height)
{
    m_MaxLifeTime = duration;
    m_LifeTime = 0.0f;
    m_Billboard.Init(position, width, height, texturePath);
    m_OriginalColor = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
}
BillboardEffect::~BillboardEffect()   // ★ デストラクタを追加
{
    m_Billboard.Dispose();
    OutputDebugStringA("BillboardEffect デストラクタ呼ばれた\n");
}

void BillboardEffect::Update(float deltaTime)
{
    m_LifeTime += deltaTime;

    if (m_LifeTime >= m_MaxLifeTime)
    {
        m_IsActive = false;
        return;
    }

    if (m_FadeOut)
    {
        float lifeRatio = m_LifeTime / m_MaxLifeTime;
        float alpha = 1.0f - lifeRatio;
        // m_Billboard.SetColor(Vector4(1, 1, 1, alpha)); // 実装があれば
    }

    if (m_Scale != 1.0f)
    {
        float newWidth = m_InitialWidth * m_Scale;
        float newHeight = m_InitialHeight * m_Scale;
        // m_Billboard.SetSize(newWidth, newHeight); // 実装があれば
    }
}

void BillboardEffect::Draw(const Matrix4x4& viewMatrix)
{
    if (!m_IsActive) return;
    m_Billboard.Update(viewMatrix);
    m_Billboard.Draw();
}

// ========================================
// ParticleEffect - 改良版
// ========================================
//ParticleEffect(SparkEmitter* sharedEmitter,
//    const Vector3& position, const Vector3& direction,
//    const ParticleEmitterGroup& config)
//    : m_Emitter(sharedEmitter),
//    m_Config(config),
//    m_Position(position),
//    m_Direction(direction),
//    m_ParticleCountToEmit(config.emitCount),
//    m_HasSetup(false),
//    m_EmitDuration(0.3f)  // ★ 最初の0.3秒だけ放出
//{
//    m_MaxLifeTime = config.duration;
//    m_LifeTime = 0.0f;
//}


void ParticleEffect::Update(float deltaTime)
{
    m_LifeTime += deltaTime;

    if (!m_HasSetup && m_Emitter)
    {
        m_Emitter->SetColorRange(m_Config.startColor, m_Config.endColor);
        m_Emitter->SetSpeedRange(m_Config.minSpeed, m_Config.maxSpeed);
        m_Emitter->SetSpreadAngle(m_Config.spreadAngle);
        m_Emitter->SetGravity(m_Config.gravity);
        m_Emitter->SetParticleSize(m_Config.particleSize);
        m_Emitter->SetBehaviorType(m_Config.behaviorType);  // ★ 設定
        m_HasSetup = true;
    }

    if (m_HasSetup && m_Emitter && m_LifeTime < m_EmitDuration)
    {
        DirectX::XMFLOAT3 pos = { m_Position.x, m_Position.y, m_Position.z };
        DirectX::XMFLOAT3 dir = { m_Direction.x, m_Direction.y, m_Direction.z };

        // ★ behaviorTypeを渡して放出
        m_Emitter->Emit(pos, dir, m_Config.behaviorType);
    }

    if (m_LifeTime >= m_MaxLifeTime)
    {
        m_IsActive = false;
    }
}

//void ParticleEffect::Update(float deltaTime)
//{
//    m_LifeTime += deltaTime;
//
//    // ★ 最初の1フレームだけ設定を適用
//    if (!m_HasSetup && m_Emitter)
//    {
//        m_Emitter->SetColorRange(m_Config.startColor, m_Config.endColor);
//        m_Emitter->SetSpeedRange(m_Config.minSpeed, m_Config.maxSpeed);
//        m_Emitter->SetSpreadAngle(m_Config.spreadAngle);
//        m_Emitter->SetGravity(m_Config.gravity);
//        m_Emitter->SetParticleSize(m_Config.particleSize);
//        m_HasSetup = true;
//    }
//
//    // ★ 最初の m_EmitDuration 秒だけパーティクルを放出
//    if (m_HasSetup && m_Emitter && m_LifeTime < m_EmitDuration)
//    {
//        DirectX::XMFLOAT3 m_ParticlePos = { m_Position.x, m_Position.y, m_Position.z };
//        DirectX::XMFLOAT3 dir = { m_Direction.x, m_Direction.y, m_Direction.z };
//
//        // 放出
//        for (int i = 0; i < m_ParticleCountToEmit; ++i)
//        {
//            m_Emitter->Emit(m_ParticlePos, dir);
//        }
//    }
//
//    // エフェクトの寿命終了
//    if (m_LifeTime >= m_MaxLifeTime)
//    {
//        m_IsActive = false;
//    }
//}

// ========================================
// EffectManager - 改良版
// ========================================
void EffectManager::Initialize()
{
    m_Effects.clear();
    m_Presets.clear();
    m_Emitters.clear();
    m_CurrentEmitterIndex = 0;

    // ★ エミッタ数を確認
    for (int i = 0; i < 5; ++i)
    {
        auto emitter = std::make_unique<SparkEmitter>();
        if (emitter->Init(Renderer::GetDevice()))
        {
            m_Emitters.push_back(std::move(emitter));

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
    printf( "合計 %zu 個のエミッタが作成されました\n", m_Emitters.size());
    OutputDebugStringA(buffer);

    EffectPresets::RegisterDefaultPresets();
}

void EffectManager::Finalize()
{
    ClearAllEffects();

    // ★ すべてのエミッタを解放
    for (auto& emitter : m_Emitters)
    {
        if (emitter)
        {
            emitter->Uninit();
            emitter.reset();
        }
    }
    m_Emitters.clear();
    m_Presets.clear();

    OutputDebugStringA("EffectManager 終了処理完了\n");
}

void EffectManager::Update(float deltaTime)
{
    // すべてのエフェクトを更新
    for (auto& effect : m_Effects)
    {
        if (effect && effect->IsActive())
        {
            effect->Update(deltaTime);
        }
    }

    // 非アクティブなエフェクトを削除
    RemoveInactiveEffects();

    // ★ すべてのエミッタを更新
    for (auto& emitter : m_Emitters)
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
    for (auto& effect : m_Effects)
    {
        if (effect && effect->IsActive())
        {
            effect->Draw(viewMatrix);
        }
    }

    for (auto& emitter : m_Emitters)
    {
        if(emitter)
        {
            // ★ viewProjを正しく作る
            DirectX::XMMATRIX view = Renderer::GetViewMatrix();
            DirectX::XMMATRIX proj = Renderer::GetProjectionMatrix();
            DirectX::XMMATRIX viewProj = view * proj;

            emitter->Render(context, view);
        }
    }
}

void EffectManager::RegisterPreset(const std::string& name, const EffectPreset& preset)
{
    m_Presets[name] = preset;
}

void EffectManager::SpawnEffect(const std::string& presetName, const Vector3& position,
    const Vector3& direction)
{
    auto it = m_Presets.find(presetName);
    if (it == m_Presets.end())
        return;

    const EffectPreset& preset = it->second;

    if (preset.type == EffectPreset::Type::Billboard)
    {
        auto effect = std::make_unique<BillboardEffect>(
            position, preset.width, preset.height,
            preset.texturePath.c_str(), preset.duration);
        effect->SetFadeOut(preset.fadeOut);
        m_Effects.push_back(std::move(effect));
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
        
        m_Effects.push_back(std::move(effect));
    }
}

void EffectManager::SpawnBillboardEffect(const Vector3& position, float width, float height,
    const wchar_t* texturePath, float duration, bool fadeOut)
{
    auto effect = std::make_unique<BillboardEffect>(
        position, width, height, texturePath, duration);
    effect->SetFadeOut(fadeOut);
    m_Effects.push_back(std::move(effect));
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
    m_Effects.push_back(std::move(effect));
}

void EffectManager::ClearAllEffects()
{
    m_Effects.clear();
}

int EffectManager::GetActiveEffectCount() const
{
    int count = 0;
    for (const auto& effect : m_Effects)
    {
        if (effect && effect->IsActive())
            ++count;
    }
    return count;
}

void EffectManager::RemoveInactiveEffects()
{
    m_Effects.erase(
        std::remove_if(m_Effects.begin(), m_Effects.end(),
            [](const std::unique_ptr<Effect>& effect) {
                return !effect || !effect->IsActive();
            }),
        m_Effects.end()
    );
}

// ★ 重要: 利用可能なエミッタを返す
SparkEmitter* EffectManager::GetAvailableEmitter()
{
    if (m_Emitters.empty())
    {
        OutputDebugStringA("エラー: エミッタリストが空です\n");
        return nullptr;
    }

    SparkEmitter* emitter = m_Emitters[m_CurrentEmitterIndex].get();
    m_CurrentEmitterIndex = (m_CurrentEmitterIndex + 1) % m_Emitters.size();

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

        // ★ 星エフェクト（敵が吹っ飛ぶ時）
        {
            EffectPreset preset;
            preset.type = EffectPreset::Type::Billboard;
            preset.texturePath = L"assets/texture/white_star.png";  // 星の画像
            preset.width = 10.5f;
            preset.height = 10.5f;
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
        // ★ 砂煙エフェクト
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