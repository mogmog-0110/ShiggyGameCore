/// @file coroutine_demo.cpp
/// @brief コルーチンで段階的な処理を実演するサンプル
///
/// sgc::Task（co_await対応）と sgc::Generator（co_yield対応）、
/// CoroutineSchedulerの使い方を紹介する。

#include <iostream>
#include <string>
#include <vector>

#include "sgc/core/Coroutine.hpp"

// ────────────────────────────────────────────────────────
// 1. Generator: 遅延シーケンス生成
// ────────────────────────────────────────────────────────

/// @brief 指定範囲の整数を遅延生成するジェネレータ
sgc::Generator<int> range(int start, int end)
{
	for (int i = start; i < end; ++i)
	{
		co_yield i;
	}
}

/// @brief フィボナッチ数列を指定個数だけ生成するジェネレータ
sgc::Generator<int> fibonacci(int count)
{
	int a = 0;
	int b = 1;
	for (int i = 0; i < count; ++i)
	{
		co_yield a;
		const int next = a + b;
		a = b;
		b = next;
	}
}

/// @brief 敵ウェーブの出現データを生成するジェネレータ（ゲーム実用例）
struct WaveData
{
	int waveNumber;
	int enemyCount;
	std::string enemyType;
};

sgc::Generator<WaveData> waveSpawner()
{
	co_yield WaveData{1, 3, "Slime"};
	co_yield WaveData{2, 5, "Goblin"};
	co_yield WaveData{3, 2, "Dragon"};
}

// ────────────────────────────────────────────────────────
// 2. Task + CoroutineScheduler: フレームベース非同期処理
// ────────────────────────────────────────────────────────

/// @brief カウントダウンタイマー（1秒ごとにカウントダウン）
sgc::Task countdownTimer(int seconds)
{
	for (int i = seconds; i > 0; --i)
	{
		std::cout << "  [Countdown] " << i << "...\n";
		co_await sgc::WaitForSeconds{1.0f};
	}
	std::cout << "  [Countdown] GO!\n";
}

/// @brief ダイアログテキストの段階的表示
sgc::Task dialogSequence()
{
	const std::vector<std::string> lines = {
		"Hero: The dungeon lies ahead...",
		"Guide: Be careful, monsters lurk within.",
		"Hero: I'm ready. Let's go!"
	};

	for (const auto& line : lines)
	{
		std::cout << "  [Dialog] " << line << "\n";
		/// 各セリフの後に0.5秒待機
		co_await sgc::WaitForSeconds{0.5f};
	}
	std::cout << "  [Dialog] (Dialog complete)\n";
}

/// @brief 条件待機の実演（外部フラグが立つまで待つ）
static bool g_dataLoaded = false;

sgc::Task waitForDataLoad()
{
	std::cout << "  [Loader] Waiting for data...\n";
	co_await sgc::WaitUntil{[]() { return g_dataLoaded; }};
	std::cout << "  [Loader] Data loaded! Processing...\n";
	co_await sgc::WaitForNextFrame{};
	std::cout << "  [Loader] Processing complete.\n";
}

// ────────────────────────────────────────────────────────
// メイン
// ────────────────────────────────────────────────────────

int main()
{
	std::cout << "=== sgc Coroutine Demo ===\n\n";

	// ── Generator: range ────────────────────────────────
	std::cout << "[1] Generator - range(1, 6):\n  ";
	for (const int v : range(1, 6))
	{
		std::cout << v << " ";
	}
	std::cout << "\n\n";

	// ── Generator: フィボナッチ数列 ─────────────────────
	std::cout << "[2] Generator - fibonacci(10):\n  ";
	for (const int v : fibonacci(10))
	{
		std::cout << v << " ";
	}
	std::cout << "\n\n";

	// ── Generator: ウェーブスポーナー ────────────────────
	std::cout << "[3] Generator - Wave Spawner:\n";
	for (const auto& wave : waveSpawner())
	{
		std::cout << "  Wave " << wave.waveNumber
				  << ": " << wave.enemyCount << "x " << wave.enemyType << "\n";
	}
	std::cout << "\n";

	// ── CoroutineScheduler: フレームシミュレーション ─────
	std::cout << "[4] CoroutineScheduler - Countdown + Dialog:\n";

	sgc::CoroutineScheduler scheduler;
	scheduler.start(countdownTimer(3));
	scheduler.start(dialogSequence());

	/// ゲームループをシミュレーション（1フレーム = 0.25秒で回す）
	constexpr float dt = 0.25f;
	int frame = 0;
	while (scheduler.activeCount() > 0)
	{
		scheduler.tick(dt);
		++frame;
	}
	std::cout << "  (Completed in " << frame << " frames, "
			  << static_cast<float>(frame) * dt << "s simulated)\n\n";

	// ── CoroutineScheduler: 条件待機 ────────────────────
	std::cout << "[5] CoroutineScheduler - WaitUntil:\n";

	sgc::CoroutineScheduler scheduler2;
	scheduler2.start(waitForDataLoad());

	/// 5フレーム後に外部フラグを立てる
	for (int i = 0; i < 10; ++i)
	{
		if (i == 5)
		{
			std::cout << "  [Main] Setting data loaded flag (frame " << i << ")\n";
			g_dataLoaded = true;
		}
		scheduler2.tick(dt);
		if (scheduler2.activeCount() == 0)
		{
			break;
		}
	}
	std::cout << "\n";

	// ── Task: 完了検出とキャンセル ───────────────────────
	std::cout << "[6] Task - Completion detection & cancel:\n";

	sgc::CoroutineScheduler scheduler3;
	const auto id = scheduler3.start(countdownTimer(5));
	std::cout << "  Active coroutines: " << scheduler3.activeCount() << "\n";

	/// 2フレーム分だけ進めてからキャンセル
	scheduler3.tick(1.0f);
	scheduler3.tick(1.0f);
	std::cout << "  Cancelling remaining coroutine...\n";
	scheduler3.cancel(id);
	std::cout << "  Active coroutines after cancel: " << scheduler3.activeCount() << "\n";

	std::cout << "\n=== Demo Complete ===\n";
	return 0;
}
