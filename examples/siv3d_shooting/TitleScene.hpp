#pragma once

/// @file TitleScene.hpp
/// @brief タイトルシーン

#include <Siv3D.hpp>

#include "sgc/siv3d/DrawAdapter.hpp"
#include "sgc/siv3d/SceneAdapter.hpp"

#include "SharedData.hpp"

/// @brief タイトル画面シーン
///
/// タイトルとハイスコアを表示し、Enterキーでゲームシーンに遷移する。
class TitleScene : public sgc::siv3d::AppScene<SharedData>
{
public:
	using AppScene::AppScene;

	/// @brief シーン開始時の初期化
	void onEnter() override
	{
		auto& data = getData();
		if (!data.titleFont)
		{
			data.titleFont = s3d::Font{60, s3d::Typeface::Heavy};
		}
		if (!data.uiFont)
		{
			data.uiFont = s3d::Font{30, s3d::Typeface::Medium};
		}
		if (!data.scoreFont)
		{
			data.scoreFont = s3d::Font{24, s3d::Typeface::Regular};
		}
		data.score = 0;
	}

	/// @brief 更新処理（実装はMain.cppで完全型が揃った後に定義）
	void update(float dt) override;

	/// @brief 描画処理
	void draw() const override
	{
		sgc::siv3d::clearBackground(sgc::Colorf{0.05f, 0.05f, 0.15f, 1.0f});

		const auto& data = getData();
		const auto center = sgc::Vec2f{400.0f, 200.0f};

		sgc::siv3d::drawTextCentered(
			data.titleFont, U"SGC SHOOTING", center,
			sgc::Colorf{0.2f, 0.8f, 1.0f, 1.0f});

		sgc::siv3d::drawTextCentered(
			data.uiFont, U"Press Enter to Start",
			sgc::Vec2f{400.0f, 350.0f},
			sgc::Colorf::white());

		if (data.highScore > 0)
		{
			sgc::siv3d::drawTextCentered(
				data.scoreFont,
				s3d::Format(U"High Score: ", data.highScore),
				sgc::Vec2f{400.0f, 420.0f},
				sgc::Colorf{1.0f, 0.8f, 0.2f, 1.0f});
		}
	}
};
