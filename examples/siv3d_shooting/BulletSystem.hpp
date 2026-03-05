#pragma once

/// @file BulletSystem.hpp
/// @brief 弾管理システム

#include <vector>

#include "sgc/ecs/Entity.hpp"
#include "sgc/ecs/World.hpp"

#include "Components.hpp"

/// @brief 弾管理システム
///
/// 画面外に出た弾を破棄する。
/// Phase::Update, Priority 20 で実行する。
///
/// @note GameSceneの弾エンティティリストへのポインタが必要。
struct BulletSystem
{
	std::vector<sgc::ecs::Entity>* bullets{nullptr};  ///< 弾エンティティリスト

	float screenWidth{800.0f};   ///< 画面幅
	float screenHeight{600.0f};  ///< 画面高さ

	/// @brief 画面外の弾を削除する
	void update(sgc::ecs::World& world, float /*dt*/)
	{
		if (!bullets) return;

		constexpr float MARGIN = 20.0f;

		std::erase_if(*bullets,
			[&](const sgc::ecs::Entity& entity) -> bool
			{
				if (!world.isAlive(entity)) return true;

				const auto* transform = world.getComponent<CTransform>(entity);
				if (!transform) return true;

				const auto& pos = transform->pos;
				const bool outOfBounds =
					pos.x < -MARGIN || pos.x > screenWidth + MARGIN ||
					pos.y < -MARGIN || pos.y > screenHeight + MARGIN;

				if (outOfBounds)
				{
					world.destroyEntity(entity);
					return true;
				}
				return false;
			});
	}
};
