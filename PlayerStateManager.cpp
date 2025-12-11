#include "PlayerStateManager.h"

// ★.cppファイルで定義（1回だけ）
const std::unordered_map<PlayerStateManager::State, std::set<PlayerStateManager::State>>
PlayerStateManager::s_conflictingStates = {
    {PlayerStateManager::State::SpiralDescending, {
        PlayerStateManager::State::Drifting,
        PlayerStateManager::State::Running,
        PlayerStateManager::State::Countdown,
        PlayerStateManager::State::RaceReady
    }},

    {PlayerStateManager::State::Countdown, {
        PlayerStateManager::State::SpiralDescending,
        PlayerStateManager::State::Drifting,
        PlayerStateManager::State::Running
    }},

    {PlayerStateManager::State::RaceReady, {
        PlayerStateManager::State::Countdown,
        PlayerStateManager::State::SpiralDescending
    }},
};
