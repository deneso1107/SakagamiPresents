#pragma once
#include "system/CShader.h"
#include <d3d11.h>
#include <wrl/client.h>
//
//using Microsoft::WRL::ComPtr;
//
//class PostProcessManager
//{
//public:
//
//    static PostProcessManager& Instance()
//    {
//        static PostProcessManager instance;
//        return instance;
//    }
//
//    // 初期化・終了
//    void Initialize();
//    void Finalize();
//
//    // 各エフェクトの有効化
//    void EnableMotionBlur(float strength);
//    void EnableChromaticAberration(float strength, float duration);
//    void EnableShockwave(float intensity);
//
//    // 無効化
//    void DisableMotionBlur();
//    void DisableChromaticAberration();
//    void DisableShockwave();
//
//    // 更新
//    void Update(float deltaTime);
//
//    // シーン描画用テクスチャの設定（Sceneから呼ぶ）
//    void SetSceneTexture(ID3D11ShaderResourceView* srv)
//    {
//        m_sceneTexture = srv;
//    }
//
//    // 描画
//    void Render(ID3D11RenderTargetView* backBuffer);
//
//    // 取得関数
//    ID3D11RenderTargetView* GetSceneRenderTarget() const { return m_sceneRTV.Get(); }
//    ID3D11DepthStencilView* GetSceneDepthStencil() const { return m_sceneDSV.Get(); }
//    ID3D11ShaderResourceView* GetSceneShaderResourceView() const
//    {
//        return m_sceneSRV.Get();  // ★ ShaderResourceView を返す
//    }
//
//
//    struct MotionBlurSettings
//    {
//        bool enabled = false;
//        float strength = 0.0f;
//        float centerX = 0.5f;
//        float centerY = 0.5f;
//    } m_motionBlur;
//
//    struct ChromaticAberrationSettings
//    {
//        bool enabled = false;
//        float strength = 0.0f;
//        float duration = 0.0f;
//        float elapsedTime = 0.0f;
//        float time = 0.0f;
//    } m_chromatic;
//
//    struct ShockwaveSettings
//    {
//        bool enabled = false;
//        float intensity = 0.0f;
//        float progress = 0.0f;
//    } m_shockwave;
//
//private:
//    PostProcessManager() = default;
//    ~PostProcessManager() { Finalize(); }
//    PostProcessManager(const PostProcessManager&) = delete;
//    PostProcessManager& operator=(const PostProcessManager&) = delete;
//
//    // テクスチャ
//    ComPtr<ID3D11Texture2D> m_sceneTexture_internal;
//    ComPtr<ID3D11RenderTargetView> m_sceneRTV;
//    ComPtr<ID3D11DepthStencilView> m_sceneDSV;
//    ComPtr<ID3D11ShaderResourceView> m_sceneSRV;
//
//    ComPtr<ID3D11Texture2D> m_intermediateTexture;
//    ComPtr<ID3D11RenderTargetView> m_intermediateRTV;
//    ComPtr<ID3D11ShaderResourceView> m_intermediateSRV;
//
//    ID3D11ShaderResourceView* m_sceneTexture = nullptr;  // 外部から設定
//
//    // シェーダー
//    CShader m_motionBlurShader;
//    CShader m_chromaticShader;
//    CShader m_shockwaveShader;
//
//    // 中間テクスチャ（パス間のやり取り用）
//    ID3D11Texture2D* m_intermediateTexture;
//    ID3D11RenderTargetView* m_intermediateRTV;
//    ID3D11ShaderResourceView* m_intermediateSRV;
//
//    // サンプラー
//    ComPtr<ID3D11SamplerState> m_sampler;
//
//    // ステート
//    ComPtr<ID3D11DepthStencilState> m_depthStateDisabled;
//    ComPtr<ID3D11RasterizerState> m_rasterizerState;
//
//    // エフェクト設定
//
//    // テクスチャ作成
//    void CreateTextures();
//    void CreateShaders();
//    void CreateStates();
//
//    // パス処理
//    void ApplyMotionBlur(ID3D11DeviceContext* context,
//        ID3D11ShaderResourceView* input, ID3D11RenderTargetView* output);
//    void ApplyChromaticAberration(ID3D11DeviceContext* context,
//        ID3D11ShaderResourceView* input, ID3D11RenderTargetView* output);
//    void ApplyShockwave(ID3D11DeviceContext* context,
//        ID3D11ShaderResourceView* input, ID3D11RenderTargetView* output);
//
//    void SetupRenderState(ID3D11DeviceContext* context);
//    void RestoreRenderState(ID3D11DeviceContext* context, ID3D11DepthStencilState*, UINT, ID3D11RasterizerState*);
//};

