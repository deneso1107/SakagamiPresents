#pragma once
#include"BaseRoad.h"
class Dirt : public BaseRoad {
protected:
    std::string GetModelFileName() const override
    {
        return "assets/model/Road/dirt.fbx";
    }
public:
   Dirt(Direction dir = Direction::NORTH/*ƒeƒXƒg*/) : BaseRoad(RoadType::DIRT, dir) {}
};


