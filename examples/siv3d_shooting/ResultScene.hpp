#pragma once

/// @file ResultScene.hpp
/// @brief リザルトシーン

#include <Siv3D.hpp>

#include "sgc/siv3d/DrawAdapter.hpp"
#include "sgc/siv3d/SceneAdapter.hpp"

#include "SharedData.hpp"

/// @brief リザルト画面シーン
///
/// 最終スコアとハイスコアを表示し、Enterキーでタイトルに戻る。
class ResultScene : public sgc::siv3d::AppScene<SharedData>
{
public:
	using AppScene::AppScene;

	/// @brief 更新処理（実装はMain.cppで完全型が揃った後に定義）
	void update(float dt) override;

	/// @brief 描画処理
	void draw() const override
	{
		sgc::siv3d::clearBackground(sgc::Colorf{0.08f, 0.02f, 0.02f, 1.0f});

		const auto& data = getData();

		sgc::siv3d::drawTextCentered(
			data.titleFont, U"GAME OVER",
			sgc::Vec2f{400.0f, 150.0f},
			sgc::Colorf{1.0f, 0.3f, 0.3f, 1.0f});

		sgc::siv3d::drawTextCentered(
			data.uiFont,
			s3d::Format(U"Score: ", data.score),
			sgc::Vec2f{400.0f, 280.0f},
			sgc::Colorf::white());

		// ハイスコア更新表示
		const bool isNewRecord = (data.score >= data.highScore && data.score > 0);
		if (isNewRecord)
		{
			sgc::siv3d::drawTextCentered(
				data.scoreFont, U"NEW RECORD!",
				sgc::Vec2f{400.0f, 330.0f},
				sgc::Colorf{1.0f, 0.8f, 0.2f, 1.0f});
		}

		sgc::siv3d::drawTextCentered(
			data.scoreFont,
			s3d::Format(U"High Score: ", data.highScore),
			sgc::Vec2f{400.0f, 370.0f},
			sgc::Colorf{0.8f, 0.8f, 0.8f, 1.0f});

		sgc::siv3d::drawTextCentered(
			data.uiFont, U"Press Enter to Return",
			sgc::Vec2f{400.0f, 460.0f},
			sgc::Colorf::white());
	}
};
