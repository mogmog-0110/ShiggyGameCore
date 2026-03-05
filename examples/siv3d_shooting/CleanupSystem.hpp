#pragma once

/// @file CleanupSystem.hpp
/// @brief 画面外敵の削除システム

#include <vector>

#include "sgc/ecs/Entity.hpp"
#include "sgc/ecs/World.hpp"

#include "Components.hpp"

/// @brief 画面外に出た敵を削除するシステム
///
/// 画面下端を超えた敵エンティティを破棄する。
/// Phase::LateUpdate, Priority 10 で実行する。
struct CleanupSystem
{
	std::vector<sgc::ecs::Entity>* enemies{nullptr};  ///< 敵エンティティリスト

	float screenHeight{600.0f};  ///< 画面高さ

	/// @brief 画面下端を超えた敵を削除する
	void update(sgc::ecs::World& world, float /*dt*/)
	{
		if (!enemies) return;

		constexpr float MARGIN = 40.0f;
		const float maxY = screenHeight + MARGIN;

		std::erase_if(*enemies,
			[&](const sgc::ecs::Entity& entity) -> bool
			{
				if (!world.isAlive(entity)) return true;

				const auto* transform = world.getComponent<CTransform>(entity);
				if (!transform) return true;

				if (transform->pos.y > maxY)
				{
					world.destroyEntity(entity);
					return true;
				}
				return false;
			});
	}
};
