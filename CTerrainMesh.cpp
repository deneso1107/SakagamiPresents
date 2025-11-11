//#include "CTerrainMesh.h"
//#include "system/CStaticMesh.h"
//#include "system/collision.h"
//void CTerrainMesh::InitializeSpatialGrid(float cellSize)
//{
//    m_cellSize = cellSize;
//
//    // 地形の境界を計算
//    m_minBounds = Vector3(FLT_MAX, FLT_MAX, FLT_MAX);
//    m_maxBounds = Vector3(-FLT_MAX, -FLT_MAX, -FLT_MAX);
//
//    for (const auto& vertex : m_vertices) {
//        if (vertex.Position.x < m_minBounds.x) m_minBounds.x = vertex.Position.x;
//        if (vertex.Position.y < m_minBounds.y) m_minBounds.y = vertex.Position.y;
//        if (vertex.Position.z < m_minBounds.z) m_minBounds.z = vertex.Position.z;
//        if (vertex.Position.x > m_maxBounds.x) m_maxBounds.x = vertex.Position.x;
//        if (vertex.Position.y > m_maxBounds.y) m_maxBounds.y = vertex.Position.y;
//        if (vertex.Position.z > m_maxBounds.z) m_maxBounds.z = vertex.Position.z;
//    }
//
//    // グリッドサイズを計算
//    m_gridWidth = (int)ceil((m_maxBounds.x - m_minBounds.x) / m_cellSize);
//    m_gridHeight = (int)ceil((m_maxBounds.z - m_minBounds.z) / m_cellSize);
//
//    // グリッドを初期化
//    m_spatialGrid.resize(m_gridHeight);
//    for (int i = 0; i < m_gridHeight; i++) {
//        m_spatialGrid[i].resize(m_gridWidth);
//    }
//
//    // 三角形をグリッドに分類
//    for (size_t i = 0; i < m_indices.size(); i += 3) {
//        Vector3 v0 = m_vertices[m_indices[i]].Position;
//        Vector3 v1 = m_vertices[m_indices[i + 1]].Position;
//        Vector3 v2 = m_vertices[m_indices[i + 2]].Position;
//
//        // 三角形の境界を計算
//        Vector3 triMin = Vector3(
//            std::min({ v0.x, v1.x, v2.x }),
//            std::min({ v0.y, v1.y, v2.y }),
//            std::min({ v0.z, v1.z, v2.z })
//        );
//        Vector3 triMax = Vector3(
//            std::max({ v0.x, v1.x, v2.x }),
//            std::max({ v0.y, v1.y, v2.y }),
//            std::max({ v0.z, v1.z, v2.z })
//        );
//
//        // 該当するグリッドセルに三角形を追加
//        int minGridX = (int)((triMin.x - m_minBounds.x) / m_cellSize);
//        int maxGridX = (int)((triMax.x - m_minBounds.x) / m_cellSize);
//        int minGridZ = (int)((triMin.z - m_minBounds.z) / m_cellSize);
//        int maxGridZ = (int)((triMax.z - m_minBounds.z) / m_cellSize);
//
//        for (int gz = minGridZ; gz <= maxGridZ && gz < m_gridHeight; gz++) {
//            for (int gx = minGridX; gx <= maxGridX && gx < m_gridWidth; gx++) {
//                if (gx >= 0 && gz >= 0) {
//                    m_spatialGrid[gz][gx].triangleIndices.push_back(i);
//                    m_spatialGrid[gz][gx].minHeight = std::min(m_spatialGrid[gz][gx].minHeight, triMin.y);
//                    m_spatialGrid[gz][gx].maxHeight = std::max(m_spatialGrid[gz][gx].maxHeight, triMax.y);
//                }
//            }
//        }
//    }
//}
//bool CTerrainMesh::RayIntersectTriangle(const Vector3& rayOrigin, const Vector3& rayDir,
//    const Vector3& v0, const Vector3& v1, const Vector3& v2,
//    float& distance, Vector3& hitPoint)
//{
//    const float EPSILON = 0.0000001f;
//
//    Vector3 edge1 = v1 - v0;
//    Vector3 edge2 = v2 - v0;
//    Vector3 h = rayDir.Cross(edge2);
//    float a = edge1.Dot(h);
//
//    if (a > -EPSILON && a < EPSILON) return false; // レイが三角形と平行
//
//    float f = 1.0f / a;
//    Vector3 s = rayOrigin - v0;
//    float u = f * s.Dot(h);
//
//    if (u < 0.0f || u > 1.0f) return false;
//
//    Vector3 q = s.Cross(edge1);
//    float v = f * rayDir.Dot(q);
//
//    if (v < 0.0f || u + v > 1.0f) return false;
//
//    float t = f * edge2.Dot(q);
//
//    if (t > EPSILON) {
//        distance = t;
//        hitPoint = rayOrigin + rayDir * t;
//        return true;
//    }
//
//    return false;
//}
//bool CTerrainMesh::GetTerrainHeightOptimized(const Vector3& position, float& height, Vector3& normal) {
//    // グリッド座標を計算
//    int gridX = (int)((position.x - m_minBounds.x) / m_cellSize);
//    int gridZ = (int)((position.z - m_minBounds.z) / m_cellSize);
//
//    if (gridX < 0 || gridX >= m_gridWidth || gridZ < 0 || gridZ >= m_gridHeight) {
//        return false;
//    }
//
//    Vector3 rayOrigin = Vector3(position.x, position.y + 1000.0f, position.z);
//    Vector3 rayDir = Vector3(0.0f, -1.0f, 0.0f);
//
//    float closestDistance = FLT_MAX;
//    Vector3 closestHitPoint;
//    Vector3 closestNormal;
//    bool hit = false;
//
//    // 該当するセルの三角形のみをチェック
//    const TerrainCell& cell = m_spatialGrid[gridZ][gridX];
//    for (int triangleIdx : cell.triangleIndices) {
//        Vector3 v0 = m_vertices[m_indices[triangleIdx]].Position;
//        Vector3 v1 = m_vertices[m_indices[triangleIdx + 1]].Position;
//        Vector3 v2 = m_vertices[m_indices[triangleIdx + 2]].Position;
//
//        float distance;
//        Vector3 hitPoint;
//
//        if (RayIntersectTriangle(rayOrigin, rayDir, v0, v1, v2, distance, hitPoint)) {
//            if (distance < closestDistance) {
//                closestDistance = distance;
//                closestHitPoint = hitPoint;
//
//                // 法線計算
//                Vector3 edge1 = v1 - v0;
//                Vector3 edge2 = v2 - v0;
//                closestNormal = edge1.Cross(edge2);
//                closestNormal.Normalize();//明日の俺へ　Claudeと一緒に当たり判定を完成させよう頑張れ
//
//                hit = true;
//            }
//        }
//    }
//
//    if (hit) {
//        height = closestHitPoint.y;
//        normal = closestNormal;
//        return true;
//    }
//
//    return false;
//}
//// 最適化された地形の高さ取得
//bool CTerrainMesh::GetTerrainHeightOptimized(const Vector3& position, float& height, Vector3& normal) {
//    // グリッド座標を計算
//    int gridX = (int)((position.x - m_minBounds.x) / m_cellSize);
//    int gridZ = (int)((position.z - m_minBounds.z) / m_cellSize);
//
//    if (gridX < 0 || gridX >= m_gridWidth || gridZ < 0 || gridZ >= m_gridHeight) {
//        return false;
//    }
//
//    Vector3 rayOrigin = Vector3(position.x, position.y + 1000.0f, position.z);
//    Vector3 rayDir = Vector3(0.0f, -1.0f, 0.0f);
//
//    float closestDistance = FLT_MAX;
//    Vector3 closestHitPoint;
//    Vector3 closestNormal;
//    bool hit = false;
//
//    // 該当するセルの三角形のみをチェック
//    const TerrainCell& cell = m_spatialGrid[gridZ][gridX];
//    for (int triangleIdx : cell.triangleIndices) {
//        Vector3 v0 = m_vertices[m_indices[triangleIdx]].Position;
//        Vector3 v1 = m_vertices[m_indices[triangleIdx + 1]].Position;
//        Vector3 v2 = m_vertices[m_indices[triangleIdx + 2]].Position;
//
//        float distance;
//        Vector3 hitPoint;
//
//        if (RayIntersectTriangle(rayOrigin, rayDir, v0, v1, v2, distance, hitPoint)) {
//            if (distance < closestDistance) {
//                closestDistance = distance;
//                closestHitPoint = hitPoint;
//
//                // 法線計算
//                Vector3 edge1 = v1 - v0;
//                Vector3 edge2 = v2 - v0;
//                closestNormal = edge1.Cross(edge2);
//                closestNormal.Normalize();//明日の俺へ　Claudeと一緒に当たり判定を完成させよう頑張れ
//
//                hit = true;
//            }
//        }
//    }
//
//    if (hit) {
//        height = closestHitPoint.y;
//        normal = closestNormal;
//        return true;
//    }
//
//    return false;
//}
//
//// プレイヤーの当たり判定更新（最適化版）
//void CTerrainMesh::UpdatePlayerCollision(BoundingSphere& player, Vector3& velocity) {
//    float terrainHeight;
//    Vector3 terrainNormal;
//
//    if (GetTerrainHeightOptimized(player.center, terrainHeight, terrainNormal)) {
//        // 球の底部が地形の高さより下にある場合
//        float playerBottom = player.center.y - player.radius;
//
//        if (playerBottom <= terrainHeight) {
//            // プレイヤーを地形の上に配置
//            player.center.y = terrainHeight + player.radius;
//
//            // 坂での移動調整（法線に沿って移動）
//            if (terrainNormal.y < 1.0f) { // 坂の場合
//                // 重力成分を坂に沿って調整
//                Vector3 gravityDir = Vector3(0, -1, 0);
//                Vector3 slopeDir = gravityDir - terrainNormal * gravityDir.Dot(terrainNormal);
//
//                // Y方向の速度を坂に沿った速度に変換
//                if (velocity.y < 0) {
//                    velocity.y = 0;
//                    // 坂を滑り落ちる効果を加える場合
//                    // velocity += slopeDir * 0.1f;
//                }
//            }
//
//            // 坂での横移動補正
//            Vector3 horizontalVel = Vector3(velocity.x, 0, velocity.z);
//            if (horizontalVel.Length() > 0) {
//                // 坂の傾斜に応じて移動方向を調整
//                Vector3 right = Vector3(1, 0, 0);
//                Vector3 forward = Vector3(0, 0, 1);
//                Vector3 slopeRight = (right - terrainNormal * right.Dot(terrainNormal));
//                slopeRight.Normalize();
//                Vector3 slopeForward = (forward - terrainNormal * forward.Dot(terrainNormal));
//                slopeForward.Normalize();
//
//                // 坂に沿った移動に変換
//                velocity.x = horizontalVel.Dot(slopeRight) * slopeRight.x + horizontalVel.Dot(slopeForward) * slopeForward.x;
//                velocity.z = horizontalVel.Dot(slopeRight) * slopeRight.z + horizontalVel.Dot(slopeForward) * slopeForward.z;
//            }
//        }
//    }
//}