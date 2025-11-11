#pragma once
#include"BaseRoad.h"
class LeftTurnRoad : public BaseRoad {
protected:
    std::string GetModelFileName() const override
    {

        return "assets/model/Road/Curveroad.fbx";
    }
public:
    LeftTurnRoad(Direction dir = Direction::NORTH/*ƒeƒXƒg*/) : BaseRoad(RoadType::STRAIGHT, dir) {}
};