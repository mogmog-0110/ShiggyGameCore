#pragma once

/// @file Timer.hpp
/// @brief ストップウォッチとデルタタイム計測
///
/// ゲームループでのフレーム時間計測に使用する。

#include <chrono>

namespace sgc
{

/// @brief 高精度ストップウォッチ
///
/// 経過時間の計測、一時停止・再開が可能。
///
/// @code
/// sgc::Stopwatch sw;
/// sw.start();
/// // ... 処理 ...
/// float elapsed = sw.elapsedSeconds();
/// @endcode
class Stopwatch
{
public:
	using Clock = std::chrono::high_resolution_clock;
	using TimePoint = Clock::time_point;

	/// @brief ストップウォッチを開始する
	void start() noexcept
	{
		m_startTime = Clock::now();
		m_running = true;
	}

	/// @brief ストップウォッチを停止する
	void stop() noexcept
	{
		if (m_running)
		{
			m_elapsed += Clock::now() - m_startTime;
			m_running = false;
		}
	}

	/// @brief ストップウォッチをリセットする
	void reset() noexcept
	{
		m_elapsed = Clock::duration::zero();
		m_running = false;
	}

	/// @brief リセットして再スタートする
	void restart() noexcept
	{
		reset();
		start();
	}

	/// @brief 経過時間をミリ秒で返す
	[[nodiscard]] double elapsedMilliseconds() const noexcept
	{
		return std::chrono::duration<double, std::milli>(totalElapsed()).count();
	}

	/// @brief 経過時間を秒で返す
	[[nodiscard]] double elapsedSeconds() const noexcept
	{
		return std::chrono::duration<double>(totalElapsed()).count();
	}

	/// @brief 経過時間をfloat秒で返す（ゲームループ向け）
	[[nodiscard]] float elapsedSecondsF() const noexcept
	{
		return std::chrono::duration<float>(totalElapsed()).count();
	}

	/// @brief 計測中かどうかを返す
	[[nodiscard]] bool isRunning() const noexcept { return m_running; }

private:
	[[nodiscard]] Clock::duration totalElapsed() const noexcept
	{
		if (m_running)
		{
			return m_elapsed + (Clock::now() - m_startTime);
		}
		return m_elapsed;
	}

	TimePoint m_startTime{};
	Clock::duration m_elapsed = Clock::duration::zero();
	bool m_running = false;
};

/// @brief フレーム間デルタタイム計測器
///
/// 毎フレーム tick() を呼ぶとデルタタイムが計算される。
///
/// @code
/// sgc::DeltaClock clock;
/// while (running) {
///     float dt = clock.tick();
///     update(dt);
///     render();
/// }
/// @endcode
class DeltaClock
{
public:
	using Clock = std::chrono::high_resolution_clock;

	/// @brief フレームを進め、前フレームからの経過秒数を返す
	/// @return デルタタイム（秒）
	[[nodiscard]] float tick() noexcept
	{
		const auto now = Clock::now();
		const float dt = std::chrono::duration<float>(now - m_lastTime).count();
		m_lastTime = now;
		return dt;
	}

	/// @brief 計測をリセットする（シーン切り替え時等）
	void reset() noexcept
	{
		m_lastTime = Clock::now();
	}

private:
	Clock::time_point m_lastTime = Clock::now();
};

} // namespace sgc
