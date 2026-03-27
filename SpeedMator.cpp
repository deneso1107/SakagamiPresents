#include"SpeedMator.h"

SpeedMator::SpeedMator() : m_setSpeed(false), m_speedParam(0.0f),
m_isOverflowing(false), m_isVibrating(false), m_overflowRotation(0.0f),
m_vibrationOffset(0.0f)
{
    std::vector<const wchar_t*> textures = {
        L"assets/texture/SpeedMatorOut2.png",    // 0: メーター背景
        L"assets/texture/NeedleCow.png"  // 1: 針
    };

    auto billboard = std::make_unique<ScreenFixedBillboard>//メータ
        (
            Vector2(0.1f, 0.1f), 0.15f, 0.15f, textures[0]);
    m_matorPic.push_back(std::move(billboard));

    auto billboard2 = std::make_unique<ScreenFixedBillboard>//針
        (
            Vector2(0.1f, 0.17f), 0.15f, 0.15f, textures[1]);
    m_matorPic.push_back(std::move(billboard2));

    // 針の初期角度を設定(最小角度に設定)
    if (m_matorPic.size() > 1)
    {
        m_matorPic[1]->SetAngle(m_minAngle);
    }
}

void SpeedMator::Init()//コンストラクタで定義したほうが楽なのでそうしましょう
{
    std::vector<const wchar_t*> textures =
    {
       L"assets/texture/SpeedMatorOut.png",
       L"assets/texture/SoeedMatorNeedle.png"
    };
}

void SpeedMator::Update(float deltatime)
{
 
    if (m_setSpeed)
    {
        // 現在の表示速度を目標速度に向けて滑らかに補間
        m_currentDisplaySpeed = Lerp(m_currentDisplaySpeed, m_speedParam,
            m_smoothingSpeed * deltatime);

        // 振動時間の更新（常に更新して滑らかな振動を維持）
        m_vibrationTime += deltatime;

        // オーバーフロー判定と処理
        if (m_speedParam >= m_overflowThreshold)
        {
            // オーバーフロー状態に初めて入る時、現在の針角度を引き継ぐ
            if (!m_isOverflowing)
            {
                m_isOverflowing = true;
                m_overflowRotation = CalculateNeedleAngle(m_speedParam);
            }

            // 速度に応じて回転速度を計算
            float speedRatio = (m_speedParam - m_overflowThreshold) / (m_maxSpeed - m_overflowThreshold);
            speedRatio = std::clamp(speedRatio, 0.0f, 1.0f);

            // 回転速度を速度に応じて増加
            float currentRotationSpeed = m_baseRotationSpeed + speedRatio * (m_maxRotationSpeed - m_baseRotationSpeed);

            // 針を時計回りに連続回転
            m_overflowRotation -= currentRotationSpeed * deltatime;

            // 360度を超えたらリセット(連続回転のため)
            while (m_overflowRotation >= 360.0f)
            {
                m_overflowRotation -= 360.0f;
            }
        }
        else if (m_speedParam >= m_needleVibrationSpeed)
        {
            // 振動状態（中間地点）
            m_isVibrating = true;
            m_isOverflowing = false;
        }
        else
        {
            // 通常状態
            m_isOverflowing = false;
            m_isVibrating = false;
        }

        // 針の角度を更新
        if (m_matorPic.size() > 1)
        {
            float needleAngle;

            if (m_isOverflowing)
            {
                // オーバーフロー時は回転角度をそのまま使用
                needleAngle = m_overflowRotation;
            }
            else
            {
                // 通常時は速度に応じた角度
                needleAngle = CalculateNeedleAngle(m_speedParam);

                // 振動状態なら震え演出を追加
                if (m_isVibrating)
                {
                    // sin波で左右に震える（周波数を高くして細かく震わせる）
                    float vibration = sin(m_vibrationTime * m_vibrationFrequency) * m_vibrationAmplitude;
                    needleAngle += vibration;
                }
            }

            m_matorPic[1]->SetAngle(needleAngle);
            m_matorPic[1]->Update();
        }

        // 目標速度にほぼ到達したらフラグをリセット
        if (std::abs(m_currentDisplaySpeed - m_speedParam) < 0.01f)
        {
            m_setSpeed = false;
        }
    }
}

float SpeedMator::CalculateNeedleAngle(float speed)
{
    // 速度を0.0～1.0の範囲に正規化
    float normalizedSpeed = std::clamp(speed / m_maxSpeed, 0.0f, 1.0f);

    // 正規化された速度を-90度～90度の範囲にマッピング
    float angle = m_minAngle + normalizedSpeed * (m_maxAngle - m_minAngle);

    return angle;
}

void SpeedMator::UpdateNeedleRotation()
{
    if (m_matorPic.size() > 1) // 針のビルボードが存在するか確認
    {
        float needleAngle = CalculateNeedleAngle(m_speedParam);
        m_matorPic[1]->SetAngle(needleAngle);
        m_matorPic[1]->Update();
    }
}

void SpeedMator::SetSpeed(float speed)
{
    m_speedParam = speed;
    m_setSpeed = true;
}

void SpeedMator::Draw()
{
    for (auto& pic : m_matorPic)
    {
        pic->Draw();
    }
}