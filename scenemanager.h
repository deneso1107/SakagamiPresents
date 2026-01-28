#pragma once
#include <memory>
#include <unordered_map>
#include <string>
#include <thread>        
#include <atomic>       
#include <mutex>         
#include "system/noncopyable.h"
#include "system/CShader.h"
#include"ScreenFixedBillboard.h"
#include <d3d11.h>

// 前方宣言
class IScene;
extern int m_gameScore; // ゲームのスコア

class SceneManager : NonCopyable {
private:
    // トランジション用
    static CShader m_transitionShader;
    static ID3D11Buffer* m_transitionVertexBuffer;
    static ID3D11Buffer* m_transitionConstantBuffer;
    static ID3D11ShaderResourceView* m_transitionTexture;
    static ID3D11ShaderResourceView* m_blackFadeTexture; // 黒画像用
    static ID3D11ShaderResourceView* m_loadingTextTexture;
    static ID3D11ShaderResourceView* m_cowIconTexture;
    static ID3D11DepthStencilState* m_transitionDepthState;
    static ID3D11BlendState* m_transitionBlendState;
    static ID3D11SamplerState* m_transitionSamplerState;
    static ScreenFixedBillboard* m_BillboardLoad;
    static ScreenFixedBillboard* m_BillboardCowIcon;

    // トランジション用頂点構造体
    struct TransitionVertex {
        float x, y, z;
        float u, v;
    };

    // トランジション用定数バッファ
    struct TransitionConstantBuffer {
        float slideOffset;      // スライド位置
        float imageScale;       // スケール
        float imageYPosition;   // Y位置（揺れ用）
        float fadeAlpha;
    };

    // シーン関連
    static std::unordered_map<std::string, std::unique_ptr<IScene>> m_scenes;
    static std::string m_currentSceneName;
    static std::string m_nextSceneName;
    static bool m_sceneChangeRequested;

    // トランジション管理
    enum class TransitionState {
        None,
        SlideIn,      // 右から中央へ
        Loading,      // 中央で停止、ロード中
        SlideOut      // 中央から左へ
    };

    static TransitionState m_transitionState;
    static float m_slideOffset;        // 現在のスライド位置
    static float m_transitionSpeed;    // スライド速度
    static bool m_sceneLoaded;         // シーンのロード完了フラグ
    static float m_loadingRotation;    // ローディングアニメーション用

    //フェード用
    static CShader m_blackfadeShader;
    static float m_fadeAlpha;          // 背景フェードのアルファ値 (0.0=透明, 1.0=不透明)

    // 内部処理
    static void ProcessSceneChange();
    static void UpdateTransition(float deltaTime);
    static void LoadNextSceneAsync();


    static std::thread m_loadingThread;
    static std::atomic<bool> m_asyncLoading;
    static std::atomic<bool> m_asyncFinished;

public:
    // 基本機能
    static void Init();
    static void Update(float deltaTime);
    static void Draw(float deltaTime);
    static void Dispose();

    // トランジション関連
    static void InitTransitionResources();
    static void DisposeTransitionResources();
    static void DrawTransitionOverlay();
    static void DrawBlackFade();  // 黒背景フェード描画
    static void DrawLoadingIndicator();
    static void LoadTransitionTexture(const wchar_t* filepath );


    // シーン管理
    template<typename SceneType>
    static void RegisterScene(const std::string& name) {
        m_scenes[name] = std::make_unique<SceneType>();
    }

    static void SetCurrentScene(const std::string& sceneName);
    static void ChangeScene(const std::string& sceneName, bool useTransition = true);
    static std::string GetCurrentSceneName() { return m_currentSceneName; }
    static bool IsSceneChangeInProgress() {
        return m_sceneChangeRequested || m_transitionState != TransitionState::None;
    }

    // トランジション制御
    static void SetTransitionSpeed(float speed) { m_transitionSpeed = speed; }
    static bool IsTransitioning() { return m_transitionState != TransitionState::None; }

    // デバッグ・ユーティリティ
    static std::vector<std::string> GetRegisteredSceneNames();
    static bool IsSceneRegistered(const std::string& sceneName);
};