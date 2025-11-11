#include	<Windows.h>
#include	"SimpleMath.h"

using namespace DirectX::SimpleMath;

void DrawCircle(float radius, Vector3 center, HDC hdc) {
	Ellipse(hdc, 
		static_cast<int>(center.x - radius), 
		static_cast<int>(center.y - radius),
		static_cast<int>(center.x + radius),
		static_cast<int>(center.y + radius));
}

void DrawLine(Vector3 start,Vector3 end,HDC hdc) {
	MoveToEx(hdc, static_cast<int>(start.x), static_cast<int>(start.y), nullptr);
	LineTo(hdc, static_cast<int>(end.x), static_cast<int>(end.y));
}

void SelectPen(HDC hdc, HPEN pen) {
	SelectObject(hdc, pen);
}

void DrawText(HDC hdc, Vector3 point, char* text) {
	size_t len;
	len = strlen(text);
	TextOutA(hdc, static_cast<int>(point.x), static_cast<int>(point.y), text, static_cast<int>(len));
}