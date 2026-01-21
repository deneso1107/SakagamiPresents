#pragma once
#include"BaseRoad.h"
class RightTurnRoad : public BaseRoad 
{
protected:
    std::string GetModelFileName() const override
    {
        return "assets/model/Road/road.fbx";
    }
public:
    RightTurnRoad(Direction dir = Direction::NORTH/*ƒeƒXƒg*/) : BaseRoad(RoadType::TURN_RIGHT, dir) {}
};