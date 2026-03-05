/// @file ecs_basic.cpp
/// @brief ECSの基本的な使い方を示すサンプル
///
/// エンティティの作成、コンポーネントの追加、forEachクエリ、Viewの使用を実演する。

#include <iostream>

#include "sgc/ecs/World.hpp"

/// @brief 位置コンポーネント
struct Position
{
	float x{0.0f};
	float y{0.0f};
};

/// @brief 速度コンポーネント
struct Velocity
{
	float dx{0.0f};
	float dy{0.0f};
};

/// @brief 名前コンポーネント
struct Name
{
	const char* value{"unknown"};
};

int main()
{
	sgc::ecs::World world;

	// エンティティの作成とコンポーネントの追加
	auto player = world.createEntity();
	world.addComponent(player, Name{"Player"});
	world.addComponent(player, Position{0.0f, 0.0f});
	world.addComponent(player, Velocity{1.0f, 0.5f});

	auto enemy = world.createEntity();
	world.addComponent(enemy, Name{"Enemy"});
	world.addComponent(enemy, Position{10.0f, 10.0f});
	world.addComponent(enemy, Velocity{-0.5f, -0.5f});

	auto wall = world.createEntity();
	world.addComponent(wall, Name{"Wall"});
	world.addComponent(wall, Position{5.0f, 5.0f});
	// 壁にはVelocityなし → 移動システムではスキップされる

	std::cout << "=== Entity count: " << world.entityCount() << " ===\n\n";

	// forEachで移動システム
	std::cout << "--- Movement (forEach) ---\n";
	const float dt = 1.0f / 60.0f;
	world.forEach<Position, Velocity>([dt](Position& pos, Velocity& vel) {
		pos.x += vel.dx * dt;
		pos.y += vel.dy * dt;
	});

	// Viewで再利用可能なクエリ
	std::cout << "--- Positions (View) ---\n";
	auto posView = world.view<Position, Name>();
	posView.each([](Position& pos, Name& name) {
		std::cout << name.value << ": (" << pos.x << ", " << pos.y << ")\n";
	});

	// エンティティの破棄
	world.destroyEntity(enemy);
	std::cout << "\n--- After destroying enemy ---\n";
	std::cout << "Entity count: " << world.entityCount() << "\n";

	return 0;
}
