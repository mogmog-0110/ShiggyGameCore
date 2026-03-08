/// @file behavior_tree.cpp
/// @brief BehaviorTreeでゲームAIを構築するサンプル

#include <iostream>
#include <string>

#include "sgc/ai/BehaviorTree.hpp"

/// @brief ステータスを文字列に変換する
const char* statusToString(sgc::bt::Status s)
{
	switch (s)
	{
	case sgc::bt::Status::Success: return "Success";
	case sgc::bt::Status::Failure: return "Failure";
	case sgc::bt::Status::Running: return "Running";
	}
	return "Unknown";
}

/// @brief ブラックボードの状態を表示する
void printWorldState(const sgc::bt::Blackboard& bb)
{
	std::cout << "  World State:" << std::endl;
	std::cout << "    enemyVisible = "
		<< (bb.get<bool>("enemyVisible").value_or(false) ? "true" : "false") << std::endl;
	std::cout << "    isHungry     = "
		<< (bb.get<bool>("isHungry").value_or(false) ? "true" : "false") << std::endl;
	std::cout << "    action       = "
		<< bb.get<std::string>("action").value_or("(none)") << std::endl;
}

int main()
{
	std::cout << "--- BehaviorTree Demo: Guard NPC AI ---" << std::endl;
	std::cout << std::endl;

	// ── 1. ビヘイビアツリーの構築 ───────────────────────────
	/// ガードNPCのAIツリー:
	///   Selector
	///   +-- Sequence: [敵が見える？ → 敵を追跡]
	///   +-- Sequence: [空腹？ → 食事する]
	///   +-- Action: パトロール
	auto tree = sgc::bt::Builder()
		.selector()
			.sequence()
				.condition([](sgc::bt::Blackboard& bb)
				{
					return bb.get<bool>("enemyVisible").value_or(false);
				})
				.action([](sgc::bt::Blackboard& bb)
				{
					bb.set("action", std::string("Chasing enemy!"));
					std::cout << "    -> [Action] Chase enemy!" << std::endl;
					return sgc::bt::Status::Success;
				})
			.end()
			.sequence()
				.condition([](sgc::bt::Blackboard& bb)
				{
					return bb.get<bool>("isHungry").value_or(false);
				})
				.action([](sgc::bt::Blackboard& bb)
				{
					bb.set("action", std::string("Eating food"));
					std::cout << "    -> [Action] Eat food" << std::endl;
					return sgc::bt::Status::Success;
				})
			.end()
			.action([](sgc::bt::Blackboard& bb)
			{
				bb.set("action", std::string("Patrolling"));
				std::cout << "    -> [Action] Patrol" << std::endl;
				return sgc::bt::Status::Success;
			})
		.end()
		.build();

	sgc::bt::Blackboard bb;

	// ── 2. シナリオ1: 平常時（何も脅威なし） ────────────────
	std::cout << "=== Scenario 1: Peaceful (no threats) ===" << std::endl;
	bb.set<bool>("enemyVisible", false);
	bb.set<bool>("isHungry", false);
	printWorldState(bb);

	auto result = tree->tick(bb);
	std::cout << "  Result: " << statusToString(result) << std::endl;
	std::cout << "  Decision: " << bb.get<std::string>("action").value_or("?") << std::endl;
	std::cout << std::endl;

	// ── 3. シナリオ2: 敵を発見 ──────────────────────────────
	std::cout << "=== Scenario 2: Enemy spotted! ===" << std::endl;
	bb.set<bool>("enemyVisible", true);
	bb.set<bool>("isHungry", true);
	printWorldState(bb);

	result = tree->tick(bb);
	std::cout << "  Result: " << statusToString(result) << std::endl;
	std::cout << "  Decision: " << bb.get<std::string>("action").value_or("?") << std::endl;
	std::cout << "  (Enemy takes priority over hunger)" << std::endl;
	std::cout << std::endl;

	// ── 4. シナリオ3: 空腹（敵なし） ────────────────────────
	std::cout << "=== Scenario 3: Hungry guard ===" << std::endl;
	bb.set<bool>("enemyVisible", false);
	bb.set<bool>("isHungry", true);
	printWorldState(bb);

	result = tree->tick(bb);
	std::cout << "  Result: " << statusToString(result) << std::endl;
	std::cout << "  Decision: " << bb.get<std::string>("action").value_or("?") << std::endl;
	std::cout << std::endl;

	// ── 5. Inverterデコレータの例 ───────────────────────────
	std::cout << "=== Bonus: Inverter Decorator ===" << std::endl;
	std::cout << "  Building tree: inverter + condition(enemyVisible)" << std::endl;

	auto invertedTree = sgc::bt::Builder()
		.inverter()
		.condition([](sgc::bt::Blackboard& board)
		{
			return board.get<bool>("enemyVisible").value_or(false);
		})
		.build();

	bb.set<bool>("enemyVisible", true);
	auto invResult = invertedTree->tick(bb);
	std::cout << "  enemyVisible=true, inverted result: " << statusToString(invResult) << std::endl;

	bb.set<bool>("enemyVisible", false);
	invResult = invertedTree->tick(bb);
	std::cout << "  enemyVisible=false, inverted result: " << statusToString(invResult) << std::endl;

	return 0;
}
