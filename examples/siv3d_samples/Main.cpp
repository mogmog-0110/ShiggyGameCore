/// @file Main.cpp
/// @brief SGCサンプルギャラリー Siv3Dエントリーポイント
///
/// 全サンプルシーンをメニューから選択して実行できるビジュアルギャラリー。

#include <Siv3D.hpp>
#include "sgc/core/Hash.hpp"
#include "sgc/siv3d/Siv3D.hpp"

// シーン定義
#include "SharedData.hpp"
#include "KeyCodes.hpp"
#include "MenuScene.hpp"
#include "SampleTween.hpp"
#include "SampleParticle.hpp"
#include "SamplePhysics.hpp"
#include "SampleRaycast.hpp"
#include "SampleECS.hpp"
#include "SampleUI.hpp"
#include "SampleQuadtree.hpp"
#include "SampleStateMachine.hpp"
#include "SampleObjectPool.hpp"
#include "SampleBehaviorTree.hpp"
#include "SampleMath.hpp"
#include "SampleFade.hpp"

using namespace sgc::literals;

/// @brief SGCサンプルギャラリーのメインエントリーポイント
///
/// sgcのアダプター層を使い、各サンプルシーンを登録する。
/// MenuSceneをハブとして各デモに遷移する。
void Main()
{
	s3d::Window::Resize(800, 600);
	s3d::Window::SetTitle(U"SGC Sample Gallery");
	s3d::Scene::SetBackground(s3d::ColorF{0.1, 0.1, 0.12});

	// レンダラー初期化
	sgc::siv3d::Siv3DRenderer renderer;

	// テキストレンダラー初期化
	sgc::siv3d::Siv3DTextRenderer textRenderer;
	textRenderer.registerFont(48, s3d::Typeface::Heavy);
	textRenderer.registerFont(36, s3d::Typeface::Medium);
	textRenderer.registerFont(28, s3d::Typeface::Medium);
	textRenderer.registerFont(24, s3d::Typeface::Medium);
	textRenderer.registerFont(22, s3d::Typeface::Regular);
	textRenderer.registerFont(20, s3d::Typeface::Regular);
	textRenderer.registerFont(18, s3d::Typeface::Regular);
	textRenderer.registerFont(16, s3d::Typeface::Regular);
	textRenderer.registerFont(14, s3d::Typeface::Regular);
	textRenderer.registerFont(12, s3d::Typeface::Regular);
	textRenderer.registerFont(11, s3d::Typeface::Regular);
	textRenderer.registerFont(10, s3d::Typeface::Regular);
	textRenderer.registerFont(9, s3d::Typeface::Regular);

	// 入力プロバイダー初期化
	sgc::siv3d::Siv3DInputProvider inputProvider;
	inputProvider.addKey(s3d::KeyEscape, KeyCode::ESCAPE);
	inputProvider.addKey(s3d::KeyEnter, KeyCode::ENTER);
	inputProvider.addKey(s3d::KeySpace, KeyCode::SPACE);
	inputProvider.addKey(s3d::KeyLeft, KeyCode::LEFT);
	inputProvider.addKey(s3d::KeyRight, KeyCode::RIGHT);
	inputProvider.addKey(s3d::KeyUp, KeyCode::UP);
	inputProvider.addKey(s3d::KeyDown, KeyCode::DOWN);
	inputProvider.addKey(s3d::KeyBackspace, KeyCode::BACKSPACE);
	inputProvider.addKey(s3d::KeyW, KeyCode::W);
	inputProvider.addKey(s3d::KeyA, KeyCode::A);
	inputProvider.addKey(s3d::KeyS, KeyCode::S);
	inputProvider.addKey(s3d::KeyD, KeyCode::D);
	inputProvider.addKey(s3d::KeyR, KeyCode::R);
	inputProvider.addKey(s3d::Key1, KeyCode::NUM1);
	inputProvider.addKey(s3d::Key2, KeyCode::NUM2);
	inputProvider.addKey(s3d::Key3, KeyCode::NUM3);

	// アプリケーション起動
	sgc::siv3d::App<SharedData> app;
	app.getData().renderer = &renderer;
	app.getData().textRenderer = &textRenderer;
	app.getData().inputProvider = &inputProvider;
	app.getData().screenWidth = 800.0f;
	app.getData().screenHeight = 600.0f;
	app.setRenderer(&renderer);

	// シーン登録
	app.registerScene<MenuScene>("menu"_hash);
	app.registerScene<SampleTween>("tween"_hash);
	app.registerScene<SampleParticle>("particle"_hash);
	app.registerScene<SamplePhysics>("physics"_hash);
	app.registerScene<SampleRaycast>("raycast"_hash);
	app.registerScene<SampleECS>("ecs"_hash);
	app.registerScene<SampleUI>("ui"_hash);
	app.registerScene<SampleQuadtree>("quadtree"_hash);
	app.registerScene<SampleStateMachine>("state"_hash);
	app.registerScene<SampleObjectPool>("pool"_hash);
	app.registerScene<SampleBehaviorTree>("ai"_hash);
	app.registerScene<SampleMath>("math"_hash);
	app.registerScene<SampleFade>("fade"_hash);

	// 初期シーン設定
	app.setInitialScene("menu"_hash);
	app.run();
}
