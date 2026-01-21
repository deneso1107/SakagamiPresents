#pragma once
#include "Camera.h"
#include "Player.h"

// リザルト画面専用カメラ
// プレイヤーの接近に合わせてカメラも動く演出用
class ResultCamera : public Camera {
private:
    // シングルトン
    ResultCamera() = default;
    ~ResultCamera() = default;

public:
    // コピー/ムーブ禁止
    ResultCamera(const ResultCamera&) = delete;
    ResultCamera& operator=(const ResultCamera&) = delete;
    ResultCamera(ResultCamera&&) = delete;
    ResultCamera& operator=(ResultCamera&&) = delete;

    static ResultCamera& Instance();

    void Init() override;
    void Update(float deltaTime) override;

    // プレイヤー設定
    void SetTargetPlayer(Player* player) { m_targetPlayer = player; }

    // カメラ演出の状態設定
    enum class CameraState
    {
        WideShot,      // プレイヤー接近中の引きのカメラ
        CloseUp,       // プレイヤーが正面に来た時のクローズアップ
        Result         // リザルト表示時の固定カメラ
    };

    void SetCameraState(CameraState state) { m_cameraState = state; }
    void SetStateTransitionProgress(float progress) { m_transitionProgress = progress; }
    void SetFixedCamera(bool fixed, const Vector3& pos, const Vector3& lookAt);

private:
    Player* m_targetPlayer = nullptr;
    CameraState m_cameraState = CameraState::WideShot;
    float m_transitionProgress = 0.0f;  // 状態遷移の進捗(0.0~1.0)

     bool m_useFixedCamera = false;  // 固定カメラモード
    Vector3 m_fixedCameraPos;       // 固定カメラの位置
    Vector3 m_fixedLookAt;          // 固定カメラの注視点

    // 各状態でのカメラパラメータ
    struct CameraParams
    {
        Vector3 offset;      // プレイヤーからの相対位置
        Vector3 lookAtOffset; // 注視点オフセット
        float fov;           // 視野角
    };

    CameraParams m_wideShotParams;
    CameraParams m_closeUpParams;
    CameraParams m_resultParams;

    // 現在のカメラパラメータ（補間後）
    CameraParams m_currentParams;

    // イージング関数
    float EaseInOutCubic(float t);
    Vector3 LerpVector3(const Vector3& start, const Vector3& end, float t);
};