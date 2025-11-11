#pragma once
#include"BaseRoad.h"
class StraightRoad : public BaseRoad {
protected:
    std::string GetModelFileName() const override
    {
        return "assets/model/Road/road.fbx";
    }
public:
    StraightRoad(Direction dir = Direction::NORTH/*ƒeƒXƒg*/) : BaseRoad(RoadType::STRAIGHT, dir) {}
};

