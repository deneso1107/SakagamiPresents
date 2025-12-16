#pragma once
#include <set>
#include <unordered_map>
#include <string>

// 状態管理クラス
class PlayerStateManager
{
public:
    enum class State//状態を格納
    {
        OnGround,
        Drifting,
        Running,

        // ★新規追加：スタートシーケンス用の状態
        SpiralDescending,  // 螺旋降下中
        Countdown,         // カウントダウン中
        RaceReady,         // レース準備完了（カウント0の瞬間）

        // 将来的に追加予定の状態をここに追加
        // Jumping,
        // Crouching,
        // Attacking,
    };

private:
    //std::setは重複を無くすコンテナ(照準)
    std::set<State> m_activeStates;

    // 状態間の競合関係を定義（必要に応じて）
    static const std::unordered_map<State, std::set<State>> s_conflictingStates;

public:
    // 状態追加
    void AddState(State state) {
        RemoveConflictingStates(state);
        m_activeStates.insert(state);
    }

    // 状態削除
    void RemoveState(State state) {
        m_activeStates.erase(state);
    }

    // 状態チェック
    bool HasState(State state) const {
        return m_activeStates.find(state) != m_activeStates.end();
    }

    // 複数状態の同時チェック（AND条件）
    bool HasAllStates(const std::initializer_list<State>& states) const {
        for (const auto& state : states) {
            if (!HasState(state)) return false;
        }
        return true;
    }

    // いずれかの状態をチェック（OR条件）
    bool HasAnyState(const std::initializer_list<State>& states) const {
        for (const auto& state : states) {
            if (HasState(state)) return true;
        }
        return false;
    }

    // 状態切り替え
    void ToggleState(State state) {
        if (HasState(state)) {
            RemoveState(state);
        }
        else {
            AddState(state);
        }
    }

    // 全状態クリア
    void ClearAllStates() {
        m_activeStates.clear();
    }

    // 現在の要件に特化した便利メソッド
    bool IsOnGround() const { return HasState(State::OnGround); }
    bool IsDrifting() const { return HasState(State::Drifting); }
    bool IsRunning() const { return HasState(State::Running); }

    // スタートシーケンス用の便利メソッド
    bool IsSpiralDescending() const { return HasState(State::SpiralDescending); }
    bool IsCountdown() const { return HasState(State::Countdown); }
    bool IsRaceReady() const { return HasState(State::RaceReady); }
    bool IsInStartSequence() const 
    {
        return HasAnyState({ State::SpiralDescending, State::Countdown, State::RaceReady });
    }


    // よくある組み合わせチェック
    bool IsGroundDrifting() const {
        return HasAllStates({ State::OnGround, State::Drifting });
    }
    bool IsGroundRunning() const {
        return HasAllStates({ State::OnGround, State::Running });
    }

    // デバッグ用
    std::string GetActiveStatesString() const {
        std::string result;
        for (const auto& state : m_activeStates) {
            if (!result.empty()) result += ", ";
            result += StateToString(state);
        }
        return result.empty() ? "None" : result;
    }

private:
    // 競合する状態を削除
    void RemoveConflictingStates(State newState) {
        auto it = s_conflictingStates.find(newState);
        if (it != s_conflictingStates.end()) {
            for (const auto& conflictingState : it->second) {
                RemoveState(conflictingState);
            }
        }
    }

    // 状態を文字列に変換
    std::string StateToString(State state) const 
    {
        switch (state) {
        case State::OnGround: return "OnGround";
        case State::Drifting: return "Drifting";
        case State::Running: return "Running";
        default: return "Unknown";
        }
    }
};

// 競合状態の定義（現在は特になし、将来的に追加可能）
//const std::unordered_map<PlayerStateManager::State, std::set<PlayerStateManager::State>>
//PlayerStateManager::s_conflictingStates = {
//    // 例：ジャンプ時は地面にいない
//    // {State::Jumping, {State::OnGround}}