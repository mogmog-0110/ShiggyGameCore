/// @file Main.cpp
/// @brief DxLib版シューティングゲーム メインエントリーポイント
///
/// siv3d_shooting/ と同じシーンファイルをDxLibアダプターで動かす。
/// シーンコード（TitleScene/GameScene/ResultScene）はフレームワーク非依存。
///
/// @note DxLib環境でのみビルド可能。CIではビルドされない。

#include <DxLib.h>

#include "sgc/core/Hash.hpp"
#include "sgc/dxlib/DxLib.hpp"

#include "../siv3d_shooting/KeyCodes.hpp"
#include "../siv3d_shooting/SharedData.hpp"
#include "../siv3d_shooting/TitleScene.hpp"
#include "../siv3d_shooting/GameScene.hpp"
#include "../siv3d_shooting/ResultScene.hpp"

using namespace sgc::literals;

// ── メインエントリーポイント ──────────────────────────

/// @brief WinMainエントリーポイント
///
/// DxLibの初期化 → アダプター層のセットアップ → sgc::dxlib::App でゲーム実行。
/// シーンファイルはSiv3D版と完全に共通。
int WINAPI WinMain(HINSTANCE /*hInstance*/, HINSTANCE /*hPrevInstance*/,
	LPSTR /*lpCmdLine*/, int /*nCmdShow*/)
{
	// DxLib初期化
	SetGraphMode(800, 600, 32);
	ChangeWindowMode(TRUE);

	if (DxLib_Init() == -1)
	{
		return -1;
	}

	SetDrawScreen(DX_SCREEN_BACK);

	// レンダラー初期化
	sgc::dxlib::DxLibRenderer renderer;

	// テキストレンダラー初期化
	sgc::dxlib::DxLibTextRenderer textRenderer;
	textRenderer.registerFont(60, CreateFontToHandle(nullptr, 60, 3, DX_FONTTYPE_ANTIALIASING_EDGE_4X4));
	textRenderer.registerFont(30, CreateFontToHandle(nullptr, 30, 3, DX_FONTTYPE_ANTIALIASING_EDGE_4X4));
	textRenderer.registerFont(24, CreateFontToHandle(nullptr, 24, 3, DX_FONTTYPE_ANTIALIASING_EDGE_4X4));

	// 入力プロバイダー初期化
	sgc::dxlib::DxLibInputProvider inputProvider;
	inputProvider.addKey(KEY_INPUT_RETURN, KeyCode::ENTER);
	inputProvider.addKey(KEY_INPUT_SPACE,  KeyCode::SPACE);
	inputProvider.addKey(KEY_INPUT_LEFT,   KeyCode::LEFT);
	inputProvider.addKey(KEY_INPUT_RIGHT,  KeyCode::RIGHT);
	inputProvider.addKey(KEY_INPUT_UP,     KeyCode::UP);
	inputProvider.addKey(KEY_INPUT_DOWN,   KeyCode::DOWN);
	inputProvider.addKey(KEY_INPUT_A,      KeyCode::A);
	inputProvider.addKey(KEY_INPUT_D,      KeyCode::D);
	inputProvider.addKey(KEY_INPUT_W,      KeyCode::W);
	inputProvider.addKey(KEY_INPUT_S,      KeyCode::S);
	inputProvider.addKey(KEY_INPUT_Z,      KeyCode::Z);

	// アプリケーション起動
	sgc::dxlib::App<SharedData> app;

	// 共有データにアダプターを設定
	app.getData().renderer = &renderer;
	app.getData().textRenderer = &textRenderer;
	app.getData().inputProvider = &inputProvider;

	// フェードオーバーレイ用レンダラー設定
	app.setRenderer(&renderer);

	// シーン登録（ID指定）― siv3d版と同一
	app.registerScene<TitleScene>("title"_hash);
	app.registerScene<GameScene>("game"_hash);
	app.registerScene<ResultScene>("result"_hash);

	// 初期シーン設定
	app.setInitialScene("title"_hash);

	app.run();

	DxLib_End();
	return 0;
}
