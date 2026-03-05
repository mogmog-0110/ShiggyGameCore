#pragma once

/// @file TitleScene.hpp
/// @brief タイトルシーン

#include <string>

#include "sgc/animation/Tween.hpp"
#include "sgc/core/Hash.hpp"
#include "sgc/math/Easing.hpp"
#include "sgc/siv3d/SceneAdapter.hpp"
#include "sgc/ui/HudLayout.hpp"

#include "KeyCodes.hpp"
#include "SharedData.hpp"

using namespace sgc::literals;

/// @brief タイトル画面シーン
///
/// タイトルとハイスコアを表示し、Enterキーでゲームシーンに遷移する。
/// タイトルテキストはアルファパルスアニメーション付き。
class TitleScene : public sgc::siv3d::AppScene<SharedData>
{
public:
	using AppScene::AppScene;

	/// @brief シーン開始時の初期化
	void onEnter() override
	{
		getData().score = 0;

		// HUDレイアウト設定
		m_hud.clear();
		m_hud.add("title"_hash,     {sgc::ui::Anchor::TopCenter, {0.0f, 200.0f}});
		m_hud.add("prompt"_hash,    {sgc::ui::Anchor::TopCenter, {0.0f, 350.0f}});
		m_hud.add("highScore"_hash, {sgc::ui::Anchor::TopCenter, {0.0f, 420.0f}});
		m_hud.recalculate(sgc::ui::screenRect(800.0f, 600.0f));

		// タイトルアルファパルス（0.6 → 1.0 を往復）
		m_titleAlpha = sgc::Tweenf{};
		m_titleAlpha.from(0.6f).to(1.0f).during(1.5f)
			.withEasing(sgc::easing::inOutSine<float>)
			.setYoyo(true).setLoopCount(-1);

		m_titleAlphaValue = 0.6f;
	}

	/// @brief 更新処理（フレームワーク非依存）
	void update(float dt) override
	{
		m_titleAlphaValue = m_titleAlpha.step(dt);

		if (getData().inputProvider->isKeyJustPressed(KeyCode::ENTER))
		{
			getSceneManager().changeScene("game"_hash, 0.5f);
		}
	}

	/// @brief 描画処理
	void draw() const override
	{
		getData().renderer->clearBackground(sgc::Colorf{0.05f, 0.05f, 0.15f, 1.0f});

		const auto& data = getData();

		data.textRenderer->drawTextCentered(
			"SGC SHOOTING", 60.0f,
			m_hud.position("title"_hash),
			sgc::Colorf{0.2f, 0.8f, 1.0f, m_titleAlphaValue});

		data.textRenderer->drawTextCentered(
			"Press Enter to Start", 30.0f,
			m_hud.position("prompt"_hash),
			sgc::Colorf::white());

		if (data.highScore > 0)
		{
			data.textRenderer->drawTextCentered(
				"High Score: " + std::to_string(data.highScore), 24.0f,
				m_hud.position("highScore"_hash),
				sgc::Colorf{1.0f, 0.8f, 0.2f, 1.0f});
		}
	}

private:
	sgc::ui::HudLayout m_hud;         ///< HUDレイアウト
	sgc::Tweenf m_titleAlpha;         ///< タイトルアルファパルス
	float m_titleAlphaValue{1.0f};    ///< 現在のアルファ値（描画用キャッシュ）
};
