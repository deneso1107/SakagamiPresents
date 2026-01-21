#pragma once
#include "Camera.h"
#include "Player.h"

// タイトル画面専用カメラ
// プレイヤーの前方（-Z方向）から螺旋を見る固定カメラ
class TitleCamera : public Camera {
private:
    // シングルトン
    TitleCamera() = default;
    ~TitleCamera() = default;

public:
    // コピー/ムーブ禁止
    TitleCamera(const TitleCamera&) = delete;
    TitleCamera& operator=(const TitleCamera&) = delete;
    TitleCamera(TitleCamera&&) = delete;
    TitleCamera& operator=(TitleCamera&&) = delete;

    static TitleCamera& Instance();

    void Init() override;
    void Update(float deltaTime) override;

    // プレイヤー設定
    void SetTargetPlayer(Player* player) { m_targetPlayer = player; }

    // カメラオフセット設定（プレイヤー相対位置）
    void SetCameraOffset(const Vector3& offset) { m_cameraOffset = offset; }
    void SetLookAtOffset(const Vector3& offset) { m_lookAtOffset = offset; }

    // 固定カメラモード切り替え
    void SetFixedMode(bool fixed) { m_isFixedMode = fixed; }

private:
    Player* m_targetPlayer = nullptr;

    // カメラオフセット（プレイヤーからの相対位置）
    Vector3 m_cameraOffset;     // カメラ位置のオフセット
    Vector3 m_lookAtOffset;     // 注視点のオフセット

    bool m_isFixedMode = true;  // 固定モード（プレイヤーを追従しない）


	float m_fov = 60.0f;        // 視野角

    // 固定カメラ用
    Vector3 m_fixedPosition;
    Vector3 m_fixedLookAt;
};