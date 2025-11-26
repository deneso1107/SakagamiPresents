    #include "BaseRoad.h"
using namespace DirectX::SimpleMath;
// BaseRoad.cpp - 基底クラスの実装
void BaseRoad::Init() {
    // 既に初期化済みの場合は何もしない
    if (m_isInitialized) {
        return;
    }

    try {
        // モデルの初期化
        m_mesh.Load(GetModelFileName().c_str(), "assets/model/");
        m_terrainMesh.Load(GetModelFileName().c_str(), "assets/model/");

        // 空間分割を初期化
        m_terrainMesh.InitializeSpatialGrid(20.0f);
        m_spatialGridInitialized = true;

        // レンダラ初期化
        m_meshrenderer.Init(m_mesh);

        // シェーダーの初期化
        m_shader.Create("shader/unlitTextureVS.hlsl", "shader/unlitTexturePS.hlsl");
        m_shadowShader.Create(
            "shader/vertexLightingShadowVS.hlsl",
            "shader/vertexLightingShadowPS.hlsl");

        // 初期化完了フラグをセット
        m_isInitialized = true;

        

        // 既存のコードとの互換性のため、度数法の初期値を弧度法に変換
        // 例: 既存コードで m_Rotation = Vector3{0.0f, 0.0f, 90.0f} のような設定がある場合
        // m_Rotation = MathUtils::DegreesToRadians(m_Rotation);

    }
    catch (...) {
        // 初期化に失敗した場合のクリーンアップ
        m_spatialGridInitialized = false;
        m_isInitialized = false;
        throw; // 例外を再スロー
    }
}

void BaseRoad::Update(float deltatime) {
    // ワールド変換行列を更新
    SRT srt;
    srt.pos = m_Position;
    srt.rot = m_Rotation;
    srt.scale = m_Scale;
    Matrix4x4 worldMatrix = srt.GetMatrix();
    m_terrainMesh.SetWorldMatrix(worldMatrix);
}

void BaseRoad::Draw() {
    SRT srt;
    srt.pos = m_Position;
    srt.rot = m_Rotation;
    srt.scale = m_Scale;
    Matrix4x4 worldmtx = srt.GetMatrix();
    Matrix4x4 viewmtx = srt.CreateViewMatrix();

    Renderer::SetWorldMatrix(&worldmtx);
    // 通常パス：影を受け取る
    //if (Renderer::IsShadowMapEnabled())
    //{
    //    // 影付きシェーダーを使用
    //    m_shadowShader.SetGPU();
    //}
    //else
    //{
        // 影なしシェーダーを使用
        m_shader.SetGPU();
    //}
    m_meshrenderer.Draw();

    // デバッグ描画
    Color bscolor(1, 1, 1, 0.5f);
    SphereDrawerDraw(m_BoundingSphere.radius, bscolor, m_Position.x, m_Position.y, m_Position.z);
}

bool BaseRoad::IsPlayerOnEdge(const Vector3& playerPos, float edgeThreshold) const {
    // プレイヤー座標を道路のローカル座標系に変換
    Matrix rotationMatrix = Matrix::CreateRotationY(m_Rotation.y) *
        Matrix::CreateRotationX(m_Rotation.x) *
        Matrix::CreateRotationZ(m_Rotation.z);
    Matrix invTransform = rotationMatrix.Invert();

    Vector3 localPlayerPos = Vector3::Transform(playerPos - m_Position, invTransform);

    // 道路の実際のサイズを取得
    Vector3 actualSize = GetActualModelSize();
    Vector3 halfExtents = Vector3(
        actualSize.x * m_Scale.x * 0.5f,
        actualSize.y * m_Scale.y * 0.5f,
        actualSize.z * m_Scale.z * 0.5f
    );

    // まず道路上にいるか確認
    if (fabs(localPlayerPos.x) > halfExtents.x ||
        fabs(localPlayerPos.y) > halfExtents.y ||
        fabs(localPlayerPos.z) > halfExtents.z) {
        return false;  // 道路外
    }

    // X軸方向の端判定（道路の左右の端）
    float distanceFromLeftEdge = halfExtents.x - fabs(localPlayerPos.x);

    // Z軸方向の端判定（道路の前後の端）
    float distanceFromFrontEdge = halfExtents.z - fabs(localPlayerPos.z);

    // いずれかの端に近ければtrue
    return (distanceFromLeftEdge < edgeThreshold ||
        distanceFromFrontEdge < edgeThreshold);
}

EdgeType BaseRoad::GetPlayerEdgeType(const Vector3& playerPos, float edgeThreshold) const {
    // プレイヤー座標を道路のローカル座標系に変換
    Matrix rotationMatrix = Matrix::CreateRotationY(m_Rotation.y) *
        Matrix::CreateRotationX(m_Rotation.x) *
        Matrix::CreateRotationZ(m_Rotation.z);
    Matrix invTransform = rotationMatrix.Invert();

    Vector3 localPlayerPos = Vector3::Transform(playerPos - m_Position, invTransform);

    // 道路の実際のサイズを取得
    Vector3 actualSize = GetActualModelSize();
    Vector3 halfExtents = Vector3(
        actualSize.x * m_Scale.x * 0.5f,
        actualSize.y * m_Scale.y * 0.5f,
        actualSize.z * m_Scale.z * 0.5f
    );

    // 道路外チェック（おそらくY軸のせいで判定が取れていないので判定を緩和）
    float yTolerance = 10.0f;  // Y軸方向は10単位まで許容
    if (fabs(localPlayerPos.x) > halfExtents.x ||
        fabs(localPlayerPos.y) > (halfExtents.y + yTolerance) ||  // ← Y軸を緩和
        fabs(localPlayerPos.z) > halfExtents.z) {
        return EdgeType::NONE;
    }

    // 各端からの距離を計算
    bool nearLeft = (halfExtents.x + localPlayerPos.x) < edgeThreshold;   // 左端（-X側）
    bool nearRight = (halfExtents.x - localPlayerPos.x) < edgeThreshold;  // 右端（+X側）
    bool nearFront = (halfExtents.z - localPlayerPos.z) < edgeThreshold;  // 前端（+Z側）
    bool nearBack = (halfExtents.z + localPlayerPos.z) < edgeThreshold;   // 後端（-Z側）

    // 角の判定（2つの辺に近い）
    if ((nearLeft || nearRight) && (nearFront || nearBack)) {
        return EdgeType::CORNER;
    }

    // 各辺の判定
    if (nearLeft) return EdgeType::LEFT;
    if (nearRight) return EdgeType::RIGHT;
    if (nearFront) return EdgeType::FRONT;
    if (nearBack) return EdgeType::BACK;

    return EdgeType::NONE;  // 中央
}

void BaseRoad::Dispose() {
    // メッシュリソースの解放
    // 注意: CStaticMeshやCTerrainMeshにDisposeメソッドがある場合
    // m_mesh.Dispose();
    // m_terrainMesh.Dispose();

    // レンダラーの解放
    // m_meshrenderer.Dispose();

    // シェーダーの解放
    // m_shader.Release(); または m_shader.Dispose();

    // 空間分割の初期化フラグをリセット
    m_spatialGridInitialized = false;

    // その他のクリーンアップが必要なリソースがあればここに追加
}


