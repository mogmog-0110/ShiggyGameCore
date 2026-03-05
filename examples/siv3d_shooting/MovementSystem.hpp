#pragma once

/// @file MovementSystem.hpp
/// @brief 移動処理システム

#include "sgc/ecs/World.hpp"

#include "Components.hpp"

/// @brief 移動処理システム
///
/// CVelocity を CTransform に適用する。
/// Phase::Update, Priority 10 で実行する。
struct MovementSystem
{
	/// @brief 全エンティティの位置を速度で更新する
	void update(sgc::ecs::World& world, float dt)
	{
		world.forEach<CTransform, CVelocity>(
			[dt](CTransform& transform, CVelocity& velocity)
			{
				transform.pos = transform.pos + velocity.vel * dt;
			});
	}
};
