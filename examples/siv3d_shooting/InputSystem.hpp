#pragma once

/// @file InputSystem.hpp
/// @brief プレイヤー入力処理システム

#include <vector>

#include "sgc/core/Hash.hpp"
#include "sgc/ecs/Entity.hpp"
#include "sgc/ecs/World.hpp"
#include "sgc/input/ActionMap.hpp"

#include "Components.hpp"

/// @brief 入力アクションID定数
namespace Actions
{
using namespace sgc::literals;

inline constexpr sgc::ActionId MOVE_LEFT = "moveLeft"_hash;
inline constexpr sgc::ActionId MOVE_RIGHT = "moveRight"_hash;
inline constexpr sgc::ActionId MOVE_UP = "moveUp"_hash;
inline constexpr sgc::ActionId MOVE_DOWN = "moveDown"_hash;
inline constexpr sgc::ActionId FIRE = "fire"_hash;

} // namespace Actions

/// @brief プレイヤー入力処理システム
///
/// ActionMapの状態からプレイヤーの移動と弾発射を処理する。
/// Phase::Update, Priority 0 で実行する。
struct InputSystem
{
	sgc::ActionMap* actionMap{nullptr};                 ///< アクションマップ
	std::vector<sgc::ecs::Entity>* bullets{nullptr};   ///< 弾リスト（生成時に追加）

	static constexpr float PLAYER_SPEED = 400.0f;  ///< プレイヤー移動速度
	static constexpr float BULLET_SPEED = 600.0f;  ///< 弾の速度

	/// @brief 入力に基づくプレイヤー操作の更新
	void update(sgc::ecs::World& world, float dt)
	{
		if (!actionMap || !bullets) return;

		world.forEach<CPlayer, CTransform, CVelocity>(
			[this, dt, &world](CPlayer& player, CTransform& transform, CVelocity& velocity)
			{
				// 移動入力
				sgc::Vec2f moveDir{0.0f, 0.0f};
				if (actionMap->isHeld(Actions::MOVE_LEFT))  moveDir.x -= 1.0f;
				if (actionMap->isHeld(Actions::MOVE_RIGHT)) moveDir.x += 1.0f;
				if (actionMap->isHeld(Actions::MOVE_UP))    moveDir.y -= 1.0f;
				if (actionMap->isHeld(Actions::MOVE_DOWN))  moveDir.y += 1.0f;

				// 正規化して斜め移動の速度を均一にする
				if (moveDir.lengthSquared() > 0.0f)
				{
					moveDir = moveDir.normalized();
				}
				velocity.vel = moveDir * PLAYER_SPEED;

				// 弾発射クールダウン
				player.fireCooldown -= dt;
				if (player.fireCooldown < 0.0f)
				{
					player.fireCooldown = 0.0f;
				}

				// 弾発射
				if (actionMap->isHeld(Actions::FIRE) && player.fireCooldown <= 0.0f)
				{
					spawnBullet(world, transform.pos);
					player.fireCooldown = player.fireRate;
				}
			});
	}

private:
	/// @brief プレイヤー弾を生成する
	void spawnBullet(sgc::ecs::World& world, const sgc::Vec2f& playerPos)
	{
		const auto bullet = world.createEntity();
		world.addComponent(bullet, CTransform{.pos = {playerPos.x, playerPos.y - 10.0f}});
		world.addComponent(bullet, CVelocity{.vel = {0.0f, -BULLET_SPEED}});
		world.addComponent(bullet, CSprite{
			.color = sgc::Colorf{1.0f, 1.0f, 0.2f, 1.0f},
			.radius = 4.0f
		});
		world.addComponent(bullet, CBullet{.isPlayerBullet = true});
		bullets->push_back(bullet);
	}
};
