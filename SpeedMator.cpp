#include"SpeedMator.h"
SpeedMator::SpeedMator() : m_SetSpeed(false), m_SpeedParam(0.0f)
{
    std::vector<const wchar_t*> textures = {
        L"assets/texture/SpeedMatorOut.png",    // 0: メーター背景
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

    // 針の初期角度を設定（最小角度に設定）
    if (m_MatorPic.size() > 1)
    {
        m_MatorPic[1]->SetAngle(m_MinAngle);
    }
}
void SpeedMator::Init()//コンストラクタで定義したほうが楽なのでそうしましょう
{

    std::vector<const wchar_t* > textures =
    {
       L"assets/texture/SpeedMatorOut.png",
       L"assets/texture/SoeedMatorNeedle.png"
    };

    //for (auto& tex : textures)
    //{
    //    // 直接構築 - 移動やコピーの問題を回避
    //    m_MatorPic.emplace_back(Vector2(0.1f, 0.1f), 0.15f, 0.15f, tex);
    //}
}
void SpeedMator::Update(uint64_t deltatime)
{
    if (m_SetSpeed)
    {
        // 現在の表示速度を目標速度に向けて滑らかに補間
        m_CurrentDisplaySpeed = Lerp(m_CurrentDisplaySpeed, m_SpeedParam,
            m_SmoothingSpeed * deltatime);

        // 補間された速度で針の角度を更新
        if (m_MatorPic.size() > 1)
        {
            float needleAngle = CalculateNeedleAngle(m_SpeedParam);
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
        m_MatorPic[1]->SetAngle(45.0f);
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
	//m_MatorPic[1]->Draw();
}

