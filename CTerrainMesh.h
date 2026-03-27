#pragma once
#include "system/CStaticMesh.h"
#include "system/collision.h"
#include <set>

using namespace GM31::GE::Collision;

// Matrix変換用のヘルパー関数
class MatrixHelper 
{
public:
    // 座標点の変換（w=1として扱う）
    static Vector3 TransformCoord(const Vector3& v, const Matrix4x4& m) {
        Vector4 v4(v.x, v.y, v.z, 1.0f);
        Vector4 result = Vector4::Transform(v4, m);

        // w で除算して正規化（透視変換対応）
        if (result.w != 0.0f) {
            return Vector3(result.x / result.w, result.y / result.w, result.z / result.w);
        }
        return Vector3(result.x, result.y, result.z);
    }

    // 法線ベクトルの変換（w=0として扱う）
    static Vector3 TransformNormal(const Vector3& v, const Matrix4x4& m) {
        Vector4 v4(v.x, v.y, v.z, 0.0f);
        Vector4 result = Vector4::Transform(v4, m);
        return Vector3(result.x, result.y, result.z);
    }

    // 方向ベクトルの変換（TransformNormalと同じだが、意味を明確にするため）
    static Vector3 TransformDirection(const Vector3& v, const Matrix4x4& m) {
        return TransformNormal(v, m);
    }
};

// 最適化用の空間分割構造
struct TerrainCell
{
    std::vector<int> triangleIndices;
    float minHeight = FLT_MAX;    // 初期化を追加
    float maxHeight = -FLT_MAX;   // 初期化を追加
};

// 地形当たり判定用の拡張クラス
class CTerrainMesh : public CStaticMesh
{
private:
    struct TriangleHit {
        float height;
        Vector3 normal;
        float distance;
    };

    struct TriangleCandidate {
        float height;
        Vector3 normal;
        float distance;
    };

    std::vector<std::vector<TerrainCell>> m_spatialGrid;
    int m_gridWidth, m_gridHeight;
    float m_cellSize;
    Vector3 m_minBounds, m_maxBounds;

    // 座標変換用の行列
    Matrix4x4 m_worldMatrix;
    Matrix4x4 m_invWorldMatrix;

public:
    // ワールド変換行列を設定
    void SetWorldMatrix(const Matrix4x4& worldMatrix) {
        m_worldMatrix = worldMatrix;

        // 逆行列計算前に行列式をチェック
        float determinant = worldMatrix.Determinant();
        if (abs(determinant) < 1e-6f) {
            // 行列式が0に近い場合は逆行列が存在しない
            m_invWorldMatrix = Matrix4x4::Identity;
            OutputDebugStringA("Warning: Matrix is not invertible (determinant too small)\n");
        }
        else
        {
            // 逆行列を計算
            worldMatrix.Invert(m_invWorldMatrix);
        }
    }

    // 空間分割の初期化
    void InitializeSpatialGrid(float cellSize)
    {
        m_cellSize = cellSize;

        // 地形の境界を計算
        m_minBounds = Vector3(FLT_MAX, FLT_MAX, FLT_MAX);
        m_maxBounds = Vector3(-FLT_MAX, -FLT_MAX, -FLT_MAX);

        for (const auto& vertex : m_vertices) {
            if (vertex.Position.x < m_minBounds.x) m_minBounds.x = vertex.Position.x;
            if (vertex.Position.y < m_minBounds.y) m_minBounds.y = vertex.Position.y;
            if (vertex.Position.z < m_minBounds.z) m_minBounds.z = vertex.Position.z;
            if (vertex.Position.x > m_maxBounds.x) m_maxBounds.x = vertex.Position.x;
            if (vertex.Position.y > m_maxBounds.y) m_maxBounds.y = vertex.Position.y;
            if (vertex.Position.z > m_maxBounds.z) m_maxBounds.z = vertex.Position.z;
        }

        // グリッドサイズを計算
        m_gridWidth = (int)ceil((m_maxBounds.x - m_minBounds.x) / m_cellSize);
        m_gridHeight = (int)ceil((m_maxBounds.z - m_minBounds.z) / m_cellSize);

        // グリッドを初期化
        m_spatialGrid.resize(m_gridHeight);
        for (int i = 0; i < m_gridHeight; i++) {
            m_spatialGrid[i].resize(m_gridWidth);
            for (int j = 0; j < m_gridWidth; j++) {
                m_spatialGrid[i][j].minHeight = FLT_MAX;
                m_spatialGrid[i][j].maxHeight = -FLT_MAX;
            }
        }

        // 三角形をグリッドに分類
        for (size_t i = 0; i < m_indices.size(); i += 3) {
            Vector3 v0 = m_vertices[m_indices[i]].Position;
            Vector3 v1 = m_vertices[m_indices[i + 1]].Position;
            Vector3 v2 = m_vertices[m_indices[i + 2]].Position;

            Vector3 triMin = Vector3(
                std::min({ v0.x, v1.x, v2.x }),
                std::min({ v0.y, v1.y, v2.y }),
                std::min({ v0.z, v1.z, v2.z })
            );
            Vector3 triMax = Vector3(
                std::max({ v0.x, v1.x, v2.x }),
                std::max({ v0.y, v1.y, v2.y }),
                std::max({ v0.z, v1.z, v2.z })
            );

            int minGridX = std::max(0, (int)((triMin.x - m_minBounds.x) / m_cellSize));
            int maxGridX = std::min(m_gridWidth - 1, (int)((triMax.x - m_minBounds.x) / m_cellSize));
            int minGridZ = std::max(0, (int)((triMin.z - m_minBounds.z) / m_cellSize));
            int maxGridZ = std::min(m_gridHeight - 1, (int)((triMax.z - m_minBounds.z) / m_cellSize));

            for (int gz = minGridZ; gz <= maxGridZ; gz++) {
                for (int gx = minGridX; gx <= maxGridX; gx++) {
                    m_spatialGrid[gz][gx].triangleIndices.push_back(i);
                    m_spatialGrid[gz][gx].minHeight = std::min(m_spatialGrid[gz][gx].minHeight, triMin.y);
                    m_spatialGrid[gz][gx].maxHeight = std::max(m_spatialGrid[gz][gx].maxHeight, triMax.y);
                }
            }
        }

        // デバッグ出力
        char debugStr[256];
        sprintf_s(debugStr, "Spatial grid initialized: %dx%d, cell size: %.2f\n", m_gridWidth, m_gridHeight, m_cellSize);
        OutputDebugStringA(debugStr);
    }

    // レイと三角形の交差判定（既存版）
    bool RayIntersectTriangle(const Vector3& rayOrigin, const Vector3& rayDir,
        const Vector3& v0, const Vector3& v1, const Vector3& v2,
        float& distance, Vector3& hitPoint) {

        const float EPSILON = 0.0000001f;  // 浮動小数点誤差対策

        // ========================================
        // Möller-Trumbore アルゴリズム
        // ========================================

        // 三角形の2辺を計算
        Vector3 edge1 = v1 - v0;
        Vector3 edge2 = v2 - v0;

        // レイ方向と辺2の外積
        Vector3 h = rayDir.Cross(edge2);

        // 行列式を計算（三角形がレイと平行かチェック）
        float a = edge1.Dot(h);

        // レイが三角形と平行な場合（行列式が0に近い）
        if (a > -EPSILON && a < EPSILON)
            return false;  // 交差しない

        float f = 1.0f / a;  // 逆数を事前計算

        // レイ原点から三角形頂点v0へのベクトル
        Vector3 s = rayOrigin - v0;

        // 重心座標uを計算
        float u = f * s.Dot(h);

        // 三角形の境界外チェック（u座標）
        if (u < 0.0f || u > 1.0f)
            return false;

        // sと辺1の外積
        Vector3 q = s.Cross(edge1);

        // 重心座標vを計算
        float v = f * rayDir.Dot(q);

        // 三角形の境界外チェック（v座標とu+v）
        if (v < 0.0f || u + v > 1.0f)
            return false;

        // レイのパラメータtを計算（交差点までの距離）
        float t = f * edge2.Dot(q);

        // 正の方向で交差している場合のみ有効
        if (t > EPSILON) {
            distance = t;
            hitPoint = rayOrigin + rayDir * t;  // 交差点座標を計算
            return true;
        }

        return false;  // 交差点なし
    }

    // レイと三角形の交差判定（新規追加版・垂直レイ用）
    bool RayTriangleIntersection(const Vector3& rayOrigin,
        const Vector3& v0, const Vector3& v1, const Vector3& v2,
        float& hitHeight, Vector3& hitNormal) const
    {
        // 垂直下向きのレイを定義
        Vector3 rayDirection = Vector3(0.0f, -1.0f, 0.0f);

        // 三角形の法線を計算
        Vector3 edge1 = v1 - v0;
        Vector3 edge2 = v2 - v0;
        hitNormal = edge1.Cross(edge2);

        // 法線の長さが0に近い場合（退化した三角形）は無視
        float normalLength = hitNormal.Length();
        if (normalLength < 0.0001f) {
            return false;
        }

        hitNormal = hitNormal * (1.0f / normalLength); // 正規化

        // レイが三角形の平面と平行かチェック
        float ndotdir = hitNormal.Dot(rayDirection);
        if (abs(ndotdir) < 0.0001f) {
            return false; // 平行
        }

        // レイと平面の交点を計算
        float d = hitNormal.Dot(v0);
        float t = (d - hitNormal.Dot(rayOrigin)) / ndotdir;

        // 交点がレイの正の方向にない場合
        if (t < 0) {
            return false;
        }

        // 交点の座標を計算
        Vector3 hitPoint = rayOrigin + rayDirection * t;

        // 点が三角形内にあるかチェック（重心座標系を使用）
        if (IsPointInTriangle(hitPoint, v0, v1, v2)) {
            hitHeight = hitPoint.y;
            return true;
        }

        return false;
    }

    // 簡略化版のレイと三角形の交差判定
    bool RayTriangleIntersectionSimple(const Vector3& rayOrigin,
        const Vector3& v0, const Vector3& v1, const Vector3& v2,
        float& hitHeight, Vector3& hitNormal) const
    {
        // 三角形の法線を計算
        Vector3 edge1 = v1 - v0;
        Vector3 edge2 = v2 - v0;
        hitNormal = edge1.Cross(edge2);

        float normalLength = hitNormal.Length();
        if (normalLength < 0.0001f) {
            return false;
        }

        hitNormal = hitNormal * (1.0f / normalLength);

        // 三角形の平面方程式: ax + by + cz + d = 0
        float d = -hitNormal.Dot(v0);

        // Y座標を平面方程式から計算
        // hitNormal.x * rayOrigin.x + hitNormal.y * y + hitNormal.z * rayOrigin.z + d = 0
        // y = -(hitNormal.x * rayOrigin.x + hitNormal.z * rayOrigin.z + d) / hitNormal.y

        if (abs(hitNormal.y) < 0.0001f) {
            return false; // 垂直な面は無視
        }

        float y = -(hitNormal.x * rayOrigin.x + hitNormal.z * rayOrigin.z + d) / hitNormal.y;
        Vector3 hitPoint = Vector3(rayOrigin.x, y, rayOrigin.z);

        // 点が三角形内にあるかチェック
        if (IsPointInTriangle(hitPoint, v0, v1, v2)) {
            hitHeight = y;
            return true;
        }

        return false;
    }

    // 点が三角形内にあるかチェック（重心座標系）
    bool IsPointInTriangle(const Vector3& point, const Vector3& v0, const Vector3& v1, const Vector3& v2) const
    {
        // 重心座標系での判定
        Vector3 v0v1 = v1 - v0;
        Vector3 v0v2 = v2 - v0;
        Vector3 v0p = point - v0;

        float dot00 = v0v2.Dot(v0v2);
        float dot01 = v0v2.Dot(v0v1);
        float dot02 = v0v2.Dot(v0p);
        float dot11 = v0v1.Dot(v0v1);
        float dot12 = v0v1.Dot(v0p);

        float invDenom = 1.0f / (dot00 * dot11 - dot01 * dot01);
        float u = (dot11 * dot02 - dot01 * dot12) * invDenom;
        float v = (dot00 * dot12 - dot01 * dot02) * invDenom;

        return (u >= 0) && (v >= 0) && (u + v <= 1);
    }

    // 座標変換を考慮した地形の高さ取得（既存版）
    bool GetTerrainHeightOptimized(const Vector3& worldPosition, float& height, Vector3& normal)
    {
        Vector3 localPosition = MatrixHelper::TransformCoord(worldPosition, m_invWorldMatrix);
        int gridX = (int)((localPosition.x - m_minBounds.x) / m_cellSize);
        int gridZ = (int)((localPosition.z - m_minBounds.z) / m_cellSize);
        std::vector<int> candidateTriangles;

        // 3x3範囲で候補収集
        for (int dz = -1; dz <= 1; dz++) {
            for (int dx = -1; dx <= 1; dx++) {
                int checkX = gridX + dx;
                int checkZ = gridZ + dz;

                if (checkX >= 0 && checkX < m_gridWidth && checkZ >= 0 && checkZ < m_gridHeight) {
                    const TerrainCell& cell = m_spatialGrid[checkZ][checkX];
                    candidateTriangles.insert(candidateTriangles.end(),
                        cell.triangleIndices.begin(), cell.triangleIndices.end());
                }
            }
        }

        if (candidateTriangles.empty()) {
            return false;  // 候補なし
        }

        // レイキャスト
        Vector3 rayOrigin = Vector3(localPosition.x, localPosition.y + 2000.0f, localPosition.z);
        Vector3 rayDir = Vector3(0.0f, -1.0f, 0.0f);

        float closestDistance = FLT_MAX;
        Vector3 closestHitPoint;
        Vector3 closestNormal;
        bool hit = false;

        for (int triangleIdx : candidateTriangles) {
            if (triangleIdx + 2 >= (int)m_indices.size()) continue;

            Vector3 v0 = m_vertices[m_indices[triangleIdx]].Position;
            Vector3 v1 = m_vertices[m_indices[triangleIdx + 1]].Position;
            Vector3 v2 = m_vertices[m_indices[triangleIdx + 2]].Position;

            float distance;
            Vector3 hitPoint;

            if (RayIntersectTriangle(rayOrigin, rayDir, v0, v1, v2, distance, hitPoint)) {
                if (distance < closestDistance) {
                    closestDistance = distance;
                    closestHitPoint = hitPoint;

                    Vector3 edge1 = v1 - v0;
                    Vector3 edge2 = v2 - v0;
                    closestNormal = edge2.Cross(edge1);
                    closestNormal.Normalize();

                    if (closestNormal.y < 0) {
                        closestNormal = -closestNormal;
                    }
                    hit = true;
                }
            }
        }

        if (hit) {
            Vector3 worldHitPoint = MatrixHelper::TransformCoord(closestHitPoint, m_worldMatrix);
            Vector3 worldNormal = MatrixHelper::TransformNormal(closestNormal, m_worldMatrix);
            worldNormal.Normalize();

            height = worldHitPoint.y;
            normal = worldNormal;
            return true;
        }

        return false;
    }

    // 改良版地形高度取得（周辺セルも考慮）
    bool GetTerrainHeightImproved(const Vector3& position, float& height, Vector3& normal) const
    {
        int gridX, gridZ;
        if (!GetGridCoordinates(position, gridX, gridZ)) {
            return false;
        }

        // 周辺セルも検索（境界付近での精度向上）
        std::vector<TriangleCandidate> candidates;

        for (int dz = -1; dz <= 1; dz++) {
            for (int dx = -1; dx <= 1; dx++) {
                int gx = gridX + dx;
                int gz = gridZ + dz;

                if (gx < 0 || gx >= m_gridWidth || gz < 0 || gz >= m_gridHeight) {
                    continue;
                }

                const auto& cell = m_spatialGrid[gz][gx];

                for (size_t triIndex : cell.triangleIndices) {
                    Vector3 v0 = m_vertices[m_indices[triIndex]].Position;
                    Vector3 v1 = m_vertices[m_indices[triIndex + 1]].Position;
                    Vector3 v2 = m_vertices[m_indices[triIndex + 2]].Position;

                    float hitHeight;
                    Vector3 hitNormal;
                    if (RayTriangleIntersectionSimple(position, v0, v1, v2, hitHeight, hitNormal)) {
                        // 距離を計算（XZ平面での距離）
                        Vector3 triangleCenter = (v0 + v1 + v2) * (1.0f / 3.0f);
                        float distance = sqrt((position.x - triangleCenter.x) * (position.x - triangleCenter.x) +
                            (position.z - triangleCenter.z) * (position.z - triangleCenter.z));

                        candidates.push_back({ hitHeight, hitNormal, distance });
                    }
                }
            }
        }

        if (candidates.empty()) {
            return false;
        }

        // 最も近い候補を選択
        auto bestCandidate = std::min_element(candidates.begin(), candidates.end(),
            [](const TriangleCandidate& a, const TriangleCandidate& b) {
                return a.distance < b.distance;
            });

        height = bestCandidate->height;
        normal = bestCandidate->normal;
        normal.Normalize();

        return true;
    }

    // プレイヤーの当たり判定更新
    void UpdatePlayerCollision(BoundingSphere& player, Vector3& velocity) {
        float terrainHeight;
        Vector3 terrainNormal;

        if (GetTerrainHeightOptimized(player.center, terrainHeight, terrainNormal)) {
            float playerBottom = player.center.y - player.radius;

            if (playerBottom <= terrainHeight) {
                player.center.y = terrainHeight + player.radius;

                if (terrainNormal.y < 0.99f) {
                    Vector3 gravityDir = Vector3(0, -1, 0);
                    Vector3 slopeDir = gravityDir - terrainNormal * gravityDir.Dot(terrainNormal);

                    if (velocity.y < 0) {
                        velocity.y = 0;
                    }
                }

                Vector3 horizontalVel = Vector3(velocity.x, 0, velocity.z);
                if (horizontalVel.Length() > 0.01f) {
                    Vector3 right = Vector3(1, 0, 0);
                    Vector3 forward = Vector3(0, 0, 1);
                    Vector3 slopeRight = (right - terrainNormal * right.Dot(terrainNormal));
                    slopeRight.Normalize();
                    Vector3 slopeForward = (forward - terrainNormal * forward.Dot(terrainNormal));
                    slopeForward.Normalize();

                    // 水平速度を傾斜に合わせて調整
                    velocity.x = horizontalVel.Dot(slopeRight) * slopeRight.x + horizontalVel.Dot(slopeForward) * slopeForward.x;
                    velocity.z = horizontalVel.Dot(slopeRight) * slopeRight.z + horizontalVel.Dot(slopeForward) * slopeForward.z;
                }
            }
        }
    }

    // デバッグ用ゲッター
    int GetGridWidth() const { return m_gridWidth; }
    int GetGridHeight() const { return m_gridHeight; }
    float GetCellSize() const { return m_cellSize; }
    Vector3 GetMinBounds() const { return m_minBounds; }
    Vector3 GetMaxBounds() const { return m_maxBounds; }

    // グリッド座標を取得
    bool GetGridCoordinates(const Vector3& worldPos, int& gridX, int& gridZ) const {
        gridX = (int)((worldPos.x - m_minBounds.x) / m_cellSize);
        gridZ = (int)((worldPos.z - m_minBounds.z) / m_cellSize);

        return (gridX >= 0 && gridX < m_gridWidth && gridZ >= 0 && gridZ < m_gridHeight);
    }

    // セル内の三角形数を取得
    int GetTriangleCountInCell(int gridX, int gridZ) const {
        if (gridX < 0 || gridX >= m_gridWidth || gridZ < 0 || gridZ >= m_gridHeight) {
            return 0;
        }
        return (int)m_spatialGrid[gridZ][gridX].triangleIndices.size();
    }

    // セルの高さ範囲を取得
    bool GetCellHeightRange(int gridX, int gridZ, float& minHeight, float& maxHeight) const {
        if (gridX < 0 || gridX >= m_gridWidth || gridZ < 0 || gridZ >= m_gridHeight) {
            return false;
        }

        const auto& cell = m_spatialGrid[gridZ][gridX];
        minHeight = cell.minHeight;
        maxHeight = cell.maxHeight;
        return (minHeight != FLT_MAX && maxHeight != -FLT_MAX);
    }

    // GetTerrainHeight のエイリアス（互換性のため）
    bool GetTerrainHeight(const Vector3& position, float& height, Vector3& normal) const {
        return GetTerrainHeightImproved(position, height, normal);
    }
};