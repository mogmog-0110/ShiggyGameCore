#pragma once

/// @file SharedData.hpp
/// @brief シーン間で共有するデータ

#include <Siv3D.hpp>

/// @brief シーン間共有データ
///
/// タイトル→ゲーム→リザルトの各シーンで共有される。
/// App<SharedData> のテンプレート引数として使用する。
struct SharedData
{
	int score = 0;           ///< 現在のスコア
	int highScore = 0;       ///< ハイスコア
	s3d::Font titleFont;     ///< タイトル用フォント
	s3d::Font uiFont;        ///< UI用フォント
	s3d::Font scoreFont;     ///< スコア表示用フォント
};
