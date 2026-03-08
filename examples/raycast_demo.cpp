/// @file raycast_demo.cpp
/// @brief 2Dレイキャストでオブジェクト検出を実演するサンプル
///
/// sgcのRayCast2Dを使って以下をデモする:
/// - 複数のAABB（壁）に対するレイキャスト
/// - 円（敵）に対するレイキャスト
/// - 複数方向へのレイ発射
/// - ヒット距離・衝突点・法線の取得
/// - maxDistanceによる検出距離制限
/// - 視野シミュレーション（扇状レイキャスト）

#include <cmath>
#include <iomanip>
#include <iostream>
#include <numbers>
#include <string>
#include <vector>

#include "sgc/physics/RayCast2D.hpp"
#include "sgc/math/Geometry.hpp"

/// @brief ヒット結果を整形表示する
/// @param rayName レイの名前
/// @param hit レイキャスト結果
void printHitResult(const std::string& rayName, const sgc::physics::RayCastHit2Df& hit)
{
	std::cout << "  " << std::setw(14) << rayName << ": ";
	if (hit.hit)
	{
		std::cout << "HIT at (" << std::fixed << std::setprecision(1)
				  << hit.point.x << ", " << hit.point.y << ")"
				  << "  dist=" << hit.distance
				  << "  normal=(" << hit.normal.x << ", " << hit.normal.y << ")\n";
	}
	else
	{
		std::cout << "MISS (no intersection)\n";
	}
}

/// @brief 方向ベクトルを角度（度）から生成する
/// @param degrees 角度（0度=右、90度=下）
/// @return 正規化された方向ベクトル
sgc::Vec2f directionFromDegrees(float degrees)
{
	const float rad = degrees * std::numbers::pi_v<float> / 180.0f;
	return {std::cos(rad), std::sin(rad)};
}

int main()
{
	std::cout << "=== sgc RayCast2D Demo ===\n\n";

	using namespace sgc::physics;

	// ── シーン構築 ──────────────────────────────────────────
	// 簡易マップ: プレイヤーの周囲に壁と敵を配置
	//
	//   (0,0)
	//     +--------------------+
	//     |   [Wall A]         |
	//     |                    |
	//     |       P -->        |  P = プレイヤー位置 (50, 100)
	//     |                    |
	//     |            (Enemy) |  円形の敵
	//     |   [Wall B]         |
	//     +--------------------+
	//                     (200,200)

	/// 壁A: 上部の横壁
	const sgc::AABB2f wallA{{20.0f, 20.0f}, {180.0f, 40.0f}};

	/// 壁B: 下部の横壁
	const sgc::AABB2f wallB{{20.0f, 160.0f}, {180.0f, 180.0f}};

	/// 壁C: 右側の縦壁
	const sgc::AABB2f wallC{{170.0f, 40.0f}, {190.0f, 160.0f}};

	/// 壁D: 小さな障害物（中央やや右）
	const sgc::AABB2f wallD{{110.0f, 80.0f}, {130.0f, 95.0f}};

	/// 敵（円形）
	const sgc::Vec2f enemyCenter{150.0f, 130.0f};
	const float enemyRadius = 15.0f;

	/// プレイヤー位置（レイの始点）
	const sgc::Vec2f playerPos{50.0f, 100.0f};

	std::cout << "  Scene Layout:\n";
	std::cout << "    Player: (" << playerPos.x << ", " << playerPos.y << ")\n";
	std::cout << "    Wall A: (20,20)-(180,40)   [top wall]\n";
	std::cout << "    Wall B: (20,160)-(180,180)  [bottom wall]\n";
	std::cout << "    Wall C: (170,40)-(190,160)  [right wall]\n";
	std::cout << "    Wall D: (110,80)-(130,95)   [obstacle]\n";
	std::cout << "    Enemy:  center=(150,130) r=15 [circle]\n\n";

	// ────────────────────────────────────────────────────
	// 1. 4方向へのレイキャスト（壁検出）
	// ────────────────────────────────────────────────────
	std::cout << "--- 1. Directional Raycasts (walls only) ---\n";
	{
		struct NamedRay
		{
			std::string name;
			sgc::Vec2f direction;
		};

		const NamedRay rays[] = {
			{"Right",      {1.0f, 0.0f}},
			{"Left",       {-1.0f, 0.0f}},
			{"Up",         {0.0f, -1.0f}},
			{"Down",       {0.0f, 1.0f}},
			{"Upper-Right", {0.707f, -0.707f}},
			{"Lower-Right", {0.707f, 0.707f}},
		};

		const sgc::AABB2f* walls[] = {&wallA, &wallB, &wallC, &wallD};
		const std::string wallNames[] = {"Wall A", "Wall B", "Wall C", "Wall D"};

		for (const auto& ray : rays)
		{
			float closestDist = 1e10f;
			RayCastHit2Df closestHit;
			std::string hitName = "nothing";

			for (int i = 0; i < 4; ++i)
			{
				const auto hit = raycastAABB(playerPos, ray.direction, *walls[i]);
				if (hit.hit && hit.distance < closestDist)
				{
					closestDist = hit.distance;
					closestHit = hit;
					hitName = wallNames[i];
				}
			}

			std::cout << "  " << std::setw(14) << ray.name << ": ";
			if (closestHit.hit)
			{
				std::cout << "hits " << hitName
						  << " at dist=" << std::fixed << std::setprecision(1)
						  << closestDist
						  << " point=(" << closestHit.point.x << ", "
						  << closestHit.point.y << ")\n";
			}
			else
			{
				std::cout << "misses all walls\n";
			}
		}
	}

	// ────────────────────────────────────────────────────
	// 2. 円（敵）へのレイキャスト
	// ────────────────────────────────────────────────────
	std::cout << "\n--- 2. Raycast vs Circle (enemy detection) ---\n";
	{
		/// プレイヤーから敵方向へのベクトルを計算
		const sgc::Vec2f toEnemy = enemyCenter - playerPos;
		const float len = std::sqrt(toEnemy.x * toEnemy.x + toEnemy.y * toEnemy.y);
		const sgc::Vec2f dirToEnemy = {toEnemy.x / len, toEnemy.y / len};

		std::cout << "  Direction to enemy: ("
				  << std::fixed << std::setprecision(3)
				  << dirToEnemy.x << ", " << dirToEnemy.y << ")\n";

		/// 敵に向かってレイを撃つ
		const auto hit = raycastCircle(playerPos, dirToEnemy,
									   enemyCenter, enemyRadius);
		printHitResult("Direct Ray", hit);

		/// 少しずれた方向（外す）
		const sgc::Vec2f missDir = {dirToEnemy.x + 0.3f, dirToEnemy.y - 0.3f};
		const float missLen = std::sqrt(missDir.x * missDir.x + missDir.y * missDir.y);
		const sgc::Vec2f missDirNorm = {missDir.x / missLen, missDir.y / missLen};

		const auto missHit = raycastCircle(playerPos, missDirNorm,
										   enemyCenter, enemyRadius);
		printHitResult("Off-target Ray", missHit);
	}

	// ────────────────────────────────────────────────────
	// 3. maxDistance による検出距離制限
	// ────────────────────────────────────────────────────
	std::cout << "\n--- 3. maxDistance Limiting ---\n";
	{
		const sgc::Vec2f dir{1.0f, 0.0f};  // 右方向

		/// 制限なし: Wall Dにヒット
		const auto hitFar = raycastAABB(playerPos, dir, wallD);
		std::cout << "  maxDistance=inf: ";
		if (hitFar.hit)
		{
			std::cout << "HIT Wall D at dist=" << std::fixed
					  << std::setprecision(1) << hitFar.distance << "\n";
		}

		/// 50ピクセル以内に制限: Wall Dは60ピクセル先なので届かない
		const auto hitShort = raycastAABB(playerPos, dir, wallD, 50.0f);
		std::cout << "  maxDistance=50:  ";
		if (hitShort.hit)
		{
			std::cout << "HIT at dist=" << hitShort.distance << "\n";
		}
		else
		{
			std::cout << "MISS (wall is beyond 50px range)\n";
		}

		/// 70ピクセル以内に制限: ギリギリ届く
		const auto hitMedium = raycastAABB(playerPos, dir, wallD, 70.0f);
		std::cout << "  maxDistance=70:  ";
		if (hitMedium.hit)
		{
			std::cout << "HIT at dist=" << std::fixed
					  << std::setprecision(1) << hitMedium.distance << "\n";
		}
		else
		{
			std::cout << "MISS\n";
		}
	}

	// ────────────────────────────────────────────────────
	// 4. 視野シミュレーション（扇状レイキャスト）
	// ────────────────────────────────────────────────────
	std::cout << "\n--- 4. Field of View Simulation (fan-shaped rays) ---\n";
	{
		/// 前方（右向き = 0度）を中心に、±45度の視野を12本のレイで走査
		const float fovHalf = 45.0f;  // 片側45度 = 合計90度の視野
		const int rayCount = 12;
		const float maxViewDist = 200.0f;

		const sgc::AABB2f* walls[] = {&wallA, &wallB, &wallC, &wallD};

		std::cout << "  Player at (" << playerPos.x << ", " << playerPos.y
				  << "), FOV=90 degrees, range=" << maxViewDist << "px\n";
		std::cout << "  Scanning " << rayCount << " rays...\n\n";

		int wallHits = 0;
		int enemyHits = 0;

		for (int i = 0; i < rayCount; ++i)
		{
			/// 角度を均等に分割（-45度 ～ +45度）
			const float angle = -fovHalf + (2.0f * fovHalf * i) / (rayCount - 1);
			const sgc::Vec2f dir = directionFromDegrees(angle);

			/// 壁の中で最も近いヒットを探す
			float closestDist = maxViewDist;
			std::string hitTarget = "nothing";
			bool anyHit = false;

			for (int w = 0; w < 4; ++w)
			{
				const auto hit = raycastAABB(playerPos, dir, *walls[w], maxViewDist);
				if (hit.hit && hit.distance < closestDist)
				{
					closestDist = hit.distance;
					hitTarget = "Wall";
					anyHit = true;
				}
			}

			/// 敵（円）との判定
			const auto enemyHit = raycastCircle(playerPos, dir,
												enemyCenter, enemyRadius, maxViewDist);
			if (enemyHit.hit && enemyHit.distance < closestDist)
			{
				closestDist = enemyHit.distance;
				hitTarget = "ENEMY!";
				anyHit = true;
				++enemyHits;
			}
			else if (anyHit)
			{
				++wallHits;
			}

			/// レイの結果をバーで可視化
			const int barLen = 30;
			const int filled = anyHit
				? static_cast<int>((closestDist / maxViewDist) * barLen)
				: barLen;
			const int clampedFilled = (filled > barLen) ? barLen : filled;

			std::cout << "  " << std::setw(6) << std::fixed << std::setprecision(1)
					  << angle << " deg: ";
			for (int b = 0; b < barLen; ++b)
			{
				if (b < clampedFilled)
				{
					std::cout << '-';
				}
				else if (b == clampedFilled)
				{
					std::cout << '*';
				}
				else
				{
					std::cout << ' ';
				}
			}
			std::cout << " " << std::setw(8) << hitTarget;
			if (anyHit)
			{
				std::cout << " d=" << std::setprecision(1) << closestDist;
			}
			std::cout << "\n";
		}

		std::cout << "\n  Summary: " << wallHits << " wall hits, "
				  << enemyHits << " enemy detections\n";
	}

	// ────────────────────────────────────────────────────
	// 5. 衝突法線の活用（反射ベクトル計算）
	// ────────────────────────────────────────────────────
	std::cout << "\n--- 5. Reflection Using Hit Normal ---\n";
	{
		/// 弾丸をプレイヤー位置から右上壁に向かって発射
		const sgc::Vec2f bulletDir = directionFromDegrees(-30.0f);  // 右上方向
		std::cout << "  Bullet direction: ("
				  << std::fixed << std::setprecision(3)
				  << bulletDir.x << ", " << bulletDir.y << ")\n";

		const auto hit = raycastAABB(playerPos, bulletDir, wallA);

		if (hit.hit)
		{
			std::cout << "  Hit Wall A at ("
					  << std::setprecision(1) << hit.point.x << ", "
					  << hit.point.y << ")\n";
			std::cout << "  Normal: (" << hit.normal.x << ", " << hit.normal.y << ")\n";

			/// 反射ベクトル: R = D - 2(D . N)N
			const float dotDN = bulletDir.x * hit.normal.x + bulletDir.y * hit.normal.y;
			const sgc::Vec2f reflected = {
				bulletDir.x - 2.0f * dotDN * hit.normal.x,
				bulletDir.y - 2.0f * dotDN * hit.normal.y
			};

			std::cout << "  Reflected direction: ("
					  << std::setprecision(3)
					  << reflected.x << ", " << reflected.y << ")\n";
			std::cout << "  (Bullet bounces off the wall surface!)\n";
		}
	}

	std::cout << "\n=== Demo Complete ===\n";
	return 0;
}
