#pragma once

/// @file Events.hpp
/// @brief ゲームイベント型定義

#include "sgc/math/Vec2.hpp"

/// @brief 敵撃破イベント
struct EnemyKilled
{
	sgc::Vec2f pos{};  ///< 撃破位置
	int score{0};      ///< 獲得スコア
};

/// @brief プレイヤー被弾イベント
struct PlayerDamaged
{
	int remainingLives{0};  ///< 残りライフ
};

/// @brief ゲームオーバーイベント
struct GameOver {};
