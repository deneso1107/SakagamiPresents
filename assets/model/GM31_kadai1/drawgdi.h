#pragma once
#include	<Windows.h>
#include	"SimpleMath.h"

void DrawCircle(float radius, DirectX::SimpleMath::Vector3 center, HDC hdc);
void DrawLine(DirectX::SimpleMath::Vector3 start, DirectX::SimpleMath::Vector3 end, HDC hdc);
void SelectPen(HDC hdc, HPEN pen);
void DrawText(HDC hdc, DirectX::SimpleMath::Vector3 point, char* text);