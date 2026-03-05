#pragma once

/// @file ResultScene.hpp
/// @brief リザルトシーン

#include <string>

#include "sgc/animation/Tween.hpp"
#include "sgc/core/Hash.hpp"
#include "sgc/math/Easing.hpp"
#include "sgc/scene/App.hpp"
#include "sgc/ui/HudLayout.hpp"

#include "KeyCodes.hpp"
#include "SharedData.hpp"

using namespace sgc::literals;

/// @brief リザルト画面シーン
///
/// 最終スコアとハイスコアを表示し、Enterキーでタイトルに戻る。
/// GAME OVERテキストは上からスライドインアニメーション付き。
class ResultScene : public sgc::AppScene<SharedData>
{
public:
	using AppScene::AppScene;

	/// @brief シーン開始時の初期化
	void onEnter() override
	{
		m_hud.clear();
		m_hud.add("gameOver"_hash,  {sgc::ui::Anchor::TopCenter, {0.0f, 150.0f}});
		m_hud.add("score"_hash,     {sgc::ui::Anchor::TopCenter, {0.0f, 280.0f}});
		m_hud.add("newRecord"_hash, {sgc::ui::Anchor::TopCenter, {0.0f, 330.0f}});
		m_hud.add("highScore"_hash, {sgc::ui::Anchor::TopCenter, {0.0f, 370.0f}});
		m_hud.add("prompt"_hash,    {sgc::ui::Anchor::TopCenter, {0.0f, 460.0f}});
		m_hud.recalculate(sgc::ui::screenRect(800.0f, 600.0f));

		// GAME OVERスライドイン（上から降下）
		m_slideY = sgc::Tweenf{};
		m_slideY.from(-60.0f).to(0.0f).during(0.8f)
			.withEasing(sgc::easing::outBack<float>);

		m_slideYValue = -60.0f;
	}

	/// @brief 更新処理（フレームワーク非依存）
	void update(float dt) override
	{
		m_slideYValue = m_slideY.step(dt);

		if (getData().inputProvider->isKeyJustPressed(KeyCode::ENTER))
		{
			getSceneManager().changeScene("title"_hash, 0.5f);
		}
	}

	/// @brief 描画処理
	void draw() const override
	{
		getData().renderer->clearBackground(sgc::Colorf{0.08f, 0.02f, 0.02f, 1.0f});

		const auto& data = getData();

		// GAME OVERテキスト（スライドインアニメーション付き）
		const auto gameOverPos = m_hud.position("gameOver"_hash);
		data.textRenderer->drawTextCentered(
			"GAME OVER", 60.0f,
			sgc::Vec2f{gameOverPos.x, gameOverPos.y + m_slideYValue},
			sgc::Colorf{1.0f, 0.3f, 0.3f, 1.0f});

		data.textRenderer->drawTextCentered(
			"Score: " + std::to_string(data.score), 30.0f,
			m_hud.position("score"_hash),
			sgc::Colorf::white());

		// ハイスコア更新表示
		const bool isNewRecord = (data.score >= data.highScore && data.score > 0);
		if (isNewRecord)
		{
			data.textRenderer->drawTextCentered(
				"NEW RECORD!", 24.0f,
				m_hud.position("newRecord"_hash),
				sgc::Colorf{1.0f, 0.8f, 0.2f, 1.0f});
		}

		data.textRenderer->drawTextCentered(
			"High Score: " + std::to_string(data.highScore), 24.0f,
			m_hud.position("highScore"_hash),
			sgc::Colorf{0.8f, 0.8f, 0.8f, 1.0f});

		data.textRenderer->drawTextCentered(
			"Press Enter to Return", 30.0f,
			m_hud.position("prompt"_hash),
			sgc::Colorf::white());
	}

private:
	sgc::ui::HudLayout m_hud;       ///< HUDレイアウト
	sgc::Tweenf m_slideY;           ///< GAME OVERスライドイン
	float m_slideYValue{0.0f};      ///< 現在のY オフセット（描画用キャッシュ）
};
