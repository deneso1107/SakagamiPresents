#pragma once
#include <memory>
#include <vector>
#include "BaseStageModel.h"
#include "StageModel1.h"
#include "StageModel2.h"
#include "StageModel3.h"
#include "StageSelectCamera.h"
class StageManager {
private:
    std::vector<std::unique_ptr<BaseStageModel>> m_stages;
    int   m_currentIndex = 0;
    int   m_stageCount = 0;

    // カルーセル用パラメータ
    float m_carouselRadius = 40.0f;  // 円の半径
    float m_currentAngle = 0.0f;   // 現在の回転角度（ラジアン）
    float m_targetAngle = 0.0f;   // 目標角度
    float m_rotateSpeed = 5.0f;   // 回転の補間速度

    // 各ステージの円弧上の角度を計算
    float GetStageAngle(int index) const {
        float angleStep = (2.0f * 3.14159265f) / m_stageCount;
        return index * angleStep;
    }

public:
    template<typename T>
    void RegisterStage() {
        auto stage = std::make_unique<T>();
        stage->Init();
        m_stages.push_back(std::move(stage));
        m_stageCount = (int)m_stages.size();
    }

    void Update(float deltaTime) 
    {
        m_currentAngle += (m_targetAngle - m_currentAngle) * (m_rotateSpeed * deltaTime);

        for (int i = 0; i < m_stageCount; i++) {
            if (!m_stages[i]) continue;

            float angle = GetStageAngle(i) + m_currentAngle;
            float x = sinf(angle) * m_carouselRadius;
            float z = cosf(angle) * m_carouselRadius;

            // Z座標が小さい（マイナス）ほど手前
            // -radius〜+radius を 1.0〜0.4 にマッピング
            float t = (z + m_carouselRadius) / (2.0f * m_carouselRadius);
            float carouselScale = 0.4f + t * 0.6f;

			// Y座標は一定にして、X,Zで円を描く配置
            m_stages[i]->SetPosition(Vector3(x, 30.0f, z));
            m_stages[i]->SetBaseY(30.0f);

            // 個別スケールにカルーセルスケールを掛け合わせる
            m_stages[i]->SetCarouselScale(carouselScale);
            m_stages[i]->SetSelected(i == m_currentIndex);
            m_stages[i]->Update(deltaTime);
        }
    }

    void DrawAll() {
        // 奥から手前の順に描画（Zソート）
        // Z座標が小さい（奥）順にソートして描画
        std::vector<int> order(m_stageCount);
        for (int i = 0; i < m_stageCount; i++) order[i] = i;

        std::sort(order.begin(), order.end(), [this](int a, int b) {
            return m_stages[a]->GetPosition().z < m_stages[b]->GetPosition().z;
            });

        for (int i : order) {
            if (m_stages[i]) m_stages[i]->Draw();
        }
    }

    void Next() {
        m_currentIndex = (m_currentIndex + 1) % m_stageCount; // ループ
        // 目標角度を1ステージ分だけ逆回転（手前に次が来る）
        float angleStep = (2.0f * 3.14159265f) / m_stageCount;
        m_targetAngle -= angleStep;
    }

    void Prev() {
        m_currentIndex = (m_currentIndex - 1 + m_stageCount) % m_stageCount; // ループ
        float angleStep = (2.0f * 3.14159265f) / m_stageCount;
        m_targetAngle += angleStep;
    }

    void Dispose() {
        for (auto& s : m_stages) if (s) s->Dispose();
        m_stages.clear();
    }

    int  GetCurrentIndex() const { return m_currentIndex; }
    int  GetStageCount()   const { return m_stageCount; }
    BaseStageModel* GetCurrent() const {
        if (m_stages.empty()) return nullptr;
        return m_stages[m_currentIndex].get();
    }

    // カルーセル調整用
    void SetRadius(float r) { m_carouselRadius = r; }
    void SetRotateSpeed(float s) { m_rotateSpeed = s; }
};