/// @file scene_transition.cpp
/// @brief SceneManagerでシーン遷移を実演するサンプル

#include <iostream>
#include <string>

#include "sgc/scene/SceneManager.hpp"
#include "sgc/core/Hash.hpp"

using namespace sgc::literals;

/// @brief シーンID定数
namespace SceneIds
{
	constexpr sgc::SceneId Title  = "Title"_hash;
	constexpr sgc::SceneId Game   = "Game"_hash;
	constexpr sgc::SceneId Result = "Result"_hash;
}

/// @brief タイトルシーン
class TitleScene : public sgc::Scene
{
public:
	/// @param manager シーンマネージャへの参照（遷移用）
	explicit TitleScene(sgc::SceneManager& manager)
		: m_manager(manager) {}

	void onEnter() override
	{
		std::cout << "  [TitleScene] onEnter - Welcome to the game!" << std::endl;
	}

	void update(float dt) override
	{
		m_elapsed += dt;
		std::cout << "  [TitleScene] update (elapsed=" << m_elapsed << "s)" << std::endl;

		/// 1秒経過したらGameシーンへ遷移
		if (m_elapsed >= 1.0f)
		{
			std::cout << "  [TitleScene] -> Transitioning to Game..." << std::endl;
			m_manager.changeScene(SceneIds::Game, 0.5f);
		}
	}

	void onExit() override
	{
		std::cout << "  [TitleScene] onExit" << std::endl;
	}

private:
	sgc::SceneManager& m_manager;
	float m_elapsed{0.0f};
};

/// @brief ゲームプレイシーン
class GameScene : public sgc::Scene
{
public:
	explicit GameScene(sgc::SceneManager& manager)
		: m_manager(manager) {}

	void onEnter() override
	{
		std::cout << "  [GameScene] onEnter - Game started!" << std::endl;
	}

	void update(float dt) override
	{
		m_elapsed += dt;
		++m_frameCount;
		std::cout << "  [GameScene] update frame=" << m_frameCount
			<< " (elapsed=" << m_elapsed << "s)" << std::endl;

		/// 3フレーム経過したらResultシーンへ
		if (m_frameCount >= 3)
		{
			std::cout << "  [GameScene] -> Game over! Going to Result..." << std::endl;
			m_manager.changeScene(SceneIds::Result, 0.5f);
		}
	}

	void onExit() override
	{
		std::cout << "  [GameScene] onExit" << std::endl;
	}

private:
	sgc::SceneManager& m_manager;
	float m_elapsed{0.0f};
	int m_frameCount{0};
};

/// @brief リザルトシーン
class ResultScene : public sgc::Scene
{
public:
	explicit ResultScene(sgc::SceneManager& manager)
		: m_manager(manager) {}

	void onEnter() override
	{
		std::cout << "  [ResultScene] onEnter - Your score: 12345" << std::endl;
	}

	void update(float dt) override
	{
		m_elapsed += dt;
		std::cout << "  [ResultScene] update (elapsed=" << m_elapsed << "s)" << std::endl;

		/// 1秒経過したらTitleに戻る
		if (m_elapsed >= 1.0f)
		{
			std::cout << "  [ResultScene] -> Back to Title..." << std::endl;
			m_manager.changeScene(SceneIds::Title, 0.5f);
		}
	}

	void onExit() override
	{
		std::cout << "  [ResultScene] onExit" << std::endl;
	}

private:
	sgc::SceneManager& m_manager;
	float m_elapsed{0.0f};
};

int main()
{
	std::cout << "--- SceneManager Demo ---" << std::endl;
	std::cout << std::endl;

	sgc::SceneManager manager;

	// ── 1. シーンの登録（ファクトリ関数） ────────────────────
	std::cout << "=== Registering Scenes ===" << std::endl;
	manager.registerScene(SceneIds::Title, [&manager]() -> std::unique_ptr<sgc::Scene>
	{
		return std::make_unique<TitleScene>(manager);
	});
	manager.registerScene(SceneIds::Game, [&manager]() -> std::unique_ptr<sgc::Scene>
	{
		return std::make_unique<GameScene>(manager);
	});
	manager.registerScene(SceneIds::Result, [&manager]() -> std::unique_ptr<sgc::Scene>
	{
		return std::make_unique<ResultScene>(manager);
	});
	std::cout << "  Registered: Title, Game, Result" << std::endl;
	std::cout << std::endl;

	// ── 2. 初期シーンをpush ─────────────────────────────────
	std::cout << "=== Push Initial Scene (Title) ===" << std::endl;
	manager.pushScene(SceneIds::Title);
	std::cout << "  Stack depth: " << manager.depth() << std::endl;
	std::cout << std::endl;

	// ── 3. ゲームループをシミュレート ────────────────────────
	/// フェード遷移を含む15フレーム分のシミュレーション
	constexpr float dt = 0.5f;
	constexpr int maxFrames = 15;

	for (int frame = 0; frame < maxFrames; ++frame)
	{
		std::cout << "--- Frame " << (frame + 1) << " ---" << std::endl;

		/// フェード状態の表示
		switch (manager.fadeState())
		{
		case sgc::FadeState::FadingOut:
			std::cout << "  [Fade] OUT progress=" << manager.getFadeProgress() << std::endl;
			break;
		case sgc::FadeState::FadingIn:
			std::cout << "  [Fade] IN progress=" << manager.getFadeProgress() << std::endl;
			break;
		default:
			break;
		}

		manager.update(dt);
		std::cout << "  Stack depth: " << manager.depth() << std::endl;
		std::cout << std::endl;
	}

	std::cout << "=== Demo Complete ===" << std::endl;

	return 0;
}
