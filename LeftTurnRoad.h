#pragma once
#include"BaseRoad.h"
class LeftTurnRoad : public BaseRoad {
protected:
    std::string GetModelFileName() const override
    {

        return "assets/model/Road/Curve99.fbx ";
    }
public:
    LeftTurnRoad(Direction dir = Direction::NORTH/*ƒeƒXƒg*/) : BaseRoad(RoadType::TURN_LEFT, dir) {}
};