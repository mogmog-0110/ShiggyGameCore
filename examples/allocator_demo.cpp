/// @file allocator_demo.cpp
/// @brief Arena・Pool・FreeListアロケータの使い方を実演するサンプル
///
/// sgcのカスタムアロケータ群の基本操作と、
/// StlAllocatorAdapterでSTLコンテナと組み合わせる方法を紹介する。

#include <iostream>
#include <string>
#include <vector>

#include "sgc/memory/ArenaAllocator.hpp"
#include "sgc/memory/PoolAllocator.hpp"
#include "sgc/memory/FreeListAllocator.hpp"
#include "sgc/memory/StlAllocatorAdapter.hpp"

/// @brief メモリ使用量を表示するヘルパー
static void printMemoryUsage(const char* label, std::size_t used, std::size_t total)
{
	std::cout << "  " << label << ": "
			  << used << " / " << total << " bytes ("
			  << (total > 0 ? (used * 100 / total) : 0) << "% used)\n";
}

/// @brief ゲームで使いそうな構造体（パーティクル）
struct Particle
{
	float x, y;
	float vx, vy;
	float life;
};

/// @brief ゲームで使いそうな構造体（変換行列的なもの）
struct Transform
{
	float posX, posY, posZ;
	float rotX, rotY, rotZ;
	float scaleX, scaleY, scaleZ;
};

int main()
{
	std::cout << "=== sgc Allocator Demo ===\n\n";

	// ────────────────────────────────────────────────────
	// 1. ArenaAllocator: 線形割り当て、一括リセット
	// ────────────────────────────────────────────────────
	std::cout << "[1] ArenaAllocator (1024 bytes):\n";
	{
		sgc::ArenaAllocator arena(1024);
		printMemoryUsage("Initial", arena.used(), arena.capacity());

		/// create<T>() でオブジェクトを構築
		auto* p1 = arena.create<Particle>(10.0f, 20.0f, 1.0f, -0.5f, 3.0f);
		std::cout << "  Allocated Particle: (" << p1->x << ", " << p1->y
				  << ") life=" << p1->life << "\n";
		printMemoryUsage("After Particle", arena.used(), arena.capacity());

		auto* t1 = arena.create<Transform>(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);
		std::cout << "  Allocated Transform: scale=("
				  << t1->scaleX << ", " << t1->scaleY << ", " << t1->scaleZ << ")\n";
		printMemoryUsage("After Transform", arena.used(), arena.capacity());

		/// 配列的に連続割り当て
		constexpr int batchSize = 10;
		for (int i = 0; i < batchSize; ++i)
		{
			(void)arena.create<float>(static_cast<float>(i) * 1.5f);
		}
		printMemoryUsage("After 10 floats", arena.used(), arena.capacity());

		/// reset() で全メモリを一括リセット（個別解放は不要）
		arena.reset();
		printMemoryUsage("After reset()", arena.used(), arena.capacity());
		std::cout << "  Remaining: " << arena.remaining() << " bytes\n";
	}
	std::cout << "\n";

	// ────────────────────────────────────────────────────
	// 2. PoolAllocator: 固定サイズブロック、O(1)割り当て・解放
	// ────────────────────────────────────────────────────
	std::cout << "[2] PoolAllocator (Particle x 8 blocks):\n";
	{
		constexpr std::size_t blockCount = 8;
		sgc::PoolAllocator pool(sizeof(Particle), alignof(Particle), blockCount);
		std::cout << "  Block size: " << pool.blockSize() << " bytes\n";
		std::cout << "  Free blocks: " << pool.freeCount() << " / " << pool.blockCount() << "\n";

		/// 5つのパーティクルを割り当て
		std::vector<Particle*> particles;
		for (int i = 0; i < 5; ++i)
		{
			auto* mem = pool.allocate(sizeof(Particle));
			auto* p = new (mem) Particle{
				static_cast<float>(i * 10), static_cast<float>(i * 5),
				1.0f, -1.0f, 2.0f + static_cast<float>(i)
			};
			particles.push_back(p);
		}
		std::cout << "  After allocating 5: free=" << pool.freeCount() << "\n";

		/// 2つを解放してプールに戻す
		particles[1]->~Particle();
		pool.deallocate(particles[1]);
		particles[3]->~Particle();
		pool.deallocate(particles[3]);
		std::cout << "  After deallocating 2: free=" << pool.freeCount() << "\n";

		/// 再割り当て（解放済みブロックが再利用される）
		auto* reused = pool.allocate(sizeof(Particle));
		auto* p = new (reused) Particle{99.0f, 99.0f, 0.0f, 0.0f, 1.0f};
		std::cout << "  Reused block: (" << p->x << ", " << p->y << ")\n";
		std::cout << "  After reuse: free=" << pool.freeCount() << "\n";

		/// reset() で全ブロックをリセット
		pool.reset();
		std::cout << "  After reset(): free=" << pool.freeCount() << "\n";
	}
	std::cout << "\n";

	// ────────────────────────────────────────────────────
	// 3. FreeListAllocator: 可変サイズ割り当て・解放・結合
	// ────────────────────────────────────────────────────
	std::cout << "[3] FreeListAllocator (4096 bytes, BestFit):\n";
	{
		sgc::FreeListAllocator freelist(4096, sgc::FitPolicy::BestFit);
		printMemoryUsage("Initial", freelist.used(), freelist.capacity());

		/// 異なるサイズのオブジェクトを割り当て
		auto* intVal = freelist.create<int>(42);
		std::cout << "  Allocated int: " << *intVal << "\n";
		printMemoryUsage("After int", freelist.used(), freelist.capacity());

		auto* particle = freelist.create<Particle>(5.0f, 10.0f, 0.0f, 0.0f, 1.0f);
		std::cout << "  Allocated Particle: (" << particle->x << ", " << particle->y << ")\n";
		printMemoryUsage("After Particle", freelist.used(), freelist.capacity());

		auto* transform = freelist.create<Transform>(1.0f, 2.0f, 3.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);
		std::cout << "  Allocated Transform: pos=("
				  << transform->posX << ", " << transform->posY << ", " << transform->posZ << ")\n";
		printMemoryUsage("After Transform", freelist.used(), freelist.capacity());

		/// 中間のオブジェクトを解放（コアレスが発動しうる）
		freelist.deallocate(particle);
		std::cout << "  Deallocated Particle\n";
		printMemoryUsage("After dealloc", freelist.used(), freelist.capacity());

		/// 全リセット
		freelist.reset();
		printMemoryUsage("After reset()", freelist.used(), freelist.capacity());
	}
	std::cout << "\n";

	// ────────────────────────────────────────────────────
	// 4. StlAllocatorAdapter: カスタムアロケータ + STLコンテナ
	// ────────────────────────────────────────────────────
	std::cout << "[4] StlAllocatorAdapter (FreeListAllocator + std::vector):\n";
	{
		sgc::FreeListAllocator freelist(8192);

		/// makeStlAllocator<T>() でSTL互換アロケータを生成
		auto stlAlloc = sgc::makeStlAllocator<int>(freelist);
		std::vector<int, decltype(stlAlloc)> scores(stlAlloc);

		/// std::vectorの通常操作がそのまま使える
		scores.push_back(100);
		scores.push_back(250);
		scores.push_back(180);
		scores.push_back(320);
		scores.push_back(95);

		std::cout << "  Scores: ";
		for (const int s : scores)
		{
			std::cout << s << " ";
		}
		std::cout << "\n";
		printMemoryUsage("FreeList usage", freelist.used(), freelist.capacity());

		/// 別の型のvectorも同一アロケータから割り当て可能
		auto floatAlloc = sgc::makeStlAllocator<float>(freelist);
		std::vector<float, decltype(floatAlloc)> positions(floatAlloc);
		positions.push_back(1.5f);
		positions.push_back(3.7f);
		positions.push_back(9.2f);

		std::cout << "  Positions: ";
		for (const float p : positions)
		{
			std::cout << p << " ";
		}
		std::cout << "\n";
		printMemoryUsage("After both vectors", freelist.used(), freelist.capacity());
	}
	std::cout << "\n";

	// ────────────────────────────────────────────────────
	// 5. 比較: 各アロケータの特性まとめ
	// ────────────────────────────────────────────────────
	std::cout << "[5] Allocator Comparison:\n";
	std::cout << "  +-------------------+------------+------------+-----------+\n";
	std::cout << "  | Allocator         | Alloc O()  | Dealloc    | Use Case  |\n";
	std::cout << "  +-------------------+------------+------------+-----------+\n";
	std::cout << "  | ArenaAllocator    | O(1)       | N/A(reset) | Per-frame |\n";
	std::cout << "  | PoolAllocator     | O(1)       | O(1)       | Particles |\n";
	std::cout << "  | FreeListAllocator | O(n)       | O(n)       | General   |\n";
	std::cout << "  +-------------------+------------+------------+-----------+\n";

	std::cout << "\n=== Demo Complete ===\n";
	return 0;
}
