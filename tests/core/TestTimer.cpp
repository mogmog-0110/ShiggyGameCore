/// @file TestTimer.cpp
/// @brief Timer.hpp のユニットテスト

#include <catch2/catch_test_macros.hpp>

#include "sgc/core/Timer.hpp"

#include <thread>

TEST_CASE("Stopwatch measures elapsed time", "[core][timer]")
{
	sgc::Stopwatch sw;
	sw.start();
	std::this_thread::sleep_for(std::chrono::milliseconds(10));
	sw.stop();

	REQUIRE(sw.elapsedMilliseconds() > 5.0);
	REQUIRE(sw.elapsedSeconds() > 0.005);
}

TEST_CASE("Stopwatch restart resets and starts", "[core][timer]")
{
	sgc::Stopwatch sw;
	sw.start();
	std::this_thread::sleep_for(std::chrono::milliseconds(10));
	sw.restart();

	REQUIRE(sw.isRunning());
	REQUIRE(sw.elapsedMilliseconds() < 10.0);
}

TEST_CASE("DeltaClock returns positive delta", "[core][timer]")
{
	sgc::DeltaClock clock;
	std::this_thread::sleep_for(std::chrono::milliseconds(5));
	float dt = clock.tick();

	REQUIRE(dt > 0.0f);
	REQUIRE(dt < 1.0f);
}

TEST_CASE("Stopwatch reset clears elapsed and stops", "[core][timer]")
{
	sgc::Stopwatch sw;
	sw.start();
	std::this_thread::sleep_for(std::chrono::milliseconds(10));
	sw.reset();

	REQUIRE_FALSE(sw.isRunning());
	REQUIRE(sw.elapsedMilliseconds() < 1.0);
}

TEST_CASE("DeltaClock reset makes next tick small", "[core][timer]")
{
	sgc::DeltaClock clock;
	std::this_thread::sleep_for(std::chrono::milliseconds(20));
	(void)clock.tick(); // 消費

	clock.reset();
	float dt = clock.tick();
	// reset直後のtickは非常に小さいデルタを返す
	REQUIRE(dt < 0.05f);
}

TEST_CASE("Stopwatch elapsedSecondsF returns float", "[core][timer]")
{
	sgc::Stopwatch sw;
	sw.start();
	std::this_thread::sleep_for(std::chrono::milliseconds(10));
	sw.stop();

	float elapsed = sw.elapsedSecondsF();
	REQUIRE(elapsed > 0.005f);
	REQUIRE(elapsed < 1.0f);
}
