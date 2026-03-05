#pragma once

/// @file GameScene.hpp
/// @brief ゲームプレイシーン

#include <Siv3D.hpp>
#include <vector>

#include "sgc/ecs/System.hpp"
#include "sgc/ecs/World.hpp"
#include "sgc/input/ActionMap.hpp"
#include "sgc/patterns/EventDispatcher.hpp"
#include "sgc/siv3d/DrawAdapter.hpp"
#include "sgc/siv3d/InputAdapter.hpp"
#include "sgc/siv3d/SceneAdapter.hpp"

#include "BulletSystem.hpp"
#include "CleanupSystem.hpp"
#include "CollisionSystem.hpp"
#include "Components.hpp"
#include "EnemySpawnSystem.hpp"
#include "Events.hpp"
#include "InputSystem.hpp"
#include "MovementSystem.hpp"
#include "RenderSystem.hpp"
#include "SharedData.hpp"

/// @brief ゲームプレイシーン
///
/// ECSワールドを初期化し、各システムを登録してゲームループを実行する。
/// sgcのECS、EventDispatcher、ActionMap、IntervalTimerを活用する。
class GameScene : public sgc::siv3d::AppScene<SharedData>
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

		m_gameOver = false;
	}

	/// @brief 更新処理（実装はMain.cppで完全型が揃った後に定義）
	void update(float dt) override;

	/// @brief 描画処理
	void draw() const override
	{
		sgc::siv3d::clearBackground(sgc::Colorf{0.02f, 0.02f, 0.08f, 1.0f});

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
	sgc::siv3d::InputAdapter m_inputAdapter;     ///< 入力アダプター
	sgc::EventDispatcher m_dispatcher;           ///< イベントディスパッチャ

	sgc::ecs::Entity m_player{};                 ///< プレイヤーエンティティ
	std::vector<sgc::ecs::Entity> m_bullets;     ///< 弾エンティティリスト
	std::vector<sgc::ecs::Entity> m_enemies;     ///< 敵エンティティリスト

	bool m_gameOver{false};                      ///< ゲームオーバーフラグ

	static constexpr float SCREEN_W = 800.0f;
	static constexpr float SCREEN_H = 600.0f;

	/// @brief 入力セットアップ
	void setupInput()
	{
		m_inputAdapter.clear();
		m_inputAdapter.bind(s3d::KeyLeft, Actions::MOVE_LEFT);
		m_inputAdapter.bind(s3d::KeyA, Actions::MOVE_LEFT);
		m_inputAdapter.bind(s3d::KeyRight, Actions::MOVE_RIGHT);
		m_inputAdapter.bind(s3d::KeyD, Actions::MOVE_RIGHT);
		m_inputAdapter.bind(s3d::KeyUp, Actions::MOVE_UP);
		m_inputAdapter.bind(s3d::KeyW, Actions::MOVE_UP);
		m_inputAdapter.bind(s3d::KeyDown, Actions::MOVE_DOWN);
		m_inputAdapter.bind(s3d::KeyS, Actions::MOVE_DOWN);
		m_inputAdapter.bind(s3d::KeySpace, Actions::FIRE);
		m_inputAdapter.bind(s3d::KeyZ, Actions::FIRE);
		m_inputAdapter.setupBindings(m_actionMap);
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
		m_scheduler.addSystem(RenderSystem{}, Phase::Render, 0);
	}

	/// @brief イベントリスナー登録
	void setupEvents()
	{
		// 敵撃破 → スコア加算
		m_dispatcher.on<EnemyKilled>(
			[this](const EnemyKilled& e)
			{
				getData().score += e.score;
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

	/// @brief HUD描画
	void drawHUD() const
	{
		const auto& data = getData();

		// スコア
		sgc::siv3d::drawText(
			data.scoreFont,
			s3d::Format(U"Score: ", data.score),
			sgc::Vec2f{10.0f, 10.0f},
			sgc::Colorf::white());

		// 残機表示
		if (m_world.isAlive(m_player))
		{
			const auto* player = m_world.getComponent<CPlayer>(m_player);
			if (player)
			{
				sgc::siv3d::drawText(
					data.scoreFont,
					s3d::Format(U"Lives: ", player->lives),
					sgc::Vec2f{10.0f, 40.0f},
					sgc::Colorf{0.2f, 1.0f, 0.4f, 1.0f});
			}
		}
	}
};
