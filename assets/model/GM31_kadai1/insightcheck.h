#pragma once
#include	"SimpleMath.h"
#include"CalcPointLineDist.h"

bool InsightCheckXZ(DirectX::SimpleMath::Vector3 eyepos, DirectX::SimpleMath::Vector3 lookat, float fov, DirectX::SimpleMath::Vector3 checkpoint, float length);
bool InsightCheckXZwithCircle(DirectX::SimpleMath::Vector3 eyepos, DirectX::SimpleMath::Vector3 lookat, float fov, DirectX::SimpleMath::Vector3 circlecenter, float radius, float length);