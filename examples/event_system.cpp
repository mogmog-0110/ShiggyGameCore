/// @file event_system.cpp
/// @brief EventDispatcherの型安全イベントシステムを実演するサンプル
///
/// sgcのEventDispatcherを使ったパブリッシュ/サブスクライブパターンのデモ。
/// - カスタムイベント構造体の定義
/// - 複数リスナーの登録と発火
/// - 優先度制御（High / Normal / Low）
/// - リスナーの解除（off）
/// - once()による1回限りリスナー
/// - 登録していない型のイベントは無視される

#include <iostream>
#include <string>

#include "sgc/patterns/EventDispatcher.hpp"

// ── カスタムイベント定義 ──────────────────────────────────────

/// @brief ダメージイベント
struct DamageEvent
{
	int amount;            ///< ダメージ量
	std::string source;    ///< ダメージ源（敵名等）
};

/// @brief 回復イベント
struct HealEvent
{
	int amount;            ///< 回復量
	std::string healerName; ///< 回復者名
};

/// @brief レベルアップイベント
struct LevelUpEvent
{
	int newLevel;          ///< 新しいレベル
	int statPoints;        ///< 獲得ステータスポイント
};

/// @brief アイテム取得イベント（リスナー未登録の型として使う）
struct ItemPickupEvent
{
	std::string itemName;
};

int main()
{
	std::cout << "=== sgc EventDispatcher Demo ===\n\n";

	sgc::EventDispatcher dispatcher;

	// ────────────────────────────────────────────────────
	// 1. 基本的なイベントリスナー登録と発火
	// ────────────────────────────────────────────────────
	std::cout << "--- 1. Basic Event Subscription ---\n";

	/// ダメージイベントのリスナーを登録（HPバー更新役）
	const auto hpBarListener = dispatcher.on<DamageEvent>(
		[](const DamageEvent& e)
		{
			std::cout << "  [HP Bar] Took " << e.amount
					  << " damage from " << e.source << "!\n";
		});

	/// ダメージイベントの別リスナー（サウンド再生役）
	const auto soundListener = dispatcher.on<DamageEvent>(
		[](const DamageEvent& e)
		{
			std::cout << "  [Sound] Playing hit sound (damage: "
					  << e.amount << ")\n";
		});

	/// ダメージイベントの別リスナー（ダメージログ役）
	const auto logListener = dispatcher.on<DamageEvent>(
		[](const DamageEvent& e)
		{
			std::cout << "  [Combat Log] " << e.source
					  << " dealt " << e.amount << " damage.\n";
		});

	std::cout << "\n  Emitting DamageEvent(30, \"Goblin\"):\n";
	dispatcher.emit(DamageEvent{30, "Goblin"});

	std::cout << "\n  Emitting DamageEvent(55, \"Dragon\"):\n";
	dispatcher.emit(DamageEvent{55, "Dragon"});

	// ────────────────────────────────────────────────────
	// 2. 複数のイベント型
	// ────────────────────────────────────────────────────
	std::cout << "\n--- 2. Multiple Event Types ---\n";

	/// 回復イベントのリスナー
	dispatcher.on<HealEvent>(
		[](const HealEvent& e)
		{
			std::cout << "  [HP Bar] Healed " << e.amount
					  << " HP by " << e.healerName << ".\n";
		});

	/// レベルアップイベントのリスナー
	dispatcher.on<LevelUpEvent>(
		[](const LevelUpEvent& e)
		{
			std::cout << "  [Level Up!] Now level " << e.newLevel
					  << " (++" << e.statPoints << " stat points)\n";
		});

	std::cout << "  Emitting HealEvent(20, \"Cleric\"):\n";
	dispatcher.emit(HealEvent{20, "Cleric"});

	std::cout << "\n  Emitting LevelUpEvent(5, 3):\n";
	dispatcher.emit(LevelUpEvent{5, 3});

	// ────────────────────────────────────────────────────
	// 3. 登録していない型のイベントは無視される
	// ────────────────────────────────────────────────────
	std::cout << "\n--- 3. Unregistered Event Type ---\n";
	std::cout << "  Emitting ItemPickupEvent (no listeners registered):\n";
	dispatcher.emit(ItemPickupEvent{"Magic Sword"});
	std::cout << "  (nothing happened - no listeners for this type)\n";

	// ────────────────────────────────────────────────────
	// 4. リスナーの解除
	// ────────────────────────────────────────────────────
	std::cout << "\n--- 4. Removing a Listener (off) ---\n";
	std::cout << "  Removing sound listener (id=" << soundListener << ")...\n";
	dispatcher.off(soundListener);

	std::cout << "  Emitting DamageEvent(10, \"Skeleton\"):\n";
	dispatcher.emit(DamageEvent{10, "Skeleton"});
	std::cout << "  (sound listener is gone - only HP Bar and Combat Log fired)\n";

	// ────────────────────────────────────────────────────
	// 5. 優先度制御
	// ────────────────────────────────────────────────────
	std::cout << "\n--- 5. Priority Control ---\n";

	/// 既存リスナーをクリアしてから再登録
	dispatcher.clearListeners<DamageEvent>();

	/// 低優先度: UIエフェクト
	dispatcher.on<DamageEvent>(
		[](const DamageEvent&)
		{
			std::cout << "  [3rd - Low] UI shake effect\n";
		}, sgc::Priority::Low);

	/// 高優先度: シールド判定（ダメージ軽減）
	dispatcher.on<DamageEvent>(
		[](const DamageEvent&)
		{
			std::cout << "  [1st - High] Shield check (damage reduction)\n";
		}, sgc::Priority::High);

	/// 通常優先度: HP更新
	dispatcher.on<DamageEvent>(
		[](const DamageEvent&)
		{
			std::cout << "  [2nd - Normal] Apply damage to HP\n";
		}, sgc::Priority::Normal);

	std::cout << "  Emitting DamageEvent (should fire High -> Normal -> Low):\n";
	dispatcher.emit(DamageEvent{40, "Boss"});

	// ────────────────────────────────────────────────────
	// 6. once() — 1回限りのリスナー
	// ────────────────────────────────────────────────────
	std::cout << "\n--- 6. Once Listener ---\n";

	/// 最初のレベルアップだけ特別演出を出す
	dispatcher.once<LevelUpEvent>(
		[](const LevelUpEvent& e)
		{
			std::cout << "  [ONCE] First level up special effect! (Lv."
					  << e.newLevel << ")\n";
		});

	std::cout << "  Emit LevelUpEvent #1:\n";
	dispatcher.emit(LevelUpEvent{6, 3});

	std::cout << "\n  Emit LevelUpEvent #2 (once listener should be gone):\n";
	dispatcher.emit(LevelUpEvent{7, 3});

	// ────────────────────────────────────────────────────
	// 7. 実践的な例: ゲームの戦闘システム
	// ────────────────────────────────────────────────────
	std::cout << "\n--- 7. Practical Example: Combat System ---\n";

	/// プレイヤーのHP管理
	int playerHP = 100;
	int totalDamageReceived = 0;
	int totalHealing = 0;

	/// 全リスナーをリセット
	dispatcher.clearAll();

	/// ダメージ処理（HP減少）
	dispatcher.on<DamageEvent>(
		[&playerHP, &totalDamageReceived](const DamageEvent& e)
		{
			playerHP -= e.amount;
			totalDamageReceived += e.amount;
			if (playerHP < 0) playerHP = 0;
			std::cout << "  HP: " << (playerHP + e.amount)
					  << " -> " << playerHP
					  << " (-" << e.amount << " from " << e.source << ")\n";
		});

	/// 回復処理（HP増加）
	dispatcher.on<HealEvent>(
		[&playerHP, &totalHealing](const HealEvent& e)
		{
			const int before = playerHP;
			playerHP += e.amount;
			if (playerHP > 100) playerHP = 100;
			totalHealing += (playerHP - before);
			std::cout << "  HP: " << before << " -> " << playerHP
					  << " (+" << (playerHP - before) << " by " << e.healerName << ")\n";
		});

	/// 戦闘シナリオを実行
	std::cout << "  Starting combat (HP: " << playerHP << "/100):\n\n";

	dispatcher.emit(DamageEvent{25, "Wolf"});
	dispatcher.emit(DamageEvent{15, "Wolf"});
	dispatcher.emit(HealEvent{10, "Potion"});
	dispatcher.emit(DamageEvent{35, "Wolf Alpha"});
	dispatcher.emit(HealEvent{50, "Cleric"});
	dispatcher.emit(DamageEvent{20, "Trap"});

	std::cout << "\n  Battle Summary:\n";
	std::cout << "    Final HP:       " << playerHP << "/100\n";
	std::cout << "    Total Damage:   " << totalDamageReceived << "\n";
	std::cout << "    Total Healing:  " << totalHealing << "\n";

	std::cout << "\n=== Demo Complete ===\n";
	return 0;
}
