/// @file tween_animation.cpp
/// @brief Tween・TweenSequence・TweenTimelineの使い方を示すサンプル
///
/// sgcのトゥイーンアニメーションシステムを実演する。
/// - 単一Tween（イージング付き）
/// - TweenSequence（3つのTweenを逐次実行）
/// - TweenTimeline（2つのTweenを並列実行）
/// - ヨーヨー＆ループ
/// - onUpdate / onComplete コールバック

#include <iomanip>
#include <iostream>
#include <string>

#include "sgc/animation/Tween.hpp"
#include "sgc/animation/TweenTimeline.hpp"
#include "sgc/math/Easing.hpp"

/// @brief 進行度をASCIIバーで可視化する
/// @param label ラベル文字列
/// @param value 現在値
/// @param maxValue 最大値（バーの右端）
/// @param barWidth バーの幅（文字数）
void printBar(const std::string& label, float value, float maxValue, int barWidth = 30)
{
	const float ratio = (maxValue > 0.0f) ? (value / maxValue) : 0.0f;
	const int filled = static_cast<int>(ratio * barWidth);
	std::cout << std::setw(12) << label << " [";
	for (int i = 0; i < barWidth; ++i)
	{
		std::cout << ((i < filled) ? '#' : '.');
	}
	std::cout << "] " << std::fixed << std::setprecision(2) << value << "\n";
}

int main()
{
	std::cout << "=== sgc Tween Animation Demo ===\n\n";

	// ────────────────────────────────────────────────────
	// 1. 単一Tween + イージング
	// ────────────────────────────────────────────────────
	std::cout << "--- 1. Single Tween (outCubic easing, 0 -> 100 in 1.0s) ---\n";
	{
		sgc::Tweenf tween;
		tween.from(0.0f).to(100.0f).during(1.0f)
			.withEasing(sgc::easing::outCubic<float>);

		const float dt = 0.1f;
		int frame = 0;
		while (!tween.isFinished())
		{
			const float val = tween.step(dt);
			std::cout << "  t=" << std::fixed << std::setprecision(1)
					  << (frame + 1) * dt << "s  ";
			printBar("position", val, 100.0f, 25);
			++frame;
		}
	}

	// ────────────────────────────────────────────────────
	// 2. onUpdate / onComplete コールバック
	// ────────────────────────────────────────────────────
	std::cout << "\n--- 2. Callbacks (onUpdate + onComplete) ---\n";
	{
		int updateCount = 0;
		bool completed = false;

		sgc::Tweenf tween;
		tween.from(0.0f).to(50.0f).during(0.5f)
			.withEasing(sgc::easing::inOutQuad<float>)
			.onUpdate([&updateCount](float val)
			{
				++updateCount;
				std::cout << "  [onUpdate #" << updateCount << "] value = "
						  << std::fixed << std::setprecision(2) << val << "\n";
			})
			.onComplete([&completed]()
			{
				completed = true;
				std::cout << "  [onComplete] Animation finished!\n";
			});

		const float dt = 0.1f;
		while (!tween.isFinished())
		{
			tween.step(dt);
		}
		std::cout << "  Total onUpdate calls: " << updateCount
				  << ", completed: " << (completed ? "true" : "false") << "\n";
	}

	// ────────────────────────────────────────────────────
	// 3. ヨーヨー＆ループ
	// ────────────────────────────────────────────────────
	std::cout << "\n--- 3. Yoyo + Loop (0 -> 10, yoyo, 2 loops) ---\n";
	{
		sgc::Tweenf tween;
		tween.from(0.0f).to(10.0f).during(0.5f)
			.withEasing(sgc::easing::linear<float>)
			.setYoyo(true)
			.setLoopCount(2);

		const float dt = 0.1f;
		int step = 0;
		while (!tween.isFinished() && step < 30)
		{
			const float val = tween.step(dt);
			std::cout << "  step " << std::setw(2) << step
					  << ": value = " << std::fixed << std::setprecision(2) << val << "\n";
			++step;
		}
	}

	// ────────────────────────────────────────────────────
	// 4. TweenSequence（3つのTweenを逐次実行）
	// ────────────────────────────────────────────────────
	std::cout << "\n--- 4. TweenSequence (chain 3 tweens: 0->100->50->200) ---\n";
	{
		sgc::TweenSequence<float> seq;

		/// フェーズ1: 0 → 100（outCubicで加速開始）
		sgc::Tweenf phase1;
		phase1.from(0.0f).to(100.0f).during(0.5f)
			.withEasing(sgc::easing::outCubic<float>);

		/// フェーズ2: 100 → 50（inQuadで減速的に戻る）
		sgc::Tweenf phase2;
		phase2.from(100.0f).to(50.0f).during(0.3f)
			.withEasing(sgc::easing::inQuad<float>);

		/// フェーズ3: 50 → 200（outBounceでバウンド）
		sgc::Tweenf phase3;
		phase3.from(50.0f).to(200.0f).during(0.7f)
			.withEasing(sgc::easing::outBounce<float>);

		seq.add(std::move(phase1))
			.add(std::move(phase2))
			.add(std::move(phase3));

		const float dt = 0.1f;
		int step = 0;
		while (!seq.isFinished())
		{
			const float val = seq.step(dt);
			std::cout << "  step " << std::setw(2) << step << ": ";
			printBar("value", val, 200.0f, 25);
			++step;
		}
	}

	// ────────────────────────────────────────────────────
	// 5. TweenTimeline（2つのTweenを並列実行）
	// ────────────────────────────────────────────────────
	std::cout << "\n--- 5. TweenTimeline (parallel: X position + opacity) ---\n";
	{
		/// Xポジション: 0 → 400 を 1.0秒で移動
		float posX = 0.0f;
		sgc::Tweenf tweenX;
		tweenX.from(0.0f).to(400.0f).during(1.0f)
			.withEasing(sgc::easing::outQuart<float>)
			.onUpdate([&posX](float val) { posX = val; });

		/// 透明度: 0 → 1 を 0.5秒でフェードイン（先に完了する）
		float alpha = 0.0f;
		sgc::Tweenf tweenAlpha;
		tweenAlpha.from(0.0f).to(1.0f).during(0.5f)
			.withEasing(sgc::easing::inOutSine<float>)
			.onUpdate([&alpha](float val) { alpha = val; });

		sgc::TweenTimelinef timeline;
		timeline.add(std::move(tweenX));
		timeline.add(std::move(tweenAlpha));
		timeline.onComplete([]()
		{
			std::cout << "  [Timeline Complete] All tweens finished!\n";
		});

		const float dt = 0.1f;
		int step = 0;
		std::cout << std::fixed << std::setprecision(2);
		while (!timeline.isFinished())
		{
			timeline.step(dt);
			std::cout << "  step " << std::setw(2) << step
					  << ": posX = " << std::setw(7) << posX
					  << "  alpha = " << alpha << "\n";
			++step;
		}
	}

	// ────────────────────────────────────────────────────
	// 6. イージング関数比較
	// ────────────────────────────────────────────────────
	std::cout << "\n--- 6. Easing Function Comparison (t=0.5) ---\n";
	{
		const float t = 0.5f;
		std::cout << std::fixed << std::setprecision(4);
		std::cout << "  linear:      " << sgc::easing::linear(t) << "\n";
		std::cout << "  inQuad:      " << sgc::easing::inQuad(t) << "\n";
		std::cout << "  outQuad:     " << sgc::easing::outQuad(t) << "\n";
		std::cout << "  inOutQuad:   " << sgc::easing::inOutQuad(t) << "\n";
		std::cout << "  outCubic:    " << sgc::easing::outCubic(t) << "\n";
		std::cout << "  outBounce:   " << sgc::easing::outBounce(t) << "\n";
		std::cout << "  outElastic:  " << sgc::easing::outElastic(t) << "\n";
		std::cout << "  inBack:      " << sgc::easing::inBack(t) << "\n";
		std::cout << "  outBack:     " << sgc::easing::outBack(t) << "\n";
	}

	std::cout << "\n=== Demo Complete ===\n";
	return 0;
}
