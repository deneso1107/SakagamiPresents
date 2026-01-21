#include"SpeedMator.h"

SpeedMator::SpeedMator() : m_SetSpeed(false), m_SpeedParam(0.0f),
m_IsOverflowing(false), m_IsVibrating(false), m_OverflowRotation(0.0f),
m_VibrationOffset(0.0f)
{
    std::vector<const wchar_t*> textures = {
        L"assets/texture/SpeedMatorOut2.png",    // 0: メーター背景
        L"assets/texture/NeedleCow.png"  // 1: 針
    };

    auto billboard = std::make_unique<ScreenFixedBillboard>//メータ
        (
            Vector2(0.1f, 0.1f), 0.15f, 0.15f, textures[0]);
    m_MatorPic.push_back(std::move(billboard));

    auto billboard2 = std::make_unique<ScreenFixedBillboard>//針
        (
            Vector2(0.1f, 0.17f), 0.15f, 0.15f, textures[1]);
    m_MatorPic.push_back(std::move(billboard2));

    // 針の初期角度を設定(最小角度に設定)
    if (m_MatorPic.size() > 1)
    {
        m_MatorPic[1]->SetAngle(m_MinAngle);
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
 
    if (m_SetSpeed)
    {
        // 現在の表示速度を目標速度に向けて滑らかに補間
        m_CurrentDisplaySpeed = Lerp(m_CurrentDisplaySpeed, m_SpeedParam,
            m_SmoothingSpeed * deltatime);

        // 振動時間の更新（常に更新して滑らかな振動を維持）
        m_VibrationTime += deltatime;

        // オーバーフロー判定と処理
        if (m_SpeedParam >= m_OverflowThreshold)
        {
            // オーバーフロー状態に初めて入る時、現在の針角度を引き継ぐ
            if (!m_IsOverflowing)
            {
                m_IsOverflowing = true;
                m_OverflowRotation = CalculateNeedleAngle(m_SpeedParam);
            }

            // 速度に応じて回転速度を計算
            float speedRatio = (m_SpeedParam - m_OverflowThreshold) / (m_MaxSpeed - m_OverflowThreshold);
            speedRatio = std::clamp(speedRatio, 0.0f, 1.0f);

            // 回転速度を速度に応じて増加
            float currentRotationSpeed = m_BaseRotationSpeed + speedRatio * (m_MaxRotationSpeed - m_BaseRotationSpeed);

            // 針を時計回りに連続回転
            m_OverflowRotation -= currentRotationSpeed * deltatime;

            // 360度を超えたらリセット(連続回転のため)
            while (m_OverflowRotation >= 360.0f)
            {
                m_OverflowRotation -= 360.0f;
            }
        }
        else if (m_SpeedParam >= m_NeedleVibrationSpeed)
        {
            // 振動状態（中間地点）
            m_IsVibrating = true;
            m_IsOverflowing = false;
        }
        else
        {
            // 通常状態
            m_IsOverflowing = false;
            m_IsVibrating = false;
        }

        // 針の角度を更新
        if (m_MatorPic.size() > 1)
        {
            float needleAngle;

            if (m_IsOverflowing)
            {
                // オーバーフロー時は回転角度をそのまま使用
                needleAngle = m_OverflowRotation;
            }
            else
            {
                // 通常時は速度に応じた角度
                needleAngle = CalculateNeedleAngle(m_SpeedParam);

                // 振動状態なら震え演出を追加
                if (m_IsVibrating)
                {
                    // sin波で左右に震える（周波数を高くして細かく震わせる）
                    float vibration = sin(m_VibrationTime * m_VibrationFrequency) * m_VibrationAmplitude;
                    needleAngle += vibration;
                }
            }

            m_MatorPic[1]->SetAngle(needleAngle);
            m_MatorPic[1]->Update();
        }

        // 目標速度にほぼ到達したらフラグをリセット
        if (std::abs(m_CurrentDisplaySpeed - m_SpeedParam) < 0.01f)
        {
            m_SetSpeed = false;
        }
    }
}

float SpeedMator::CalculateNeedleAngle(float speed)
{
    // 速度を0.0～1.0の範囲に正規化
    float normalizedSpeed = std::clamp(speed / m_MaxSpeed, 0.0f, 1.0f);

    // 正規化された速度を-90度～90度の範囲にマッピング
    float angle = m_MinAngle + normalizedSpeed * (m_MaxAngle - m_MinAngle);

    return angle;
}

void SpeedMator::UpdateNeedleRotation()
{
    if (m_MatorPic.size() > 1) // 針のビルボードが存在するか確認
    {
        float needleAngle = CalculateNeedleAngle(m_SpeedParam);
        m_MatorPic[1]->SetAngle(needleAngle);
        m_MatorPic[1]->Update();
    }
}

void SpeedMator::SetSpeed(float speed)
{
    m_SpeedParam = speed;
    m_SetSpeed = true;
}

void SpeedMator::Draw()
{
    for (auto& pic : m_MatorPic)
    {
        pic->Draw();
    }
}