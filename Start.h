#pragma once
#include"BaseRoad.h"
class Start : public BaseRoad {
protected:
    std::string GetModelFileName() const override
    {
        return "assets/model/Road/road.fbx";
    }
public:
    Start(Direction dir = Direction::NORTH/*ƒeƒXƒg*/) : BaseRoad(RoadType::START_LINE, dir) {}
};
