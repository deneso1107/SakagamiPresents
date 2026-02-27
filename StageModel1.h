#pragma once
#include "BaseStageModel.h"
class StageModel1 : public BaseStageModel
{
protected:
    std::string GetModelFileName() const override
    {
        return "assets/model/stage_select/stage1.fbx"; // assets/model/ ‚ÌŒã‚ë‚¾‚¯‘‚¯‚ÎOK
    }
public:
    StageModel1() {
        m_baseScale = Vector3(1.0f, 1.0f, 1.0f); // ‚±‚±‚ÅŒÂ•Ê‚ÉƒXƒP[ƒ‹’²®
    }
};