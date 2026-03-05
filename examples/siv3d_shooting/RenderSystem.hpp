#pragma once

/// @file RenderSystem.hpp
/// @brief 描画システム

#include "sgc/ecs/World.hpp"
#include "sgc/graphics/IRenderer.hpp"

#include "Components.hpp"

/// @brief 描画システム
///
/// CTransformとCSpriteを持つ全エンティティを描画する。
/// Phase::Render, Priority 0 で実行する。
/// IRenderer経由で描画し、フレームワーク非依存で動作する。
struct RenderSystem
{
	sgc::IRenderer* renderer = nullptr;  ///< 描画インターフェース

	/// @brief 全描画対象エンティティを描画する
	void update(sgc::ecs::World& world, float /*dt*/)
	{
		if (!renderer) return;

		auto* r = renderer;
		world.forEach<CTransform, CSprite>(
			[r](const CTransform& transform, const CSprite& sprite)
			{
				r->drawCircle(transform.pos, sprite.radius, sprite.color);
			});
	}
};
