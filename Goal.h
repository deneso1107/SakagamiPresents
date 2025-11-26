#pragma once
#include"BaseRoad.h"
class Goal : public BaseRoad {
protected:
    std::string GetModelFileName() const override
    {
        return "assets/model/Road/road.fbx";
    }
public:
    Goal(Direction dir = Direction::NORTH/*ƒeƒXƒg*/) : BaseRoad(RoadType::GOAL_LINE, dir) {}
};

