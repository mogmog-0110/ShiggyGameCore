#pragma once

/// @file GameEffects.hpp
/// @brief ゲームエフェクト（パーティクル・スコアフラッシュ）

#include <numbers>

#include "sgc/animation/Tween.hpp"
#include "sgc/effects/ParticleSystem.hpp"
#include "sgc/graphics/IRenderer.hpp"
#include "sgc/math/Easing.hpp"

/// @brief 敵撃破時のパーティクルを放出する
/// @param particles パーティクルシステム
/// @param pos 放出位置
inline void emitDeathParticles(sgc::ParticleSystem& particles, const sgc::Vec2f& pos)
{
	sgc::EmitterConfig cfg;
	cfg.positionX = pos.x;
	cfg.positionY = pos.y;
	cfg.rate = 0.0f;  // バースト専用
	cfg.lifetime = 0.5f;
	cfg.speed = 150.0f;
	cfg.spread = static_cast<float>(std::numbers::pi * 2.0);
	cfg.startSize = 3.0f;
	cfg.endSize = 0.5f;
	cfg.startR = 1.0f;
	cfg.startG = 0.5f;
	cfg.startB = 0.1f;
	cfg.startA = 1.0f;
	cfg.endA = 0.0f;
	particles.setConfig(cfg);
	particles.emit(20);
}

/// @brief パーティクルを描画する
/// @param renderer レンダラー
/// @param particles パーティクルシステム
inline void drawParticles(const sgc::IRenderer* renderer, const sgc::ParticleSystem& particles)
{
	for (const auto& p : particles.activeParticles())
	{
		renderer->drawCircle(
			sgc::Vec2f{p.x, p.y}, p.size,
			sgc::Colorf{p.r, p.g, p.b, p.a});
	}
}

/// @brief スコアフラッシュTweenを開始する
/// @param flash 対象のTween
/// @return フラッシュ初期値（1.0f）
inline float startScoreFlash(sgc::Tweenf& flash)
{
	flash = sgc::Tweenf{};
	flash.from(1.0f).to(0.0f).during(0.3f)
		.withEasing(sgc::easing::outQuad<float>);
	return 1.0f;
}
