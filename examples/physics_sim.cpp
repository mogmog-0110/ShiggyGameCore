/// @file physics_sim.cpp
/// @brief RigidBody2D・AABB衝突・固定タイムステップの物理シミュレーション
///
/// sgcの物理ユーティリティを使って以下をデモする:
/// - RigidBody2Dによる重力落下シミュレーション
/// - FixedTimestepによる固定間隔更新
/// - AABB衝突検出と衝突応答（地面での跳ね返り）
/// - 複数オブジェクトの物理演算

#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

#include "sgc/physics/RigidBody2D.hpp"
#include "sgc/physics/FixedTimestep.hpp"

/// @brief 物体の位置をASCIIで可視化する
/// @param name 名前
/// @param y Y座標（下が正）
/// @param groundY 地面のY座標
/// @param width 描画幅
void printHeight(const std::string& name, float y, float groundY, int width = 40)
{
	/// Y座標を上下反転して高さに変換（groundYが底）
	const float height = groundY - y;
	const float maxHeight = groundY;
	const float ratio = (maxHeight > 0.0f) ? (height / maxHeight) : 0.0f;
	const int pos = static_cast<int>(ratio * width);
	const int clampedPos = (pos < 0) ? 0 : (pos > width ? width : pos);

	std::cout << std::setw(10) << name << " |";
	for (int i = 0; i < width; ++i)
	{
		if (i == clampedPos)
		{
			std::cout << 'O';
		}
		else if (i == width - 1)
		{
			std::cout << '|';
		}
		else
		{
			std::cout << ' ';
		}
	}
	std::cout << " y=" << std::fixed << std::setprecision(1) << y << "\n";
}

int main()
{
	std::cout << "=== sgc Physics Simulation Demo ===\n\n";

	using namespace sgc::physics;

	// ────────────────────────────────────────────────────
	// 1. 基本的な自由落下
	// ────────────────────────────────────────────────────
	std::cout << "--- 1. Free Fall with Gravity ---\n";
	std::cout << "  A ball falls from height and bounces on the ground.\n";
	std::cout << "  (Left = high, Right = ground level)\n\n";
	{
		/// 落下する物体
		RigidBody2Df ball;
		ball.position = {100.0f, 0.0f};    // 画面上部（y=0が上）
		ball.mass = 2.0f;
		ball.restitution = 0.6f;            // バウンド（反発）あり
		ball.drag = 0.01f;

		/// 地面（静的オブジェクト）
		RigidBody2Df ground;
		ground.position = {100.0f, 300.0f};
		ground.isStatic = true;

		/// オブジェクトのサイズ（衝突判定用の半分サイズ）
		const sgc::Vec2f ballHalf{10.0f, 10.0f};
		const sgc::Vec2f groundHalf{200.0f, 10.0f};

		const float dt = 1.0f / 60.0f;
		const float gravity = 980.0f;  // ピクセル/秒^2

		for (int frame = 0; frame < 30; ++frame)
		{
			/// 重力を適用
			ball.applyForce({0.0f, gravity * ball.mass});

			/// 物理ステップ
			ball.integrate(dt);

			/// AABB衝突検出
			const auto ballAABB = ball.bounds(ballHalf);
			const auto groundAABB = ground.bounds(groundHalf);
			const auto collision = detectAABBCollision(ballAABB, groundAABB);

			if (collision.colliding)
			{
				resolveRigidBodyCollision(ball, ground, collision);
			}

			if (frame % 3 == 0)
			{
				std::cout << "  f" << std::setw(2) << frame << ": ";
				printHeight("Ball", ball.position.y, 300.0f);
			}
		}
		std::cout << "  Final position: (" << ball.position.x << ", "
				  << ball.position.y << ")\n";
	}

	// ────────────────────────────────────────────────────
	// 2. FixedTimestep による固定間隔更新
	// ────────────────────────────────────────────────────
	std::cout << "\n--- 2. FixedTimestep (200Hz physics, variable frame rate) ---\n";
	{
		FixedTimestep stepper{1.0 / 200.0};  // 200Hzの固定ステップ

		/// 可変フレームレートをシミュレーション（各フレームのdt）
		const double frameTimes[] = {
			0.016,   // 60fps
			0.033,   // 30fps（重いフレーム）
			0.008,   // 120fps
			0.016,
			0.050,   // ラグスパイク
			0.016,
			0.016,
		};

		int totalPhysicsSteps = 0;
		RigidBody2Df obj;
		obj.position = {0.0f, 0.0f};
		obj.velocity = {50.0f, 0.0f};  // 右方向に等速運動

		std::cout << std::fixed << std::setprecision(3);
		for (int frame = 0; frame < 7; ++frame)
		{
			const double dt = frameTimes[frame];
			const int steps = stepper.update(dt, [&](double stepDt)
			{
				obj.integrate(static_cast<float>(stepDt));
			});

			totalPhysicsSteps += steps;
			std::cout << "  Frame " << frame
					  << ": dt=" << dt << "s"
					  << ", physics steps=" << steps
					  << ", position.x=" << std::setprecision(2) << obj.position.x
					  << std::setprecision(3) << "\n";
		}
		std::cout << "  Total physics steps: " << totalPhysicsSteps << "\n";
		std::cout << "  Interpolation factor: " << stepper.interpolationFactor() << "\n";
	}

	// ────────────────────────────────────────────────────
	// 3. 複数物体の衝突シミュレーション
	// ────────────────────────────────────────────────────
	std::cout << "\n--- 3. Multiple Body Collision ---\n";
	std::cout << "  Three balls fall and collide with each other + ground.\n\n";
	{
		/// 3つのボール
		struct Ball
		{
			RigidBody2Df body;
			sgc::Vec2f halfSize{8.0f, 8.0f};
			std::string name;
		};

		std::vector<Ball> balls;

		/// ボール1: 左上から落下
		{
			Ball b;
			b.body.position = {80.0f, 0.0f};
			b.body.velocity = {20.0f, 0.0f};
			b.body.mass = 3.0f;
			b.body.restitution = 0.5f;
			b.body.drag = 0.02f;
			b.name = "Red";
			balls.push_back(std::move(b));
		}
		/// ボール2: 中央上から落下
		{
			Ball b;
			b.body.position = {120.0f, 20.0f};
			b.body.mass = 1.5f;
			b.body.restitution = 0.7f;
			b.body.drag = 0.02f;
			b.name = "Blue";
			balls.push_back(std::move(b));
		}
		/// ボール3: 右上から落下（重い）
		{
			Ball b;
			b.body.position = {160.0f, 10.0f};
			b.body.velocity = {-10.0f, 0.0f};
			b.body.mass = 5.0f;
			b.body.restitution = 0.3f;
			b.body.drag = 0.02f;
			b.name = "Green";
			balls.push_back(std::move(b));
		}

		/// 地面
		RigidBody2Df ground;
		ground.position = {120.0f, 250.0f};
		ground.isStatic = true;
		const sgc::Vec2f groundHalf{200.0f, 10.0f};

		const float dt = 1.0f / 60.0f;
		const float gravity = 500.0f;

		for (int frame = 0; frame < 30; ++frame)
		{
			/// 重力を適用
			for (auto& ball : balls)
			{
				ball.body.applyForce({0.0f, gravity * ball.body.mass});
				ball.body.integrate(dt);
			}

			/// ボール同士の衝突
			for (std::size_t i = 0; i < balls.size(); ++i)
			{
				for (std::size_t j = i + 1; j < balls.size(); ++j)
				{
					const auto aabbA = balls[i].body.bounds(balls[i].halfSize);
					const auto aabbB = balls[j].body.bounds(balls[j].halfSize);
					const auto collision = detectAABBCollision(aabbA, aabbB);
					if (collision.colliding)
					{
						resolveRigidBodyCollision(balls[i].body, balls[j].body, collision);
						if (frame % 5 == 0)
						{
							std::cout << "  [f" << frame << "] Collision: "
									  << balls[i].name << " <-> " << balls[j].name << "\n";
						}
					}
				}
			}

			/// 地面との衝突
			for (auto& ball : balls)
			{
				const auto ballAABB = ball.body.bounds(ball.halfSize);
				const auto groundAABB = ground.bounds(groundHalf);
				const auto collision = detectAABBCollision(ballAABB, groundAABB);
				if (collision.colliding)
				{
					resolveRigidBodyCollision(ball.body, ground, collision);
				}
			}

			/// 10フレームごとに状態表示
			if (frame % 10 == 0)
			{
				std::cout << "  --- Frame " << frame << " ---\n";
				for (const auto& ball : balls)
				{
					std::cout << "    " << std::setw(6) << ball.name
							  << ": pos=(" << std::fixed << std::setprecision(1)
							  << ball.body.position.x << ", " << ball.body.position.y
							  << ") vel=(" << ball.body.velocity.x << ", "
							  << ball.body.velocity.y << ")\n";
				}
			}
		}

		std::cout << "\n  Final positions:\n";
		for (const auto& ball : balls)
		{
			std::cout << "    " << ball.name << ": ("
					  << std::fixed << std::setprecision(1)
					  << ball.body.position.x << ", "
					  << ball.body.position.y << ")\n";
		}
	}

	// ────────────────────────────────────────────────────
	// 4. 衝撃（Impulse）によるジャンプ
	// ────────────────────────────────────────────────────
	std::cout << "\n--- 4. Impulse Jump ---\n";
	{
		RigidBody2Df player;
		player.position = {100.0f, 200.0f};  // 地面レベル
		player.mass = 1.0f;
		player.drag = 0.01f;

		/// ジャンプ衝撃（上方向）
		std::cout << "  Applying upward impulse...\n";
		player.applyImpulse({0.0f, -300.0f});

		const float dt = 1.0f / 60.0f;
		const float gravity = 500.0f;
		float maxHeight = player.position.y;

		for (int frame = 0; frame < 25; ++frame)
		{
			player.applyForce({0.0f, gravity * player.mass});
			player.integrate(dt);

			if (player.position.y < maxHeight)
			{
				maxHeight = player.position.y;
			}

			/// 地面に着地
			if (player.position.y >= 200.0f)
			{
				player.position.y = 200.0f;
				player.velocity.y = 0.0f;
			}

			if (frame % 3 == 0)
			{
				std::cout << "  f" << std::setw(2) << frame << ": ";
				printHeight("Player", player.position.y, 200.0f);
			}
		}
		std::cout << "  Peak height reached: y="
				  << std::fixed << std::setprecision(1) << maxHeight
				  << " (jumped " << (200.0f - maxHeight) << " pixels)\n";
	}

	std::cout << "\n=== Demo Complete ===\n";
	return 0;
}
