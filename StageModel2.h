#pragma once
#include "BaseStageModel.h"
class StageModel2 : public BaseStageModel
{
protected:
    std::string GetModelFileName() const override
    {
        return "assets/model/stage_select/stage2.fbx"; // assets/model/ ‚ÌŒã‚ë‚¾‚¯‘‚¯‚ÎOK
    }

public:
    StageModel2() {
        m_baseScale = Vector3(1.0f, 1.0f, 1.0f); // ‚±‚±‚ÅŒÂ•Ê‚ÉƒXƒP[ƒ‹’²®
    }
};