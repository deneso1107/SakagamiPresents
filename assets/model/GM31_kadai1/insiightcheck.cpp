#include	<Windows.h>
#include	"SimpleMath.h"
#include"insightcheck.h"
using namespace DirectX::SimpleMath;

//
// 視野範囲内に存在するかどうかを判定する
//
bool InsightCheckXZ(
		Vector3 eyepos, 				// カメラ位置
		Vector3 lookat,					// 注視点
		float fov,						// 視野角
		Vector3 checkpoint,				// チェックしたい座標
		float length) {					// どこまで見渡せるか

	// チェック対象と視点を結ぶベクトル
	Vector3 vecobj;
	vecobj = checkpoint - eyepos;//これが判定用の線

	// オブジェクトとの距離を求める
	float objlength = vecobj.Length();

	// 距離範囲外？
	if (objlength > length) {
		return false;
	}

	// 視線ベクトル
	Vector3 vecview;
	vecview = lookat - eyepos;//視野角との直線

	//objと視線ベクトルをつなぐ線
	Vector3 objview;
	objview = vecobj - vecview;

	Vector3 objaroundview;
	objaroundview = vecobj - vecobj;

	// 正規化
	vecview.Normalize();				// 視線ベクトルの正規化
	vecobj.Normalize();					// 判定対象オブジェクトへのベクトルを正規化
	//objview.Normalize();//Objと視線ベクトルを繋ぐベクトル

	// 内積を計算（視線ベクトルとターゲットベクトル）
	float dotobj = vecview.Dot(vecobj);	

	// 回転
	Matrix mtx;
	mtx = mtx.CreateRotationZ(fov / 2.0f);	// 視野角の半分を指定して行列を作成（Z軸回転）

	Vector3 vecrotview;
	vecrotview = vecview.Transform(vecview, mtx);	// 視線ベクトルを視野角の半分　回転させる

	// 内積を計算
	float dotrotview = vecview.Dot(vecrotview);		// 回転させた視線ベクトル（上限）との内積を計算

	// 視野角範囲内？
	if (dotrotview <= dotobj) {
		return true;
	}

	return false;
}
bool InsightCheckXZwithCircle
(
	Vector3 eyepos, 				// カメラ位置
	Vector3 lookat,					// 注視点
	float fov,						// 視野角
	Vector3 circlecenter,	// チェックしたい座標
	float radius,
	float length) {					// どこまで見渡せるか
	CalcPointLineDist* Line=new CalcPointLineDist;
	// チェック対象と視点を結ぶベクトル
	Vector3 vecobj;
	vecobj = circlecenter - eyepos;//これが判定用の線

	// オブジェクトとの距離を求める
	float objlength = vecobj.Length();

	// 距離範囲外？
	if (objlength > length) {
		return false;
	}
	//視野の上限ベクトルを求める
	Matrix mtx;
	//回転
	mtx = mtx.CreateRotationZ(fov / 2.0f);
	Vector3 lookatplus;
	Vector3 lookatmius;

	// 視線ベクトル
	Vector3 vecview;
	vecview = lookat - eyepos;//視野角との直線

	lookatplus = vecview.Transform(vecview, mtx);//視線ベクトルを視野角の半分　回転させる
	lookatplus += eyepos;
	//視野の下限ベクトルを求める
	mtx = mtx.CreateRotationZ(-fov / 2.0f);
	lookatmius = vecview.Transform(vecview, mtx);
	lookatmius += eyepos;
	//上限ベクトルとの点の距離を求める
	{
		Segment s;
		Vector3 intersectionpoint;
		float t = 0.0f;
		float distance = 0.0f;


		s.startpoint = eyepos;
		s.endpoint = lookatplus;
		//中心座標が視野範囲内か確認
		distance = Line->calcPointSegmentDist(circlecenter, s, intersectionpoint, t);
		if (distance < radius)
		{
			return true;
		}
	}
	//下限ベクトルとの点の距離を求める
	{
		Segment s;
		Vector3 intersectionpoint;
		float t = 0.0f;
		float distance = 0.0f;


		s.startpoint = eyepos;
		s.endpoint = lookatmius;

		distance = Line->calcPointSegmentDist(circlecenter, s, intersectionpoint,t);
		if (distance < radius)
		{
			return true;
		}
	}
	//return false;
	//中心座標が視野範囲内か確認
	bool sts = InsightCheckXZ
	(
		eyepos,
		lookat,
		fov,
		circlecenter,
		length);
	return sts;
}