#pragma once

/// @file RenderSystem.hpp
/// @brief 描画システム

#include "sgc/ecs/World.hpp"
#include "sgc/siv3d/DrawAdapter.hpp"

#include "Components.hpp"

/// @brief 描画システム
///
/// CTransformとCSpriteを持つ全エンティティを描画する。
/// Phase::Render, Priority 0 で実行する。
struct RenderSystem
{
	/// @brief 全描画対象エンティティを描画する
	void update(sgc::ecs::World& world, float /*dt*/)
	{
		world.forEach<CTransform, CSprite>(
			[](const CTransform& transform, const CSprite& sprite)
			{
				sgc::siv3d::drawCircle(transform.pos, sprite.radius, sprite.color);
			});
	}
};
