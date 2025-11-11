#include	<Windows.h>
#include	"SimpleMath.h"

using namespace DirectX::SimpleMath;

//
// 視野範囲内に存在するかどうかを判定する
//
bool InsightCheckXZwithCircle
(
	Vector3 eyepos, 				// カメラ位置
	Vector3 lookat,					// 注視点
	float fov,						// 視野角
	Vector3 circlecenter,	// チェックしたい座標
	float radius,
	float length) {					// どこまで見渡せるか

	// チェック対象と視点を結ぶベクトル
	Vector3 vecobj;
	vecobj = circlecenter - eyepos;//これが判定用の線

	// オブジェクトとの距離を求める
	float objlength = vecobj.Length();

	// 距離範囲外？
	if (objlength > length) {
		return false;
	}

	// 視線ベクトル
	Vector3 vecview;
	vecview = lookat - eyepos;//視野角との直線

	Vector3 lookatplus = vecview.Transform(vecview, mtx);
	lookatplus += eyepos;

	mtx = mtx.CreateRotationZ(-fov / 2.0f);
	lookatmius = vecview.Transform(vecview, mtx);
	lookatmius -= eyepos;
	{
		Segment s;
		Vector3 intersectionpoint;
		float t = 0.0f;
		float distance = 0.0f;


		s.startpoint = eyepos;
		s.endpoint = lookatplus;

		distance = calcPointSegmentDist(circlecenter, s, intersectionpoint,t);
		if (distance < radius)
		{
			return true;
		}
	}
	lookatmius -= eyepos;
	{
		Segment s;
		Vector3 intersectionpoint;
		float t = 0.0f;
		float distance = 0.0f;


		s.startpoint = eyepos;
		s.endpoint = lookatmius;

		distance = calcPointSegmentDist(circlecenter, s, intersectionpoint);
		if (distance < radius)
		{
			return true;
		}
	}
	sts = InsightcheckXZ(
		eyepos,
		lookat,
		fov,
		circlecenter,
		length);
	retiurn sts;
}
