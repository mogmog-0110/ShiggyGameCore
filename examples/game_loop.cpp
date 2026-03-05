/// @file game_loop.cpp
/// @brief Timer + ECS + EventDispatcher を組み合わせたミニゲームループのサンプル
///
/// デルタタイムで移動を更新し、イベントで衝突を通知する簡易的なゲームループを実演する。

#include <cmath>
#include <iostream>

#include "sgc/core/Timer.hpp"
#include "sgc/ecs/World.hpp"
#include "sgc/patterns/EventDispatcher.hpp"

// ── コンポーネント ──────────────────────────────────────────────

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

/// @brief 体力コンポーネント
struct Health
{
	int hp{100};
};

// ── イベント ────────────────────────────────────────────────────

/// @brief 衝突イベント
struct CollisionEvent
{
	sgc::ecs::Entity a;
	sgc::ecs::Entity b;
	float distance;
};

/// @brief エンティティ死亡イベント
struct DeathEvent
{
	sgc::ecs::Entity entity;
};

int main()
{
	// ワールドとイベントディスパッチャーの初期化
	sgc::ecs::World world;
	sgc::EventDispatcher events;

	// イベントリスナーの登録
	events.on<CollisionEvent>([](const CollisionEvent& e) {
		std::cout << "[Event] Collision detected! distance=" << e.distance << "\n";
	});

	events.on<DeathEvent>([](const DeathEvent&) {
		std::cout << "[Event] Entity died!\n";
	});

	// エンティティの作成
	auto player = world.createEntity();
	world.addComponent(player, Position{0.0f, 0.0f});
	world.addComponent(player, Velocity{2.0f, 1.0f});
	world.addComponent(player, Health{100});

	auto enemy = world.createEntity();
	world.addComponent(enemy, Position{5.0f, 5.0f});
	world.addComponent(enemy, Velocity{-1.0f, -0.5f});
	world.addComponent(enemy, Health{50});

	// Viewの事前作成（ストレージポインタをキャッシュ）
	auto moveView = world.view<Position, Velocity>();
	auto healthView = world.view<Position, Health>();

	// ゲームループ（固定ステップシミュレーション）
	constexpr int MAX_FRAMES = 10;
	constexpr float FIXED_DT = 1.0f / 60.0f;
	constexpr float COLLISION_DISTANCE = 3.0f;

	sgc::DeltaClock clock;

	std::cout << "=== Game Loop Start (" << MAX_FRAMES << " frames) ===\n\n";

	for (int frame = 0; frame < MAX_FRAMES; ++frame)
	{
		const float dt = (frame == 0) ? FIXED_DT : clock.tick();
		(void)dt; // サンプルでは固定ステップを使用

		// 移動システム
		moveView.each([](Position& pos, Velocity& vel) {
			pos.x += vel.dx * FIXED_DT;
			pos.y += vel.dy * FIXED_DT;
		});

		// 衝突検出システム（簡易的: 全ペア比較）
		const auto* playerPos = world.getComponent<Position>(player);
		const auto* enemyPos = world.getComponent<Position>(enemy);
		if (playerPos && enemyPos)
		{
			const float ddx = playerPos->x - enemyPos->x;
			const float ddy = playerPos->y - enemyPos->y;
			const float dist = std::sqrt(ddx * ddx + ddy * ddy);

			if (dist < COLLISION_DISTANCE)
			{
				events.emit(CollisionEvent{player, enemy, dist});

				// 衝突ダメージ
				auto* hp = world.getComponent<Health>(enemy);
				if (hp)
				{
					hp->hp -= 10;
					if (hp->hp <= 0)
					{
						events.emit(DeathEvent{enemy});
					}
				}
			}
		}

		// フレーム情報の表示
		if (frame % 3 == 0 || frame == MAX_FRAMES - 1)
		{
			std::cout << "Frame " << frame << ":\n";
			healthView.each([](Position& pos, Health& hp) {
				std::cout << "  pos=(" << pos.x << ", " << pos.y << ") hp=" << hp.hp << "\n";
			});
		}

		(void)clock.tick();
	}

	std::cout << "\n=== Game Loop End ===\n";
	std::cout << "Entities alive: " << world.entityCount() << "\n";

	return 0;
}
