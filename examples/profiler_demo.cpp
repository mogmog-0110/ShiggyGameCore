/// @file profiler_demo.cpp
/// @brief Profilerでコードセクションの実行時間を計測するサンプル
///
/// SGC_PROFILE_SCOPE / SGC_PROFILE_FUNCTION マクロと
/// Profiler::allStats() を使って計測結果を一覧表示する。

#include <algorithm>
#include <chrono>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <thread>
#include <vector>

#include "sgc/core/Profiler.hpp"

// ────────────────────────────────────────────────────────
// 計測対象の関数群
// ────────────────────────────────────────────────────────

/// @brief 物理シミュレーション風の計算処理
void simulatePhysics()
{
	SGC_PROFILE_FUNCTION();

	/// 重い計算をシミュレート
	volatile double sum = 0.0;
	for (int i = 0; i < 100000; ++i)
	{
		sum += std::sin(static_cast<double>(i) * 0.001);
	}
}

/// @brief レンダリング風の処理（内部にネストされたプロファイルセクション）
void render()
{
	SGC_PROFILE_FUNCTION();

	{
		SGC_PROFILE_SCOPE("Render/SortSprites");
		/// スプライトソートをシミュレート
		std::vector<int> sprites(1000);
		for (int i = 0; i < 1000; ++i)
		{
			sprites[static_cast<std::size_t>(i)] = 1000 - i;
		}
		std::sort(sprites.begin(), sprites.end());
	}

	{
		SGC_PROFILE_SCOPE("Render/DrawCalls");
		/// 描画呼び出しをシミュレート
		std::this_thread::sleep_for(std::chrono::microseconds(500));
	}

	{
		SGC_PROFILE_SCOPE("Render/PostProcess");
		/// ポストプロセスをシミュレート
		volatile double v = 0.0;
		for (int i = 0; i < 50000; ++i)
		{
			v += std::cos(static_cast<double>(i) * 0.002);
		}
	}
}

/// @brief AI更新風の処理
void updateAI()
{
	SGC_PROFILE_FUNCTION();

	std::this_thread::sleep_for(std::chrono::microseconds(200));
}

/// @brief オーディオ処理風の処理
void processAudio()
{
	SGC_PROFILE_FUNCTION();

	std::this_thread::sleep_for(std::chrono::microseconds(100));
}

/// @brief ゲームループ1フレーム分の処理
void gameFrame()
{
	SGC_PROFILE_SCOPE("GameFrame");

	simulatePhysics();
	updateAI();
	render();
	processAudio();
}

// ────────────────────────────────────────────────────────
// 計測結果の表示
// ────────────────────────────────────────────────────────

/// @brief 計測結果をテーブル形式で表示する
void printProfileResults()
{
	const auto stats = sgc::Profiler::instance().allStats();

	std::cout << std::fixed << std::setprecision(4);
	std::cout << "  +--------------------------+-------+----------+----------+----------+\n";
	std::cout << "  | Section                  | Calls |  Avg(ms) |  Min(ms) |  Max(ms) |\n";
	std::cout << "  +--------------------------+-------+----------+----------+----------+\n";

	for (const auto& s : stats)
	{
		/// セクション名を最大24文字にトリミング
		std::string name = s.name;
		if (name.length() > 24)
		{
			name = name.substr(0, 21) + "...";
		}

		std::cout << "  | " << std::left << std::setw(24) << name
				  << " | " << std::right << std::setw(5) << s.callCount
				  << " | " << std::setw(8) << s.averageMs
				  << " | " << std::setw(8) << s.minMs
				  << " | " << std::setw(8) << s.maxMs
				  << " |\n";
	}

	std::cout << "  +--------------------------+-------+----------+----------+----------+\n";
}

// ────────────────────────────────────────────────────────
// メイン
// ────────────────────────────────────────────────────────

int main()
{
	std::cout << "=== sgc Profiler Demo ===\n\n";

	/// プロファイラをリセット（前回のデータがあれば消去）
	sgc::Profiler::instance().reset();

	// ── 1. 基本的なスコープ計測 ──────────────────────────
	std::cout << "[1] Basic scoped profiling:\n";
	{
		SGC_PROFILE_SCOPE("BasicTest");
		std::this_thread::sleep_for(std::chrono::milliseconds(5));
	}
	{
		const auto s = sgc::Profiler::instance().stats("BasicTest");
		std::cout << "  BasicTest: " << std::fixed << std::setprecision(2)
				  << s.averageMs << " ms (1 call)\n";
	}
	std::cout << "\n";

	/// リセットして本計測に備える
	sgc::Profiler::instance().reset();

	// ── 2. ゲームループシミュレーション ──────────────────
	std::cout << "[2] Simulating 10 game frames...\n";
	for (int frame = 0; frame < 10; ++frame)
	{
		gameFrame();
	}
	std::cout << "  Done.\n\n";

	// ── 3. 計測結果の表示 ────────────────────────────────
	std::cout << "[3] Profile Results (10 frames):\n";
	printProfileResults();
	std::cout << "\n";

	// ── 4. 個別セクションの詳細 ──────────────────────────
	std::cout << "[4] Detailed stats for 'GameFrame':\n";
	{
		const auto s = sgc::Profiler::instance().stats("GameFrame");
		std::cout << std::fixed << std::setprecision(4);
		std::cout << "  Total time: " << s.totalMs << " ms\n";
		std::cout << "  Average:    " << s.averageMs << " ms\n";
		std::cout << "  Min:        " << s.minMs << " ms\n";
		std::cout << "  Max:        " << s.maxMs << " ms\n";
		std::cout << "  Calls:      " << s.callCount << "\n";
	}
	std::cout << "\n";

	// ── 5. 手動begin/end計測 ─────────────────────────────
	std::cout << "[5] Manual begin/end profiling:\n";
	{
		sgc::Profiler::instance().beginSection("ManualSection");
		std::this_thread::sleep_for(std::chrono::milliseconds(2));
		sgc::Profiler::instance().endSection("ManualSection");

		const auto s = sgc::Profiler::instance().stats("ManualSection");
		std::cout << "  ManualSection: " << std::fixed << std::setprecision(2)
				  << s.averageMs << " ms\n";
	}

	std::cout << "\n=== Demo Complete ===\n";
	return 0;
}
