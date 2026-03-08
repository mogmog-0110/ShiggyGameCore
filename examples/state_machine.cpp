/// @file state_machine.cpp
/// @brief StateMachineでゲームキャラクターの状態遷移を実演するサンプル
///
/// std::variant + std::visit ベースの有限状態マシンを使い、
/// ゲームキャラクターの4状態（Idle / Walk / Jump / Attack）を管理する。
/// - 状態ごとの構造体定義とデータ保持
/// - onEnter / onExit コールバック
/// - update()内のvisitorによる状態遷移
/// - ガード関数による遷移制御
/// - 状態の問い合わせ（isIn, visit）

#include <iostream>
#include <optional>
#include <string>
#include <type_traits>

#include "sgc/patterns/StateMachine.hpp"

// ── 状態型の定義 ──────────────────────────────────────────────
// 各状態は構造体で表現し、状態固有のデータを持てる

/// @brief 待機状態
struct Idle
{
	int idleFrames{0};  ///< 待機フレーム数（暇演出に使う）
};

/// @brief 歩行状態
struct Walk
{
	float speed{0.0f};       ///< 移動速度
	float distance{0.0f};    ///< 移動距離の累積
};

/// @brief ジャンプ状態
struct Jump
{
	float verticalVelocity{0.0f};  ///< 垂直速度
	float height{0.0f};            ///< 現在の高さ
	bool ascending{true};          ///< 上昇中か
};

/// @brief 攻撃状態
struct Attack
{
	int damage{0};              ///< 攻撃力
	int remainingFrames{0};     ///< 攻撃モーション残りフレーム
	std::string attackName;     ///< 技名
};

/// @brief 状態マシン型エイリアス
using CharacterFSM = sgc::StateMachine<Idle, Walk, Jump, Attack>;

/// @brief 状態名を文字列で返すvisitor
/// @param state 現在の状態variant
/// @return 状態名
std::string getStateName(const CharacterFSM::StateVariant& state)
{
	return std::visit([](const auto& s) -> std::string
	{
		using T = std::decay_t<decltype(s)>;
		if constexpr (std::is_same_v<T, Idle>)   return "Idle";
		if constexpr (std::is_same_v<T, Walk>)   return "Walk";
		if constexpr (std::is_same_v<T, Jump>)   return "Jump";
		if constexpr (std::is_same_v<T, Attack>) return "Attack";
	}, state);
}

/// @brief 状態の詳細情報を表示するvisitor
/// @param state 現在の状態variant
void printStateDetail(const CharacterFSM::StateVariant& state)
{
	std::visit([](const auto& s)
	{
		using T = std::decay_t<decltype(s)>;
		if constexpr (std::is_same_v<T, Idle>)
		{
			std::cout << "    (idle frames: " << s.idleFrames << ")\n";
		}
		if constexpr (std::is_same_v<T, Walk>)
		{
			std::cout << "    (speed: " << s.speed
					  << ", distance: " << s.distance << ")\n";
		}
		if constexpr (std::is_same_v<T, Jump>)
		{
			std::cout << "    (height: " << s.height
					  << ", velocity: " << s.verticalVelocity
					  << ", " << (s.ascending ? "ascending" : "descending") << ")\n";
		}
		if constexpr (std::is_same_v<T, Attack>)
		{
			std::cout << "    (attack: " << s.attackName
					  << ", damage: " << s.damage
					  << ", frames left: " << s.remainingFrames << ")\n";
		}
	}, state);
}

int main()
{
	std::cout << "=== sgc StateMachine Demo ===\n\n";

	// ────────────────────────────────────────────────────
	// 1. 状態マシンの初期化とコールバック設定
	// ────────────────────────────────────────────────────
	std::cout << "--- 1. Setup ---\n";

	CharacterFSM fsm{Idle{0}};

	/// onEnterコールバック: 状態に入った時に表示
	fsm.setOnEnter([](const CharacterFSM::StateVariant& state)
	{
		std::cout << "  >> Enter: " << getStateName(state) << "\n";
	});

	/// onExitコールバック: 状態を出る時に表示
	fsm.setOnExit([](const CharacterFSM::StateVariant& state)
	{
		std::cout << "  << Exit:  " << getStateName(state) << "\n";
	});

	/// 遷移コールバック: 遷移元→遷移先を表示
	fsm.setOnTransition([](const CharacterFSM::StateVariant& from,
						   const CharacterFSM::StateVariant& to)
	{
		std::cout << "  == Transition: " << getStateName(from)
				  << " -> " << getStateName(to) << "\n";
	});

	std::cout << "  Initial state: " << getStateName(fsm.current()) << "\n";

	// ────────────────────────────────────────────────────
	// 2. ゲームループシミュレーション
	// ────────────────────────────────────────────────────
	std::cout << "\n--- 2. Game Loop Simulation ---\n";

	/// シミュレーション用のゲーム入力（各フレームの行動指示）
	enum class Action { None, MoveRight, JumpStart, AttackSlash, Stop };
	const Action scriptedInputs[] = {
		Action::None,        // フレーム0: 待機
		Action::None,        // フレーム1: 待機（暇演出カウントアップ）
		Action::MoveRight,   // フレーム2: 歩行開始
		Action::None,        // フレーム3: 歩行継続
		Action::None,        // フレーム4: 歩行継続
		Action::JumpStart,   // フレーム5: ジャンプ開始
		Action::None,        // フレーム6: ジャンプ上昇中
		Action::None,        // フレーム7: ジャンプ頂点付近
		Action::None,        // フレーム8: ジャンプ下降→着地
		Action::AttackSlash, // フレーム9: 攻撃開始
		Action::None,        // フレーム10: 攻撃モーション中
		Action::None,        // フレーム11: 攻撃モーション終了→Idle
		Action::Stop,        // フレーム12: 終了
	};

	for (int frame = 0; frame < 13; ++frame)
	{
		const Action input = scriptedInputs[frame];
		std::cout << "\n  [Frame " << frame << "] State: "
				  << getStateName(fsm.current()) << "\n";
		printStateDetail(fsm.current());

		/// update()はvisitorを受け取り、各状態ごとの遷移ロジックを処理する。
		/// std::nulloptを返すと状態は変化しない。
		fsm.update([input](auto& state) -> std::optional<CharacterFSM::StateVariant>
		{
			using T = std::decay_t<decltype(state)>;

			if constexpr (std::is_same_v<T, Idle>)
			{
				++state.idleFrames;
				if (input == Action::MoveRight) return Walk{3.5f, 0.0f};
				if (input == Action::JumpStart) return Jump{10.0f, 0.0f, true};
				if (input == Action::AttackSlash) return Attack{25, 3, "Slash"};
				return std::nullopt;
			}
			if constexpr (std::is_same_v<T, Walk>)
			{
				state.distance += state.speed;
				if (input == Action::JumpStart) return Jump{12.0f, 0.0f, true};
				if (input == Action::AttackSlash) return Attack{30, 3, "Running Slash"};
				if (input == Action::Stop) return Idle{0};
				return std::nullopt;
			}
			if constexpr (std::is_same_v<T, Jump>)
			{
				/// 簡易ジャンプ物理: 上昇→頂点→下降→着地
				state.height += state.verticalVelocity;
				state.verticalVelocity -= 4.0f;  // 重力
				if (state.verticalVelocity < 0.0f) state.ascending = false;
				if (state.height <= 0.0f && !state.ascending)
				{
					state.height = 0.0f;
					return Idle{0};  // 着地→Idle
				}
				return std::nullopt;
			}
			if constexpr (std::is_same_v<T, Attack>)
			{
				--state.remainingFrames;
				if (state.remainingFrames <= 0)
				{
					return Idle{0};  // 攻撃終了→Idle
				}
				return std::nullopt;
			}
			return std::nullopt;
		});
	}

	// ────────────────────────────────────────────────────
	// 3. 状態の問い合わせ
	// ────────────────────────────────────────────────────
	std::cout << "\n--- 3. State Query ---\n";
	std::cout << "  isIn<Idle>():   " << (fsm.isIn<Idle>() ? "true" : "false") << "\n";
	std::cout << "  isIn<Walk>():   " << (fsm.isIn<Walk>() ? "true" : "false") << "\n";
	std::cout << "  isIn<Jump>():   " << (fsm.isIn<Jump>() ? "true" : "false") << "\n";
	std::cout << "  isIn<Attack>(): " << (fsm.isIn<Attack>() ? "true" : "false") << "\n";

	/// visitで値を取り出す
	const std::string desc = fsm.visit([](const auto& s) -> std::string
	{
		using T = std::decay_t<decltype(s)>;
		if constexpr (std::is_same_v<T, Idle>) return "Character is resting.";
		if constexpr (std::is_same_v<T, Walk>) return "Character is moving.";
		if constexpr (std::is_same_v<T, Jump>) return "Character is airborne.";
		if constexpr (std::is_same_v<T, Attack>) return "Character is fighting.";
	});
	std::cout << "  Description: " << desc << "\n";

	// ────────────────────────────────────────────────────
	// 4. ガード関数による遷移制御
	// ────────────────────────────────────────────────────
	std::cout << "\n--- 4. Guard Function ---\n";

	/// 攻撃中→ジャンプへの遷移を禁止するガード
	fsm.addGuard([](const CharacterFSM::StateVariant& from,
					const CharacterFSM::StateVariant& to) -> bool
	{
		const bool fromAttack = std::holds_alternative<Attack>(from);
		const bool toJump = std::holds_alternative<Jump>(to);
		if (fromAttack && toJump)
		{
			std::cout << "  [Guard] BLOCKED: Cannot jump during attack!\n";
			return false;
		}
		return true;
	});

	/// Attack状態に強制設定
	fsm.setState(Attack{50, 5, "Heavy Strike"});
	std::cout << "  Forced to Attack state.\n";

	/// 攻撃中にジャンプを試みる→ガードがブロック
	fsm.update([](auto& state) -> std::optional<CharacterFSM::StateVariant>
	{
		using T = std::decay_t<decltype(state)>;
		if constexpr (std::is_same_v<T, Attack>)
		{
			std::cout << "  Attempting to jump during Attack...\n";
			return Jump{15.0f, 0.0f, true};
		}
		return std::nullopt;
	});

	std::cout << "  State after blocked transition: "
			  << getStateName(fsm.current()) << "\n";

	// ────────────────────────────────────────────────────
	// 5. 前の状態の参照
	// ────────────────────────────────────────────────────
	std::cout << "\n--- 5. Previous State ---\n";
	if (fsm.previousState().has_value())
	{
		std::cout << "  Previous state: "
				  << getStateName(*fsm.previousState()) << "\n";
	}
	else
	{
		std::cout << "  No previous state recorded.\n";
	}

	std::cout << "\n=== Demo Complete ===\n";
	return 0;
}
