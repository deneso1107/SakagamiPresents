#pragma once
#include <memory>
#include <unordered_map>
#include <string>
#include "system/noncopyable.h"
#include "system/CShader.h"
#include <d3d11.h>

// 前方宣言
class IScene;

extern int m_gameScore; // ゲームのスコア  
class SceneManager : NonCopyable {
private:

    // 既存のメンバ変数...
    static CShader m_fadeShader;
    static ID3D11Buffer* m_fadeVertexBuffer;
    static ID3D11Buffer* m_fadeConstantBuffer;

    // フェード用頂点構造体
    struct FadeVertex {
        float x, y, z;
        float u, v;
    };

    // フェード用定数バッファ構造体
    struct FadeConstantBuffer {
        float fadeAlpha;
        float padding[3];
    };

    // シーン関連
    static std::unordered_map<std::string, std::unique_ptr<IScene>> m_scenes;
    static std::string m_currentSceneName;
    static std::string m_nextSceneName;
    static bool m_sceneChangeRequested;

    // オブジェクト管理
    //static std::vector<std::unique_ptr<Object>> m_objects;

    // フェード管理
    static bool m_fadeInProgress;
    static float m_fadeAlpha;
    static float m_fadeSpeed;

    // 内部処理
    static void ProcessSceneChange();
    static void UpdateFade(float deltaTime);

    // クラス外部で宣言する必要があります
    static int m_GameScore; // ゲームのスコア

public:
    // 基本機能
    static void Init();
    static void Update(float deltaTime);
    static void Draw(float deltaTime);
    static void Dispose();
    static void DrawFadeOverlay();

    static void InitFadeResources();
    static void DisposeFadeResources();
    // シーン管理
    template<typename SceneType>
    static void RegisterScene(const std::string& name) {
        m_scenes[name] = std::make_unique<SceneType>();
    }

    static void SetCurrentScene(const std::string& sceneName);
    static void ChangeScene(const std::string& sceneName, bool useFade = true);
    static std::string GetCurrentSceneName() { return m_currentSceneName; }
    static bool IsSceneChangeInProgress() { return m_sceneChangeRequested || m_fadeInProgress; }

    // オブジェクト管理
   /* template<typename T, typename... Args>
    static T* AddObject(Args&&... args) {
        auto obj = std::make_unique<T>(std::forward<Args>(args)...);
        T* ptr = obj.get();
        m_objects.emplace_back(std::move(obj));
        return ptr;
    }

    template<typename T>
    static std::vector<T*> GetObjects() {
        std::vector<T*> result;
        for (auto& obj : m_objects) {
            if (T* derivedObj = dynamic_cast<T*>(obj.get())) {
                result.emplace_back(derivedObj);
            }
        }
        return result;
    }*/

   /* template<typename T>
    static T* GetFirstObject() {
        for (auto& obj : m_objects) {
            if (T* derivedObj = dynamic_cast<T*>(obj.get())) {
                return derivedObj;
            }
        }
        return nullptr;
    }*/

   // static void RemoveObject(Object* obj);
    static void ClearObjects();
    //static size_t GetObjectCount() { return m_objects.size(); }

    // フェード制御
    static void SetFadeSpeed(float speed) { m_fadeSpeed = speed; }
    static float GetFadeAlpha() { return m_fadeAlpha; }
    static bool IsFading() { return m_fadeInProgress; }

    // デバッグ・ユーティリティ
    static std::vector<std::string> GetRegisteredSceneNames();
    static bool IsSceneRegistered(const std::string& sceneName);
};
//
//// 前方宣言
//class IScene;
//
//class SceneManager : NonCopyable{
//
//	static std::unordered_map<std::string, std::unique_ptr<IScene>> m_scenes;
//	static std::string m_currentSceneName;
//
//public:
//	static void SetCurrentScene(std::string);
//	static void Dispose();
//	static void Init();
//	static void Update(uint64_t deltatime);
//	static void Draw(uint64_t deltatime);
//};
