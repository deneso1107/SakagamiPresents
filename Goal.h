#pragma once
#include"ObjectBase.h"
#include"scenemanager.h"
class Goal :public ObjectBase
{
	public:
	// 描画の為の情報（メッシュに関わる情報）
	CStaticMeshRenderer m_meshrenderer;
	CStaticMesh         m_mesh;                         // メッシュデータ
	// 描画の為の情報（見た目に関わる部分）
	CShader m_shader;   // シェーダ
	void Init() override;
	void Draw() override;
	void Update(float) override;
	void Dispose() override;
	// SRT情報を取得
	SRT GetSRT() const override {
		return ObjectBase::GetSRT();
	}
};

