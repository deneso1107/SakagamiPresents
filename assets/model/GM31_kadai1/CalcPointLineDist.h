#pragma once
#include<windows.h>
#include"SimpleMath.h"
#include"insightcheck.h"
struct Segment
{
	DirectX::SimpleMath::Vector3 startpoint;
	DirectX::SimpleMath::Vector3 endpoint;
};
class CalcPointLineDist
{
public:
	float calcPointSegmentDist(const DirectX::SimpleMath::Vector3&, const Segment&, DirectX::SimpleMath::Vector3&, float&);
};

