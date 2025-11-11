#pragma once
#include	"CommonTypes.h"

/**
 * @struct SRT
 * @brief スケール、回転、平行移動（Translation）情報を格納する構造体
 */
struct SRT {
	Vector3 scale = { 1.0f,1.0f,1.0f };  ///< スケール情報
	Vector3 rot = { 0.0f,0.0f,0.0f };    ///< 回転情報
	Vector3 pos = { 0.0f,0.0f,0.0f };    ///< 平行移動（位置）情報

	Matrix4x4 GetMatrix() const {
		return Matrix4x4::CreateScale(scale) *
			Matrix4x4::CreateFromYawPitchRoll(rot.y, rot.x, rot.z) *
			Matrix4x4::CreateTranslation(pos);
	}
	Matrix4x4 CreateViewMatrix()
	{
		Vector3 forward = Vector3::Transform(Vector3(0.0f, 0.0f, -1.0f),
			Matrix4x4::CreateFromYawPitchRoll(rot.y, rot.x, rot.z));
		Vector3 target = pos + forward;
		Vector3 up = Vector3(0.0f, 1.0f, 0.0f);

		return Matrix4x4::CreateLookAt(pos, target, up);
	}
};
