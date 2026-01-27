#pragma once
#include"BaseRoad.h"
class TurningRoad : public BaseRoad {
protected:
    std::string GetModelFileName() const override
    {
        return "assets/model/Road/Turning.fbx";
    }
public:
   TurningRoad(Direction dir = Direction::NORTH/*ƒeƒXƒg*/) : BaseRoad(RoadType::TURNING, dir) {}
};

