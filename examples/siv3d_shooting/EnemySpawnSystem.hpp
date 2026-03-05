#pragma once

/// @file EnemySpawnSystem.hpp
/// @brief 敵スポーンシステム

#include <random>
#include <vector>

#include "sgc/core/Timer.hpp"
#include "sgc/ecs/Entity.hpp"
#include "sgc/ecs/World.hpp"

#include "Components.hpp"

/// @brief 敵スポーンシステム
///
/// IntervalTimerで一定間隔ごとに敵を生成する。
/// Phase::Update, Priority 5 で実行する。
struct EnemySpawnSystem
{
	std::vector<sgc::ecs::Entity>* enemies{nullptr};  ///< 敵エンティティリスト

	float screenWidth{800.0f};   ///< 画面幅
	float spawnY{-20.0f};        ///< スポーンY座標

	sgc::IntervalTimer spawnTimer{1.5f};  ///< スポーン間隔タイマー
	std::mt19937 rng{std::random_device{}()};  ///< 乱数生成器

	/// @brief 敵のスポーン処理
	void update(sgc::ecs::World& world, float dt)
	{
		if (!enemies) return;

		spawnTimer.update(dt);

		while (spawnTimer.consume())
		{
			spawnEnemy(world);
		}
	}

	/// @brief スポーン間隔を設定する
	/// @param interval 間隔（秒）
	void setSpawnInterval(float interval)
	{
		spawnTimer.setInterval(interval);
	}

private:
	/// @brief 敵を1体生成する
	void spawnEnemy(sgc::ecs::World& world)
	{
		std::uniform_real_distribution<float> xDist(40.0f, screenWidth - 40.0f);
		std::uniform_real_distribution<float> speedDist(80.0f, 200.0f);
		std::uniform_int_distribution<int> hpDist(1, 3);

		const float x = xDist(rng);
		const float speed = speedDist(rng);
		const int hp = hpDist(rng);

		const auto enemy = world.createEntity();
		world.addComponent(enemy, CTransform{.pos = {x, spawnY}});
		world.addComponent(enemy, CVelocity{.vel = {0.0f, speed}});
		world.addComponent(enemy, CSprite{
			.color = sgc::Colorf{1.0f, 0.3f, 0.3f, 1.0f},
			.radius = 12.0f + static_cast<float>(hp) * 4.0f
		});
		world.addComponent(enemy, CEnemy{
			.hp = hp,
			.speed = speed,
			.scoreValue = hp * 100
		});
		enemies->push_back(enemy);
	}
};
