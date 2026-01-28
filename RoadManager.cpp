#include "RoadManager.h"
using namespace DirectX::SimpleMath;
std::unique_ptr<BaseRoad> RoadManager::CreateRoad(RoadType type, Direction direction) {
    switch (type) {
    case RoadType::STRAIGHT:
        return std::make_unique<StraightRoad>(direction);
    case RoadType::TURN_LEFT:
        return std::make_unique<LeftTurnRoad>(direction);
    case RoadType::TURN_RIGHT:
        return std::make_unique<RightTurnRoad>(direction);
	case RoadType::START_LINE:
		return std::make_unique<Start>(direction);
    case RoadType::GOAL_LINE:
        return std::make_unique<Goal>(direction);
    case RoadType::SLOPE_UP:
        return std::make_unique<StraightRoad>(direction);
    case RoadType::SLOPE_DOWN:
        return std::make_unique<StraightRoad>(direction);
    case RoadType::DIRT:
        return std::make_unique<Dirt>(direction);
    case RoadType::TURNING:
        return std::make_unique<TurningRoad>(direction);
    case RoadType::NONE:
        return nullptr;  // 道路なしの場合はnullptrを返す
    default:
        return nullptr;  // 未知のタイプもnullptrを返す
    }
}

Vector3 RoadManager::CalculateRotation(Direction direction, RoadType roadType) {
    float baseRotation = static_cast<float>(direction);
	float slopeRotation = 0.0f;

    // 道路タイプに応じて追加の回転を適用
    switch (roadType) {
    case RoadType::TURN_LEFT:
        // 左カーブは基本方向から90度左に曲がる
        break;
    case RoadType::TURN_RIGHT:
        // 右カーブは基本方向から90度右に曲がる
        break;
	case RoadType::SLOPE_UP:
		slopeRotation = -m_slopeangle; // 上り坂はX軸に-15度の傾斜  坂の角度によって道の位置調整
		break;
	case RoadType::SLOPE_DOWN:
		slopeRotation = m_slopeangle; // 下り坂はX軸に15度の傾斜
		break;
    default:
        break;
    }

    return Vector3(MathUtils::DegreesToRadians(slopeRotation), MathUtils::DegreesToRadians(baseRotation), 0.0f);
}

void  RoadManager::ResizeGrid(int width, int height)
{
    if (width <= 0 || height <= 0) return;

    m_gridWidth = width;
    m_gridHeight = height;

    DisposeAll();

    // m_roadGridの初期化
    m_roadGrid.clear();
    m_roadGrid.resize(m_gridHeight);
    for (auto& row : m_roadGrid) {
        row.resize(m_gridWidth);
    }

    // m_roadLayoutの初期化（重要！）
    m_roadLayout.clear();
    m_roadLayout.resize(m_gridHeight, std::vector<RoadSegment>(m_gridWidth));

    printf("Grid resized: %dx%d, Layout size: %zux%zu\n",
        width, height, m_roadLayout.size(),
        m_roadLayout.empty() ? 0 : m_roadLayout[0].size());

}

void RoadManager::InitializeCircuit(const std::vector<std::vector<RoadSegment>>& layout) {
    if (layout.empty() || layout[0].empty()) return;

    m_gridHeight = layout.size();
    m_gridWidth = layout[0].size();

    // グリッドをクリア
    m_roadGrid.clear();
    m_roadGrid.resize(m_gridHeight);
    for (auto& row : m_roadGrid) {
        row.resize(m_gridWidth);
    }

    m_roadLayout = layout;

    // レイアウトに基づいて道路を生成
    for (int y = 0; y < m_gridHeight; ++y) {
        for (int x = 0; x < m_gridWidth; ++x) {
            const auto& segment = layout[y][x];

            // 道路が設定されている場合のみ生成
            if (segment.HasRoad()) {  // 重要な変更点
                auto road = CreateRoad(segment.type, segment.direction);
                if (road) {
                    // 位置を計算 (グリッド座標 * 道路サイズ)
                    Vector3 position = Vector3(
                        x * m_roadSize - (m_gridWidth - 1) * m_roadSize * 0.5f,
                        0.0f,
                        y * m_roadSize - (m_gridHeight - 1) * m_roadSize * 0.5f
                    );

                    road->SetPosition(position);
                    road->SetRotation(CalculateRotation(segment.direction, segment.type));
                    road->SetScale (Vector3(m_roadSize, 1.0f, m_roadSize)) ;

                    road->Init();
                    m_roadGrid[y][x] = std::move(road);
                }
            }
            // segment.HasRoad() == false の場合は何もしない（道路を生成しない）
        }
    }
}

void RoadManager::SetRoad(int x, int y, RoadType type, Direction direction) {
    if (x < 0 || x >= m_gridWidth || y < 0 || y >= m_gridHeight) return;

    if (m_roadGrid[y][x]) {
        m_roadGrid[y][x].reset();
    }

    auto road = CreateRoad(type, direction);
    if (road) {
        if (type == RoadType::DIRT)
        {
            float dirtsize = m_roadSize * 1.75f;//諸事情でダートだけ大きめに
            road->SetScale(Vector3(m_roadSize, 1.0f, dirtsize));
        }
        else
        {
            road->SetScale(Vector3(m_roadSize, 1.0f, m_roadSize));
        }
        road->Init();

        Vector3 actualSize = road->GetActualModelSize();
        float actualWidth = actualSize.x * m_roadSize;
        float actualDepth = actualSize.z * m_roadSize;

        float baseSpacingX = std::max(actualWidth, actualDepth);
        float baseSpacingZ = std::max(actualWidth, actualDepth);

        // この道路の実際の占有距離
        float thisRoadSpacingZ = baseSpacingZ;

        // 坂道の場合、水平投影距離を計算
        if (type == RoadType::SLOPE_UP || type == RoadType::SLOPE_DOWN) {
            float slopeAngle = abs((type == RoadType::SLOPE_UP) ? -m_slopeangle : m_slopeangle);
            float slopeAngleRad = MathUtils::DegreesToRadians(slopeAngle);
            float horizontalProjection = cos(slopeAngleRad);

            if (direction == Direction::NORTH || direction == Direction::SOUTH) {
                thisRoadSpacingZ = baseSpacingZ * horizontalProjection;
            }
        }

        // 累積位置を計算
        float accumulatedZ = CalculateAccumulatedPosition(x, y, direction, thisRoadSpacingZ, actualWidth, actualDepth);

        float roadLength = (direction == Direction::NORTH || direction == Direction::SOUTH)
            ? actualDepth : actualWidth;
        if (type == RoadType::SLOPE_UP || type == RoadType::SLOPE_DOWN)
        {
            startHeight += GetPreviousRoadEndHeight(x, y, direction);//ここ　メンバ変数でやるか？
        }
        else
        {
            startHeight = GetPreviousRoadEndHeight(x, y, direction);//ここ　メンバ変数でやるか？
        }

        float heightChange = 0.0f;
        if (type == RoadType::SLOPE_UP) {
            heightChange = abs(sin(MathUtils::DegreesToRadians(-m_slopeangle)) * roadLength);
        }
        else if (type == RoadType::SLOPE_DOWN) {
            heightChange = -abs(sin(MathUtils::DegreesToRadians(m_slopeangle)) * roadLength);
        }

        float pivotFactor = (direction == Direction::EAST || direction == Direction::WEST)
            ? 1.0f : 0.5f;
        float centerHeight = startHeight + (heightChange * pivotFactor);

        Vector3 position = Vector3(
            (x - (m_gridWidth - 1) * 0.5f) * baseSpacingX,
            centerHeight,
            accumulatedZ  // 累積位置を使用
        );

        road->SetPosition(position);
        road->SetRotation(CalculateRotation(direction, type));
        m_roadGrid[y][x] = std::move(road);

        if (y < static_cast<int>(m_roadLayout.size()) &&
            x < static_cast<int>(m_roadLayout[y].size())) {
            m_roadLayout[y][x] = RoadSegment(type, direction);
            m_roadLayout[y][x].endHeight = startHeight + heightChange;
            m_roadLayout[y][x].spacingZ = thisRoadSpacingZ;  // 占有距離を保存

            startHeight +=  heightChange; // 次の道路の始点高さを更新

        }
    }
}

float RoadManager::CalculateAccumulatedPosition(int x, int y, Direction direction, float thisSpacing,float actualWidth,float actualDepth) {
    float accumulated = 0.0f;

    // グリッドの中心からの相対位置を計算
    float centerOffset = (m_gridHeight - 1) * 0.5f;

    // y=0からy-1までの道路の実際の占有距離を累積
    for (int i = 0; i < y; i++) {
        if (i < static_cast<int>(m_roadLayout.size()) &&
            x < static_cast<int>(m_roadLayout[i].size()) &&
            m_roadLayout[i][x].HasRoad()) {
            accumulated += m_roadLayout[i][x].spacingZ;
        }
        else {
            // 道路がない場合はデフォルト間隔
            accumulated += std::max(actualWidth, actualDepth);
        }
    }

    // 現在の道路の中心位置
    accumulated += thisSpacing * 0.5f;

    // 中心からのオフセット
    return accumulated - (centerOffset * std::max(actualWidth, actualDepth));
}



// 前の道路の終端高さを取得
float RoadManager::GetPreviousRoadEndHeight(int x, int y, Direction direction) {
    int prevX = x, prevY = y;

    // 方向によって前の道路の位置を決定
    switch (direction) {
    case Direction::NORTH: prevY = y + 1; break;
    case Direction::SOUTH: prevY = y - 1; break;
    case Direction::EAST:  prevX = x - 1; break;
    case Direction::WEST:  prevX = x + 1; break;
    }

    if (prevX < 0 || prevX >= m_gridWidth || prevY < 0 || prevY >= m_gridHeight) {
        return 0.0f;
    }

    if (prevY < static_cast<int>(m_roadLayout.size()) &&
        prevX < static_cast<int>(m_roadLayout[prevX].size()) &&
        m_roadLayout[prevY][prevX].HasRoad()) {

        float prevEndHeight = m_roadLayout[prevY][prevX].endHeight;
        printf("Previous road at (%d,%d) endHeight: %.2f\n", prevX, prevY, prevEndHeight);
        return prevEndHeight;
    }

    return 0.0f;
}
// 道路の終端高さを計算
float RoadManager::GetRoadEndHeight(RoadType type, Direction direction, float roadLength, float currentHeight) {
    if (type == RoadType::SLOPE_UP) {
        float slopeAngleRad = MathUtils::DegreesToRadians(8.0f);
        float heightChange = sin(slopeAngleRad) * roadLength;
        return currentHeight + heightChange;
    }
    else if (type == RoadType::SLOPE_DOWN) {
        float slopeAngleRad = MathUtils::DegreesToRadians(-8.0f);
        float heightChange = sin(slopeAngleRad) * roadLength;
        return currentHeight + heightChange;
    }

    return currentHeight;  // 平坦な道路
}

// 既存の道路の回転を変更するメソッド
void RoadManager::SetRoadRotation(int x, int y, const Vector3& rotation) {
    if (x < 0 || x >= m_gridWidth || y < 0 || y >= m_gridHeight) return;

    if (m_roadGrid[y][x]) {  // nullptrチェック
        m_roadGrid[y][x]->SetRotation(rotation); // 正しい構文
    }
}

void RoadManager::UpdateAll(float deltatime) {
    for (auto& row : m_roadGrid) {
        for (auto& road : row) {
            if (road) {
                road->Update(deltatime);
            }
        }
    }
}

void RoadManager::DrawAll() {
    for (auto& row : m_roadGrid) {
        for (auto& road : row) {
            if (road) {
                road->Draw();
            }
        }
    }
}

void RoadManager::DisposeAll() {
    // 各道路のDisposeメソッドを呼び出し
    for (auto& row : m_roadGrid) {
        for (auto& road : row) {
            if (road) {
                road->Dispose();  // 各道路のリソースを解放
                road.reset();     // unique_ptrをnullptrにリセット
            }
        }
    }
}


bool RoadManager::GetTerrainHeight(const Vector3& position, float& height, Vector3& normal) {

    const float LANDING_THRESHOLD = 1.0f;  // 着地可能な高さ差（調整可能）

    bool foundAny = false;
    float closestHeight = 0.0f;
    Vector3 closestNormal;
    float minDistance = FLT_MAX;

    for (auto& row : m_roadGrid) {
        for (auto& road : row) {
            if (road) {
                float candidateHeight;
                Vector3 candidateNormal;

                if (road->GetTerrainHeight(position, candidateHeight, candidateNormal)) {
                    // ★重要な変更：プレイヤーより上の道路は無視★
                    if (candidateHeight >= position.y+ LANDING_THRESHOLD) {
                        continue;  // 上の道路はスキップ
                    }

                    // プレイヤーとの高さの差を計算
                    float heightDifference = abs(position.y - candidateHeight);

                    // より近い道路を優先
                    if (heightDifference < minDistance) {
                        minDistance = heightDifference;
                        closestHeight = candidateHeight;
                        closestNormal = candidateNormal;
                        foundAny = true;
                    }
                }
            }
        }
    }

    if (foundAny) {
        height = closestHeight;
        normal = closestNormal;
        return true;
    }

    return false;
}
std::optional<Vector3> RoadManager::GetStartPos()
{
    for (auto& row : m_roadGrid) {
        for (auto& road : row)
        {
            if (road)
            {
                if (road->GetRoadType() == RoadType::START_LINE) {
                    return road->GetPosition();
                }
            }
        }
    }
    return std::nullopt; // 見つからなかった
}

BaseRoad* RoadManager::GetStart()
{
    for (auto& row : m_roadGrid) {
        for (auto& road : row)
        {
            if (road)
            {
                if (road->GetRoadType() == RoadType::START_LINE) {
                    return road.get();
                }
            }
        }
    }
    return nullptr; // 見つからなかった
}

std::optional<Vector3> RoadManager::GetGoalPos()
{
    for (auto& row : m_roadGrid) {
        for (auto& road : row)
        {
            if (road)
            {
                if (road->GetRoadType() == RoadType::GOAL_LINE) {
                    return road->GetPosition();
                }
            }
        }
    }
    return std::nullopt; // 見つからなかった
}

BaseRoad* RoadManager::GetGoalRoad()
{
    for (auto& row : m_roadGrid) {
        for (auto& road : row)
        {
            if (road)
            {
                if (road->GetRoadType() == RoadType::GOAL_LINE) {
                    return road.get();
                }
            }
        }
    }
    return nullptr; // 見つからなかった
}

//指定したグリッド座標の道路位置を取得する
Vector3 RoadManager::GetRoadPosition(int x, int y) const
{
    // 範囲チェック
    if (x < 0 || x >= m_gridWidth || y < 0 || y >= m_gridHeight) {
        return Vector3(0,0,0);
    }

    // nullチェック（これがないとnullエラー）
    if (!m_roadGrid[y][x]) {
        return Vector3(0, 0, 0);
    }

    return m_roadGrid[y][x]->GetPosition();
}

//指定した道路情報を取得する
BaseRoad* RoadManager::GetRoad(int x, int y) const
{
    if (x < 0 || x >= m_gridWidth || y < 0 || y >= m_gridHeight) {
        return nullptr;
    }
    return m_roadGrid[y][x].get();
}
//特定の状態をすべて取得
std::vector<Vector3> RoadManager::GetRoadPositionsByType(RoadType type) const
{
    std::vector<Vector3> positions;

    for (int y = 0; y < m_gridHeight; y++) {
        for (int x = 0; x < m_gridWidth; x++) {
            if (m_roadGrid[y][x] && m_roadLayout[y][x].type == type) {
                positions.push_back(m_roadGrid[y][x]->GetPosition());
            }
        }
    }

    return positions;
}

std::vector<BaseRoad*> RoadManager::GetRoadByType(RoadType type) const
{
    std::vector<BaseRoad*> positions;

    for (int y = 0; y < m_gridHeight; y++) {
        for (int x = 0; x < m_gridWidth; x++) {
            if (m_roadGrid[y][x] && m_roadLayout[y][x].type == type) {
                positions.push_back(m_roadGrid[y][x].get());
            }
        }
    }

    return positions;
}


//指定した道路の位置をずらした場所を取得する
Vector3 RoadManager::GetRoadEdgePosition(
    int x, int y, float offsetX, float offsetZ) const
{
    auto basePos = GetRoadPosition(x, y);

    Vector3 pos =basePos;
    pos.x += offsetX;
    pos.z += offsetZ;
    return pos;
}

//Plauerがどの路面タイプにいるか取得する際に使用
bool RoadManager::GetRoadSurfaceType(const Vector3& position, RoadType& outSurfaceType) {
    // プレイヤーの位置にある道路を検索
    for (auto& row : m_roadGrid) {
        for (auto& road : row) {
            if (road) {
                // 道路の範囲内にプレイヤーがいるかチェック
                float height;
                Vector3 normal;
                if (road->GetTerrainHeight(position, height, normal))
                {
                    // この道路の上にいる
                    outSurfaceType = road->GetRoadType();
                    return true;
                }
            }
        }
    }
    return false;  // どの道路の上にもいない
}


void RoadManager::UpdatePlayerCollision(BoundingSphere& player, Vector3& velocity)
{
    for (auto& row : m_roadGrid) {
        for (auto& road : row) {
            if (road) {
                road->UpdatePlayerCollision(player, velocity);
            }
        }
    }
}

BaseRoad* RoadManager::GetRoadAtPosition(const Vector3& position)
{
    for (auto& row : m_roadGrid) {
        for (auto& road : row) {
            if (road) {
                float height;
                Vector3 normal;
                // この道路の範囲内にいるかチェック
                if (road->GetTerrainHeight(position, height, normal)) {
                    // 高さの許容範囲内かチェック（プレイヤーが道路の上にいる）
                    float heightDiff = abs(position.y - height);
                    if (heightDiff < 5.0f) { // 許容範囲
                        return road.get();
                    }
                }
            }
        }
    }
    return nullptr;
}

bool RoadManager::GetSafePositionOnRoad(BaseRoad* road,//向きは要検討
    const Vector3& referencePos,
    Vector3& outPosition,
    Vector3& outRotation)
{
    if (!road) return false;

    // 道路の中心位置を取得
    Vector3 roadPos = road->GetPosition();
    Vector3 roadRot = road->GetRotation();

    // 参照位置（プレイヤーが最後にいた位置）に最も近い道路上の点を探す
    // 簡易実装：道路の中心を使用
    float terrainHeight;
    Vector3 terrainNormal;

    // 参照位置の水平座標で道路の高さを取得
    Vector3 checkPos = Vector3(referencePos.x, roadPos.y, referencePos.z);


    if (road->GetTerrainHeight(roadPos, terrainHeight, terrainNormal)) {
        // 道路の中心にリスポーン
        outPosition = Vector3(roadPos.x, terrainHeight + 2.5f, roadPos.z);

        // ★ 道路の向き（Y軸回転）をそのまま使用
       outRotation = roadRot;

        return true;
    }

    return false;
}

bool RoadManager::IsRoadSafe(BaseRoad* road) const
{
    if (!road) return false;

    RoadType type = road->GetRoadType();

    // リスポーン可能な道路タイプ
    return (type != RoadType::NONE);
}

BaseRoad* RoadManager::FindNearestStraightRoad(BaseRoad* fromRoad)
{
    if (!fromRoad) return nullptr;

    Vector3 fromPos = fromRoad->GetPosition();

    float minDistance = FLT_MAX;
    BaseRoad* nearestStraight = nullptr;

    // 全ての道路を検索
    for (int y = 0; y < m_gridHeight; y++) {
        for (int x = 0; x < m_gridWidth; x++) {
            if (m_roadGrid[y][x] && m_roadLayout[y][x].HasRoad()) {
                RoadType type = m_roadLayout[y][x].type;

                // ★ STRAIGHTタイプのみ対象
                if (type == RoadType::STRAIGHT) {
                    Vector3 roadPos = m_roadGrid[y][x]->GetPosition();

                    // 水平距離を計算
                    Vector3 diff = roadPos - fromPos;
                    diff.y = 0; // 高さは無視
                    float distance = diff.Length();

                    if (distance < minDistance) {
                        minDistance = distance;
                        nearestStraight = m_roadGrid[y][x].get();
                    }
                }
            }
        }
    }

    if (nearestStraight) {
        printf("Found nearest STRAIGHT road at distance: %.2f\n", minDistance);
    }
    else {
        printf("No STRAIGHT road found!\n");
    }

    return nearestStraight;
}

BaseRoad* RoadManager::FindNearestStraightRoadFromPosition(const Vector3& position)
{
    float minDistance = FLT_MAX;
    BaseRoad* nearestStraight = nullptr;

    for (int y = 0; y < m_gridHeight; y++) {
        for (int x = 0; x < m_gridWidth; x++) {
            if (m_roadGrid[y][x] && m_roadLayout[y][x].HasRoad()) {
                RoadType type = m_roadLayout[y][x].type;

                if (type == RoadType::STRAIGHT) {
                    Vector3 roadPos = m_roadGrid[y][x]->GetPosition();

                    Vector3 diff = roadPos - position;
                    diff.y = 0;
                    float distance = diff.Length();

                    if (distance < minDistance) {
                        minDistance = distance;
                        nearestStraight = m_roadGrid[y][x].get();
                    }
                }
            }
        }
    }

    return nearestStraight;
}