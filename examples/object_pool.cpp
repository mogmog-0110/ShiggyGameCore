/// @file object_pool.cpp
/// @brief ObjectPoolでオブジェクトの再利用を実演するサンプル

#include <cstddef>
#include <iostream>
#include <string>
#include <vector>

#include "sgc/patterns/ObjectPool.hpp"

/// @brief 弾丸オブジェクト（プールで管理される典型的なゲームオブジェクト）
struct Bullet
{
	float x{0.0f};      ///< X座標
	float y{0.0f};      ///< Y座標
	float vx{0.0f};     ///< X方向の速度
	float vy{0.0f};     ///< Y方向の速度
	bool active{false};  ///< アクティブフラグ

	/// @brief 弾丸を発射する
	void fire(float startX, float startY, float velX, float velY)
	{
		x = startX;
		y = startY;
		vx = velX;
		vy = velY;
		active = true;
	}

	/// @brief 弾丸を1ステップ移動させる
	void move(float dt)
	{
		x += vx * dt;
		y += vy * dt;
	}
};

/// @brief プールの状態を表示する
void printPoolStatus(const sgc::ObjectPool<Bullet>& pool, const std::string& label)
{
	std::cout << "  [" << label << "] "
		<< "capacity=" << pool.capacity()
		<< ", available=" << pool.available()
		<< ", inUse=" << pool.inUse()
		<< std::endl;
}

/// @brief 弾丸の状態を表示する
void printBullet(const Bullet* b, const std::string& name)
{
	std::cout << "  " << name << ": "
		<< "pos=(" << b->x << ", " << b->y << ") "
		<< "vel=(" << b->vx << ", " << b->vy << ") "
		<< "active=" << (b->active ? "true" : "false")
		<< std::endl;
}

int main()
{
	std::cout << "--- ObjectPool Demo ---" << std::endl;
	std::cout << std::endl;

	// ── 1. プール作成 ───────────────────────────────────────
	constexpr std::size_t POOL_SIZE = 5;
	sgc::ObjectPool<Bullet> pool(POOL_SIZE);

	std::cout << "=== Pool Created ===" << std::endl;
	printPoolStatus(pool, "initial");
	std::cout << std::endl;

	// ── 2. オブジェクトの取得と使用 ─────────────────────────
	std::cout << "=== Acquire & Fire Bullets ===" << std::endl;

	std::vector<Bullet*> activeBullets;

	/// 3発の弾丸を発射
	Bullet* b1 = pool.acquire();
	b1->fire(0.0f, 0.0f, 100.0f, 50.0f);
	activeBullets.push_back(b1);

	Bullet* b2 = pool.acquire();
	b2->fire(10.0f, 5.0f, -80.0f, 120.0f);
	activeBullets.push_back(b2);

	Bullet* b3 = pool.acquire();
	b3->fire(50.0f, 50.0f, 0.0f, -200.0f);
	activeBullets.push_back(b3);

	printPoolStatus(pool, "after 3 acquires");
	for (std::size_t i = 0; i < activeBullets.size(); ++i)
	{
		printBullet(activeBullets[i], "bullet" + std::to_string(i + 1));
	}
	std::cout << std::endl;

	// ── 3. シミュレーション（移動） ─────────────────────────
	std::cout << "=== Simulate Movement (dt=0.016) ===" << std::endl;
	constexpr float dt = 0.016f;
	for (auto* b : activeBullets)
	{
		b->move(dt);
	}
	for (std::size_t i = 0; i < activeBullets.size(); ++i)
	{
		printBullet(activeBullets[i], "bullet" + std::to_string(i + 1));
	}
	std::cout << std::endl;

	// ── 4. オブジェクトの返却 ───────────────────────────────
	std::cout << "=== Release bullet2 ===" << std::endl;
	pool.release(b2);
	activeBullets.erase(activeBullets.begin() + 1);
	printPoolStatus(pool, "after release");
	std::cout << std::endl;

	// ── 5. 再利用の確認 ─────────────────────────────────────
	std::cout << "=== Reuse Released Slot ===" << std::endl;
	Bullet* b4 = pool.acquire();
	b4->fire(200.0f, 100.0f, 50.0f, 50.0f);
	activeBullets.push_back(b4);

	/// 返却されたメモリが再利用されたことを確認
	const bool reused = (b4 == b2);
	std::cout << "  b4 reuses b2's memory: " << (reused ? "YES" : "NO") << std::endl;
	printBullet(b4, "bullet4 (reused)");
	printPoolStatus(pool, "after reuse");
	std::cout << std::endl;

	// ── 6. プール枯渇 ───────────────────────────────────────
	std::cout << "=== Pool Exhaustion ===" << std::endl;

	/// 残り全てを取得
	while (pool.available() > 0)
	{
		Bullet* b = pool.acquire();
		b->fire(0.0f, 0.0f, 1.0f, 1.0f);
	}
	printPoolStatus(pool, "fully used");

	/// プール枯渇時はnullptrが返る
	Bullet* overflow = pool.acquire();
	std::cout << "  acquire when empty: "
		<< (overflow == nullptr ? "nullptr (expected)" : "ERROR")
		<< std::endl;

	return 0;
}
