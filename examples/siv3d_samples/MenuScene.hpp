#pragma once

/// @file MenuScene.hpp
/// @brief SGCサンプルギャラリーのメニューシーン
///
/// 3x4グリッドのボタンで各サンプルシーンへ遷移する。
/// evaluateButtonによるホバー・プレス状態の視覚フィードバック付き。

#include <array>
#include <cstdint>
#include <string>
#include <string_view>

#include "sgc/core/Hash.hpp"
#include "sgc/math/Rect.hpp"
#include "sgc/scene/App.hpp"
#include "sgc/ui/Button.hpp"

#include "SharedData.hpp"

using namespace sgc::literals;

/// @brief メニューシーン
///
/// 12個のサンプルへのナビゲーションボタンを4x3グリッドで表示する。
class MenuScene : public sgc::AppScene<SharedData>
{
public:
	using AppScene::AppScene;

	/// @brief 更新処理
	void update(float /*dt*/) override
	{
		const auto* input = getData().inputProvider;
		const auto mousePos = input->mousePosition();
		const bool mouseDown = input->isMouseButtonDown(sgc::IInputProvider::MOUSE_LEFT);
		const bool mousePressed = input->isMouseButtonPressed(sgc::IInputProvider::MOUSE_LEFT);

		for (std::size_t i = 0; i < BUTTON_COUNT; ++i)
		{
			const auto bounds = calcButtonRect(i);
			const auto result = sgc::ui::evaluateButton(bounds, mousePos, mouseDown, mousePressed);
			m_buttonStates[i] = result.state;

			if (result.clicked)
			{
				getSceneManager().changeScene(SCENE_IDS[i], 0.3f);
			}
		}
	}

	/// @brief 描画処理
	void draw() const override
	{
		auto* renderer = getData().renderer;
		auto* text = getData().textRenderer;
		const float sw = getData().screenWidth;
		const float sh = getData().screenHeight;

		// 背景
		renderer->clearBackground(sgc::Colorf{0.08f, 0.08f, 0.12f, 1.0f});

		// タイトル
		text->drawTextCentered("SGC Sample Gallery", 48.0f,
			sgc::Vec2f{sw * 0.5f, 50.0f}, sgc::Colorf{0.9f, 0.85f, 1.0f, 1.0f});

		// ボタン描画
		for (std::size_t i = 0; i < BUTTON_COUNT; ++i)
		{
			drawButton(i);
		}

		// フッター
		text->drawTextCentered(
			"Click a sample to view | ESC to return from samples", 16.0f,
			sgc::Vec2f{sw * 0.5f, sh - 30.0f},
			sgc::Colorf{0.5f, 0.5f, 0.6f, 1.0f});
	}

private:
	static constexpr std::size_t BUTTON_COUNT = 12;  ///< サンプル数
	static constexpr int COLUMNS = 4;                ///< グリッド列数
	static constexpr float BUTTON_W = 180.0f;        ///< ボタン幅
	static constexpr float BUTTON_H = 80.0f;         ///< ボタン高さ
	static constexpr float GAP_X = 20.0f;            ///< 水平間隔
	static constexpr float GAP_Y = 20.0f;            ///< 垂直間隔
	static constexpr float GRID_TOP = 120.0f;        ///< グリッド開始Y

	/// @brief ボタンラベル
	static constexpr std::array<std::string_view, BUTTON_COUNT> LABELS =
	{
		"Tween Anim", "Particles", "Physics", "Raycast",
		"ECS", "UI Widgets", "Quadtree", "State Machine",
		"Object Pool", "Behavior Tree", "Math Visual", "Scene Fade"
	};

	/// @brief シーンID
	inline static const std::array<std::uint64_t, BUTTON_COUNT> SCENE_IDS =
	{
		"tween"_hash,    "particle"_hash,
		"physics"_hash,  "raycast"_hash,
		"ecs"_hash,      "ui"_hash,
		"quadtree"_hash, "state"_hash,
		"pool"_hash,     "ai"_hash,
		"math"_hash,     "fade"_hash
	};

	/// @brief 各ボタンの視覚状態
	std::array<sgc::ui::WidgetState, BUTTON_COUNT> m_buttonStates{};

	/// @brief ボタンのアクセントカラー（色相を均等分配）
	static sgc::Colorf buttonAccentColor(std::size_t index)
	{
		const float hue = static_cast<float>(index)
			* (360.0f / static_cast<float>(BUTTON_COUNT));
		return sgc::Colorf::fromHSV(hue, 0.6f, 0.8f);
	}

	/// @brief ボタン矩形を計算する
	[[nodiscard]] sgc::Rectf calcButtonRect(std::size_t index) const
	{
		const float sw = getData().screenWidth;
		const int col = static_cast<int>(index) % COLUMNS;
		const int row = static_cast<int>(index) / COLUMNS;

		const float totalW = COLUMNS * BUTTON_W + (COLUMNS - 1) * GAP_X;
		const float startX = (sw - totalW) * 0.5f;

		const float x = startX + static_cast<float>(col) * (BUTTON_W + GAP_X);
		const float y = GRID_TOP + static_cast<float>(row) * (BUTTON_H + GAP_Y);

		return sgc::Rectf{x, y, BUTTON_W, BUTTON_H};
	}

	/// @brief ボタンを1つ描画する
	void drawButton(std::size_t index) const
	{
		auto* renderer = getData().renderer;
		auto* text = getData().textRenderer;
		const auto bounds = calcButtonRect(index);
		const auto state = m_buttonStates[index];
		const auto accent = buttonAccentColor(index);

		// 状態に応じた色
		sgc::Colorf bgColor{0.15f, 0.15f, 0.2f, 1.0f};
		sgc::Colorf borderColor = accent.withAlpha(0.4f);
		sgc::Colorf labelColor{0.8f, 0.8f, 0.85f, 1.0f};

		if (state == sgc::ui::WidgetState::Hovered)
		{
			bgColor = accent.withAlpha(0.25f);
			borderColor = accent.withAlpha(0.8f);
			labelColor = sgc::Colorf::white();
		}
		else if (state == sgc::ui::WidgetState::Pressed)
		{
			bgColor = accent.withAlpha(0.45f);
			borderColor = accent;
			labelColor = sgc::Colorf::white();
		}

		// 背景矩形
		const auto aabb = bounds.toAABB2();
		renderer->drawRect(aabb, bgColor);
		renderer->drawRectFrame(aabb, 2.0f, borderColor);

		// ホバー・プレス時に上部アクセントライン
		if (state == sgc::ui::WidgetState::Hovered ||
			state == sgc::ui::WidgetState::Pressed)
		{
			const sgc::AABB2f topLine{
				{bounds.x(), bounds.y()},
				{bounds.x() + bounds.width(), bounds.y() + 3.0f}
			};
			renderer->drawRect(topLine, accent);
		}

		// ラベル
		const auto center = bounds.center();
		text->drawTextCentered(
			std::string{LABELS[index]}, 18.0f,
			center, labelColor);
	}
};
