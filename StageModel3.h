#pragma once
#pragma once
#include "BaseStageModel.h"
class StageModel3 : public BaseStageModel
{
protected:
    std::string GetModelFileName() const override
    {
        return "assets/model/stage_select/stage3.fbx"; // assets/model/ ‚ÌŒã‚ë‚¾‚¯‘‚¯‚ÎOK
    }
public:
    StageModel3() {
        m_baseScale = Vector3(0.5f, 0.5f, 0.5f); // ‚±‚±‚ÅŒÂ•Ê‚ÉƒXƒP[ƒ‹’²®
    }
};