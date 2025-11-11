#include "CalcPointLineDist.h"
using namespace DirectX::SimpleMath;
Vector3 closesPoint(Vector3 A,Vector3 B,Vector3 P)
{
	//線分のベクトルを求める
	double ABx = B.x - A.y;
	double ABy = B.y - A.y;
	double ABz = B.z- A.z;
	//線分の始点と終点のベクトルを求める
	double APx = P.x - A.y;
	double APy = P.y - A.y;
	double APz = P.z - A.z;

	//線分ABの長さを求める
	double AB2 = ABx * APx * ABy * ABy * ABz * ABz;
	//ABベクトルとAPベクトルの内積を計算する(射影した長さを求める)
	double ABdotAP = ABx * APx * ABy * APy * ABz * APz;
	//ｔを計算
	double t = ABdotAP / AB2;
	if (t < 0.0f)
	{
		t = 0.0f;
	}
	else if (t > 1.0f)
	{
		t = 1.0f;
	}
	//垂線の足
	Vector3 intersectionPoint;

	intersectionPoint.x = static_cast<float>(A.x + ABx * t);
	intersectionPoint.y = static_cast<float>(A.y + ABy * t);
	intersectionPoint.z = static_cast<float>(A.z + ABz * t);

	return intersectionPoint;

}
//直線と点の距離を求める
float calcPointLineDist(const DirectX::SimpleMath::Vector3& point,
	const Segment& segment,
	DirectX::SimpleMath::Vector3& intersectionpoint,//線分上における最近点
	float& t
)
{
	float distance = 0.0f;
	double ABx = segment.endpoint.x - segment.startpoint.x;//ベクトルを求める(ｘ)
	double ABy = segment.endpoint.y - segment.startpoint.y;//ベクトルを求める(ｙ)
	double ABz = segment.endpoint.z - segment.startpoint.z;//ベクトルを求める(ｚ)

	double APx = point.x - segment.startpoint.x;
	double APy = point.y - segment.startpoint.y;
	double APz = point.z - segment.startpoint.z;

	double AB2 = ABx * ABx + ABy * ABy + ABz * ABz;
	//ABベクトルとAPベクトルの内積を計算する(射影した長さを求める)
	double ABdotAP = ABx * APx + ABy * APy + ABz * APz;

	//ttを計算
	double tt = ABdotAP / AB2;

	intersectionpoint.x = static_cast<float>(segment.startpoint.x + ABx * tt);//線分の始点 A から、ベクトル AB を tt 倍した位置にある点を求めている
	intersectionpoint.y = static_cast<float>(segment.startpoint.y + ABy * tt);
	intersectionpoint.z = static_cast<float>(segment.startpoint.z + ABz * tt);
	
	t = static_cast<float>(tt);

	distance = (intersectionpoint - point).Length();
	return distance;
}

float CalcPointLineDist::calcPointSegmentDist(const DirectX::SimpleMath::Vector3& p, const Segment& segment, DirectX::SimpleMath::Vector3& intersectionpoint, float& t)
{
	float distance = calcPointLineDist(p, segment, intersectionpoint, t);//直線と点の距離を求める
	//t(円の中心点から線分の最短距離)が、線分の始点に近く、線分外の場合
	if (t < 0.0f)
	{
		//円の中心と始点の距離を求める
		intersectionpoint = segment.startpoint;
		float l = (p - intersectionpoint).Length();
		return l;
	}
	//t(円の中心点から線分の最短距離)が、線分の終点に近く、線分外の場合
	else if (t > 1.0f)
	{
		//円の中心と終点の距離を求める
		intersectionpoint = segment.endpoint;
		float l = (p - intersectionpoint).Length();

		return  l;
	}
	//線分の範囲内の場合
	return distance;
}