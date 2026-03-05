#pragma once

/// @file Components.hpp
/// @brief ECSコンポーネント定義

#include "sgc/math/Vec2.hpp"
#include "sgc/types/Color.hpp"

/// @brief 位置・回転コンポーネント
struct CTransform
{
	sgc::Vec2f pos{};      ///< 位置
	float rotation{0.0f};  ///< 回転（ラジアン）
};

/// @brief 速度コンポーネント
struct CVelocity
{
	sgc::Vec2f vel{};  ///< 速度ベクトル
};

/// @brief 簡易描画情報コンポーネント
struct CSprite
{
	sgc::Colorf color{sgc::Colorf::white()};  ///< 描画色
	float radius{8.0f};                        ///< 描画半径
};

/// @brief 弾コンポーネント
struct CBullet
{
	bool isPlayerBullet{true};  ///< true=プレイヤー弾、false=敵弾
};

/// @brief 敵コンポーネント
struct CEnemy
{
	int hp{1};            ///< ヒットポイント
	float speed{100.0f};  ///< 移動速度
	int scoreValue{100};  ///< 撃破時のスコア
};

/// @brief プレイヤーコンポーネント
struct CPlayer
{
	int lives{3};              ///< 残機数
	float fireRate{0.15f};     ///< 発射間隔（秒）
	float fireCooldown{0.0f};  ///< 発射クールダウン残り
};

/// @brief 死亡マーカーコンポーネント（削除対象）
struct CDead {};
