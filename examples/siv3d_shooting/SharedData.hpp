#pragma once

/// @file SharedData.hpp
/// @brief シーン間で共有するデータ

#include "sgc/graphics/IRenderer.hpp"
#include "sgc/graphics/ITextRenderer.hpp"
#include "sgc/input/IInputProvider.hpp"

/// @brief シーン間共有データ
///
/// タイトル→ゲーム→リザルトの各シーンで共有される。
/// App<SharedData> のテンプレート引数として使用する。
struct SharedData
{
	int score = 0;                              ///< 現在のスコア
	int highScore = 0;                          ///< ハイスコア
	sgc::IRenderer* renderer = nullptr;         ///< 描画インターフェース
	sgc::ITextRenderer* textRenderer = nullptr; ///< テキスト描画インターフェース
	sgc::IInputProvider* inputProvider = nullptr; ///< 入力プロバイダー
};
