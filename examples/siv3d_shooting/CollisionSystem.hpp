#pragma once

/// @file CollisionSystem.hpp
/// @brief 弾-敵 衝突判定システム

#include <vector>

#include "sgc/ecs/Entity.hpp"
#include "sgc/ecs/World.hpp"
#include "sgc/patterns/EventDispatcher.hpp"

#include "Components.hpp"
#include "Events.hpp"

/// @brief 弾-敵 衝突判定システム
///
/// プレイヤー弾と敵の円同士の衝突判定を行う。
/// 衝突時にイベントを発行し、エンティティを破棄する。
/// Phase::LateUpdate, Priority 0 で実行する。
struct CollisionSystem
{
	std::vector<sgc::ecs::Entity>* bullets{nullptr};   ///< 弾リスト
	std::vector<sgc::ecs::Entity>* enemies{nullptr};   ///< 敵リスト
	sgc::EventDispatcher* dispatcher{nullptr};          ///< イベントディスパッチャ

	sgc::ecs::Entity playerEntity{};  ///< プレイヤーエンティティ

	/// @brief 衝突判定処理
	void update(sgc::ecs::World& world, float /*dt*/)
	{
		if (!bullets || !enemies) return;

		checkBulletEnemyCollisions(world);
		checkEnemyPlayerCollision(world);
	}

private:
	/// @brief 弾と敵の衝突判定
	void checkBulletEnemyCollisions(sgc::ecs::World& world)
	{
		// 衝突した弾と敵を記録
		std::vector<sgc::ecs::Entity> deadBullets;
		std::vector<sgc::ecs::Entity> deadEnemies;

		for (const auto& bulletEntity : *bullets)
		{
			if (!world.isAlive(bulletEntity)) continue;

			const auto* bulletComp = world.getComponent<CBullet>(bulletEntity);
			if (!bulletComp || !bulletComp->isPlayerBullet) continue;

			const auto* bulletTransform = world.getComponent<CTransform>(bulletEntity);
			const auto* bulletSprite = world.getComponent<CSprite>(bulletEntity);
			if (!bulletTransform || !bulletSprite) continue;

			for (const auto& enemyEntity : *enemies)
			{
				if (!world.isAlive(enemyEntity)) continue;

				const auto* enemyTransform = world.getComponent<CTransform>(enemyEntity);
				const auto* enemySprite = world.getComponent<CSprite>(enemyEntity);
				auto* enemyComp = world.getComponent<CEnemy>(enemyEntity);
				if (!enemyTransform || !enemySprite || !enemyComp) continue;

				// 円同士の衝突判定
				const float dist = bulletTransform->pos.distanceTo(enemyTransform->pos);
				const float combinedRadius = bulletSprite->radius + enemySprite->radius;

				if (dist < combinedRadius)
				{
					// 弾は破棄
					deadBullets.push_back(bulletEntity);

					// 敵のHP減少
					enemyComp->hp -= 1;
					if (enemyComp->hp <= 0)
					{
						deadEnemies.push_back(enemyEntity);

						// 撃破イベント発行
						if (dispatcher)
						{
							dispatcher->emit(EnemyKilled{
								.pos = enemyTransform->pos,
								.score = enemyComp->scoreValue
							});
						}
					}
					break;  // 1弾は1敵にのみヒット
				}
			}
		}

		// 弾の破棄
		for (const auto& e : deadBullets)
		{
			world.destroyEntity(e);
		}

		// 敵の破棄
		for (const auto& e : deadEnemies)
		{
			world.destroyEntity(e);
		}

		// リストから削除
		std::erase_if(*bullets,
			[&world](const sgc::ecs::Entity& e) { return !world.isAlive(e); });
		std::erase_if(*enemies,
			[&world](const sgc::ecs::Entity& e) { return !world.isAlive(e); });
	}

	/// @brief 敵とプレイヤーの衝突判定
	void checkEnemyPlayerCollision(sgc::ecs::World& world)
	{
		if (!world.isAlive(playerEntity)) return;

		const auto* playerTransform = world.getComponent<CTransform>(playerEntity);
		const auto* playerSprite = world.getComponent<CSprite>(playerEntity);
		auto* playerComp = world.getComponent<CPlayer>(playerEntity);
		if (!playerTransform || !playerComp) return;

		const float PLAYER_RADIUS = playerSprite ? playerSprite->radius : 8.0f;

		std::vector<sgc::ecs::Entity> deadEnemies;

		for (const auto& enemyEntity : *enemies)
		{
			if (!world.isAlive(enemyEntity)) continue;

			const auto* enemyTransform = world.getComponent<CTransform>(enemyEntity);
			const auto* enemySprite = world.getComponent<CSprite>(enemyEntity);
			if (!enemyTransform || !enemySprite) continue;

			const float dist = playerTransform->pos.distanceTo(enemyTransform->pos);
			if (dist < PLAYER_RADIUS + enemySprite->radius)
			{
				deadEnemies.push_back(enemyEntity);

				playerComp->lives -= 1;

				if (dispatcher)
				{
					dispatcher->emit(PlayerDamaged{.remainingLives = playerComp->lives});

					if (playerComp->lives <= 0)
					{
						dispatcher->emit(GameOver{});
					}
				}
				break;  // 1フレーム1回の被弾
			}
		}

		for (const auto& e : deadEnemies)
		{
			world.destroyEntity(e);
		}

		std::erase_if(*enemies,
			[&world](const sgc::ecs::Entity& e) { return !world.isAlive(e); });
	}
};
