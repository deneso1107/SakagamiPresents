
#include "Camera.h"
#include "Player.h"

// 比較用: 何の補間もない、ただプレイヤーの後ろにくっついてくるだけのカメラ
class SimpleFollowCamera : public Camera
{
private:
    Player* m_targetPlayer;

    // カメラの固定パラメータ
    float m_distance = 75.0f;  // プレイヤーからの距離
    float m_height = 6.0f;     // 高さ

public:
    SimpleFollowCamera();
    virtual ~SimpleFollowCamera() = default;

    static SimpleFollowCamera& Instance();

    void Init() override;
    void Update(float deltaTime);
    void Draw() override;

    void SetTargetPlayer(Player* player);
    void SetDistance(float distance) { m_distance = distance; }
    void SetHeight(float height) { m_height = height; }

    Vector3 GetForward() const;
};
