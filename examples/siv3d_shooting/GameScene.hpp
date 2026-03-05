#pragma once

/// @file GameScene.hpp
/// @brief ゲームプレイシーン

#include <string>
#include <vector>

#include "sgc/animation/Tween.hpp"
#include "sgc/core/Hash.hpp"
#include "sgc/ecs/System.hpp"
#include "sgc/ecs/World.hpp"
#include "sgc/effects/ParticleSystem.hpp"
#include "sgc/input/ActionMap.hpp"
#include "sgc/patterns/EventDispatcher.hpp"
#include "sgc/scene/App.hpp"
#include "sgc/ui/HudLayout.hpp"

#include "BulletSystem.hpp"
#include "CleanupSystem.hpp"
#include "CollisionSystem.hpp"
#include "Components.hpp"
#include "EnemySpawnSystem.hpp"
#include "Events.hpp"
#include "GameEffects.hpp"
#include "InputSystem.hpp"
#include "KeyCodes.hpp"
#include "MovementSystem.hpp"
#include "RenderSystem.hpp"
#include "SharedData.hpp"

using namespace sgc::literals;

/// @brief ゲームプレイシーン
///
/// ECSワールドを初期化し、各システムを登録してゲームループを実行する。
/// sgcのECS、EventDispatcher、ActionMap、IntervalTimerを活用する。
class GameScene : public sgc::AppScene<SharedData>
{
public:
	using AppScene::AppScene;

	/// @brief シーン開始時の初期化
	void onEnter() override
	{
		setupInput();
		setupWorld();
		setupSystems();
		setupEvents();
		setupHud();

		m_particles.clear();
		m_gameOver = false;
	}

	/// @brief 更新処理（フレームワーク非依存）
	void update(float dt) override
	{
		// 入力更新（IInputProvider経由）
		std::vector<int> pressedKeys;
		getData().inputProvider->pollPressedKeys(pressedKeys);
		m_actionMap.update(pressedKeys);

		// ECSシステム実行（Render フェーズ含む）
		m_scheduler.update(m_world, dt);

		// パーティクル更新
		m_particles.update(dt);

		// スコアフラッシュ更新
		if (!m_scoreFlash.isFinished())
		{
			m_scoreFlashValue = m_scoreFlash.step(dt);
		}

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
			getSceneManager().changeScene("result"_hash, 0.5f);
		}
	}

	/// @brief 描画処理
	void draw() const override
	{
		getData().renderer->clearBackground(sgc::Colorf{0.02f, 0.02f, 0.08f, 1.0f});

		// パーティクル描画
		drawParticlesImpl();

		// HUD描画（エンティティはRenderフェーズでupdate()内から描画済み）
		drawHUD();
	}

	/// @brief シーン終了時のクリーンアップ
	void onExit() override
	{
		m_scheduler.clear();
		m_dispatcher.clearAll();
	}

private:
	sgc::ecs::World m_world;                     ///< ECSワールド
	sgc::ecs::SystemScheduler m_scheduler;       ///< システムスケジューラ
	sgc::ActionMap m_actionMap;                   ///< アクションマップ
	sgc::EventDispatcher m_dispatcher;           ///< イベントディスパッチャ

	sgc::ecs::Entity m_player{};                 ///< プレイヤーエンティティ
	std::vector<sgc::ecs::Entity> m_bullets;     ///< 弾エンティティリスト
	std::vector<sgc::ecs::Entity> m_enemies;     ///< 敵エンティティリスト

	sgc::ui::HudLayout m_hud;                    ///< HUDレイアウト
	sgc::ParticleSystem m_particles{500};        ///< パーティクルシステム
	sgc::Tweenf m_scoreFlash;                    ///< スコアフラッシュトゥイーン
	float m_scoreFlashValue{0.0f};               ///< スコアフラッシュ値（描画用キャッシュ）
	bool m_gameOver{false};                      ///< ゲームオーバーフラグ

	static constexpr float SCREEN_W = 800.0f;
	static constexpr float SCREEN_H = 600.0f;

	/// @brief 入力セットアップ（フレームワーク非依存）
	void setupInput()
	{
		m_actionMap = sgc::ActionMap{};

		m_actionMap.bind(Actions::MOVE_LEFT, KeyCode::LEFT);
		m_actionMap.bind(Actions::MOVE_LEFT, KeyCode::A);
		m_actionMap.bind(Actions::MOVE_RIGHT, KeyCode::RIGHT);
		m_actionMap.bind(Actions::MOVE_RIGHT, KeyCode::D);
		m_actionMap.bind(Actions::MOVE_UP, KeyCode::UP);
		m_actionMap.bind(Actions::MOVE_UP, KeyCode::W);
		m_actionMap.bind(Actions::MOVE_DOWN, KeyCode::DOWN);
		m_actionMap.bind(Actions::MOVE_DOWN, KeyCode::S);
		m_actionMap.bind(Actions::FIRE, KeyCode::SPACE);
		m_actionMap.bind(Actions::FIRE, KeyCode::Z);
	}

	/// @brief ECSワールドセットアップ
	void setupWorld()
	{
		m_bullets.clear();
		m_enemies.clear();

		// プレイヤー生成
		m_player = m_world.createEntity();
		m_world.addComponent(m_player, CTransform{
			.pos = {SCREEN_W / 2.0f, SCREEN_H - 60.0f}
		});
		m_world.addComponent(m_player, CVelocity{});
		m_world.addComponent(m_player, CSprite{
			.color = sgc::Colorf{0.2f, 1.0f, 0.4f, 1.0f},
			.radius = 10.0f
		});
		m_world.addComponent(m_player, CPlayer{
			.lives = 3,
			.fireRate = 0.12f,
			.fireCooldown = 0.0f
		});
	}

	/// @brief システム登録
	void setupSystems()
	{
		using Phase = sgc::ecs::Phase;

		// Update フェーズ
		m_scheduler.addSystem(InputSystem{
			.actionMap = &m_actionMap,
			.bullets = &m_bullets
		}, Phase::Update, 0);

		m_scheduler.addSystem(EnemySpawnSystem{
			.enemies = &m_enemies,
			.screenWidth = SCREEN_W
		}, Phase::Update, 5);

		m_scheduler.addSystem(MovementSystem{}, Phase::Update, 10);

		m_scheduler.addSystem(BulletSystem{
			.bullets = &m_bullets,
			.screenWidth = SCREEN_W,
			.screenHeight = SCREEN_H
		}, Phase::Update, 20);

		// LateUpdate フェーズ
		m_scheduler.addSystem(CollisionSystem{
			.bullets = &m_bullets,
			.enemies = &m_enemies,
			.dispatcher = &m_dispatcher,
			.playerEntity = m_player
		}, Phase::LateUpdate, 0);

		m_scheduler.addSystem(CleanupSystem{
			.enemies = &m_enemies,
			.screenHeight = SCREEN_H
		}, Phase::LateUpdate, 10);

		// Render フェーズ
		m_scheduler.addSystem(RenderSystem{
			.renderer = getData().renderer
		}, Phase::Render, 0);
	}

	/// @brief イベントリスナー登録
	void setupEvents()
	{
		// 敵撃破 → スコア加算 + パーティクル + フラッシュ
		m_dispatcher.on<EnemyKilled>(
			[this](const EnemyKilled& e)
			{
				getData().score += e.score;
				emitDeathParticles(m_particles, e.pos);

				// スコアフラッシュ（白→通常色への減衰）
				m_scoreFlashValue = startScoreFlash(m_scoreFlash);
			});

		// ゲームオーバー
		m_dispatcher.on<GameOver>(
			[this](const GameOver&)
			{
				m_gameOver = true;
			});
	}

	/// @brief プレイヤーを画面内にクランプ
	void clampPlayerPosition()
	{
		if (!m_world.isAlive(m_player)) return;

		auto* transform = m_world.getComponent<CTransform>(m_player);
		if (!transform) return;

		constexpr float MARGIN = 15.0f;
		transform->pos = transform->pos.clamped(
			sgc::Vec2f{MARGIN, MARGIN},
			sgc::Vec2f{SCREEN_W - MARGIN, SCREEN_H - MARGIN}
		);
	}

	/// @brief HUDレイアウトセットアップ
	void setupHud()
	{
		m_hud.clear();
		m_hud.add("score"_hash, {sgc::ui::Anchor::TopLeft, {10.0f, 10.0f}});
		m_hud.add("lives"_hash, {sgc::ui::Anchor::TopLeft, {10.0f, 40.0f}});
		m_hud.recalculate(sgc::ui::screenRect(SCREEN_W, SCREEN_H));
	}

	/// @brief パーティクル描画
	void drawParticlesImpl() const
	{
		drawParticles(getData().renderer, m_particles);
	}

	/// @brief HUD描画
	void drawHUD() const
	{
		const auto& data = getData();

		// スコア（フラッシュ時は黄色に光る）
		const auto scoreColor = sgc::Colorf::white().lerp(
			sgc::Colorf{1.0f, 1.0f, 0.2f, 1.0f}, m_scoreFlashValue);
		data.textRenderer->drawText(
			"Score: " + std::to_string(data.score), 24.0f,
			m_hud.position("score"_hash),
			scoreColor);

		// 残機表示
		if (m_world.isAlive(m_player))
		{
			const auto* player = m_world.getComponent<CPlayer>(m_player);
			if (player)
			{
				data.textRenderer->drawText(
					"Lives: " + std::to_string(player->lives), 24.0f,
					m_hud.position("lives"_hash),
					sgc::Colorf{0.2f, 1.0f, 0.4f, 1.0f});
			}
		}
	}
};
