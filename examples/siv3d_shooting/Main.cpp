#include <Siv3D.hpp>

#include "sgc/core/Hash.hpp"
#include "sgc/siv3d/Siv3D.hpp"

#include "KeyCodes.hpp"
#include "SharedData.hpp"
#include "TitleScene.hpp"
#include "GameScene.hpp"
#include "ResultScene.hpp"

using namespace sgc::literals;

// ── メインエントリーポイント ──────────────────────────

/// @brief メインエントリーポイント
///
/// sgcのアダプター層を使い、Siv3Dゲームを構成する。
/// SceneManager で TitleScene → GameScene → ResultScene を遷移する。
/// ID指定でシーンを登録し、循環依存なくシーン遷移を実現する。
void Main()
{
	// ウィンドウ設定
	s3d::Window::Resize(800, 600);
	s3d::Window::SetTitle(U"SGC Shooting - Sample Game");

	// レンダラー初期化
	sgc::siv3d::Siv3DRenderer renderer;

	// テキストレンダラー初期化
	sgc::siv3d::Siv3DTextRenderer textRenderer;
	textRenderer.registerFont(60, s3d::Typeface::Heavy);
	textRenderer.registerFont(30, s3d::Typeface::Medium);
	textRenderer.registerFont(24, s3d::Typeface::Regular);

	// 入力プロバイダー初期化
	sgc::siv3d::Siv3DInputProvider inputProvider;
	inputProvider.addKey(s3d::KeyEnter, KeyCode::ENTER);
	inputProvider.addKey(s3d::KeySpace, KeyCode::SPACE);
	inputProvider.addKey(s3d::KeyLeft,  KeyCode::LEFT);
	inputProvider.addKey(s3d::KeyRight, KeyCode::RIGHT);
	inputProvider.addKey(s3d::KeyUp,    KeyCode::UP);
	inputProvider.addKey(s3d::KeyDown,  KeyCode::DOWN);
	inputProvider.addKey(s3d::KeyA,     KeyCode::A);
	inputProvider.addKey(s3d::KeyD,     KeyCode::D);
	inputProvider.addKey(s3d::KeyW,     KeyCode::W);
	inputProvider.addKey(s3d::KeyS,     KeyCode::S);
	inputProvider.addKey(s3d::KeyZ,     KeyCode::Z);

	// アプリケーション起動
	sgc::siv3d::App<SharedData> app;

	// 共有データにアダプターを設定
	app.getData().renderer = &renderer;
	app.getData().textRenderer = &textRenderer;
	app.getData().inputProvider = &inputProvider;

	// フェードオーバーレイ用レンダラー設定
	app.setRenderer(&renderer);

	// シーン登録（ID指定）
	app.registerScene<TitleScene>("title"_hash);
	app.registerScene<GameScene>("game"_hash);
	app.registerScene<ResultScene>("result"_hash);

	// 初期シーン設定
	app.setInitialScene("title"_hash);

	app.run();
}
