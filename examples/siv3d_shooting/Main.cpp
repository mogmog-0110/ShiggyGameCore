#include <Siv3D.hpp>

#include "sgc/siv3d/Siv3D.hpp"

#include "SharedData.hpp"
#include "TitleScene.hpp"
#include "GameScene.hpp"
#include "ResultScene.hpp"

// ── シーンのupdate()定義 ──────────────────────────────
// 循環依存を避けるため、全シーン型が揃った後にここで定義する。

void TitleScene::update(float /*dt*/)
{
	if (s3d::KeyEnter.down())
	{
		getSceneManager().changeScene<GameScene>(0.5f);
	}
}

void GameScene::update(float dt)
{
	// 入力更新
	m_inputAdapter.update(m_actionMap);

	// ECSシステム実行（Render フェーズ含む）
	m_scheduler.update(m_world, dt);

	// プレイヤーの画面端クランプ
	clampPlayerPosition();

	// ゲームオーバー判定（再入防止付き）
	if (m_gameOver)
	{
		m_gameOver = false;
		auto& data = getData();
		if (data.score > data.highScore)
		{
			data.highScore = data.score;
		}
		getSceneManager().changeScene<ResultScene>(0.5f);
	}
}

void ResultScene::update(float /*dt*/)
{
	if (s3d::KeyEnter.down())
	{
		getSceneManager().changeScene<TitleScene>(0.5f);
	}
}

// ── メインエントリーポイント ──────────────────────────

/// @brief メインエントリーポイント
///
/// sgcのアダプター層を使い、Siv3Dゲームを構成する。
/// SceneManager で TitleScene → GameScene → ResultScene を遷移する。
void Main()
{
	// ウィンドウ設定
	s3d::Window::Resize(800, 600);
	s3d::Window::SetTitle(U"SGC Shooting - Sample Game");

	// アプリケーション起動
	sgc::siv3d::App<SharedData> app;
	app.addScene<TitleScene>();
	app.run();
}
