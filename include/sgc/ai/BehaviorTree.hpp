#pragma once

/// @file BehaviorTree.hpp
/// @brief ビヘイビアツリー（Behavior Tree）によるAI意思決定
///
/// ゲームAIの行動制御に使用する。
/// Blackboardによるデータ共有、Composite/Decorator/Leafノード、
/// fluent APIのBuilderを提供する。

#include <any>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace sgc::bt
{

/// @brief ノードの実行結果
enum class Status
{
	Success,  ///< 成功
	Failure,  ///< 失敗
	Running   ///< 実行中
};

// ── Blackboard ──────────────────────────────────────────────────

/// @brief ノード間でデータを共有するブラックボード
///
/// @code
/// sgc::bt::Blackboard bb;
/// bb.set<int>("health", 100);
/// auto hp = bb.get<int>("health"); // 100
/// @endcode
class Blackboard
{
public:
	/// @brief 値を設定する
	/// @tparam T 値の型
	/// @param key キー名
	/// @param value 値
	template <typename T>
	void set(const std::string& key, T value)
	{
		m_data[key] = std::move(value);
	}

	/// @brief 値を取得する
	/// @tparam T 値の型
	/// @param key キー名
	/// @return 値（存在しない場合はnullopt）
	template <typename T>
	[[nodiscard]] std::optional<T> get(const std::string& key) const
	{
		const auto it = m_data.find(key);
		if (it == m_data.end()) return std::nullopt;

		try
		{
			return std::any_cast<T>(it->second);
		}
		catch (const std::bad_any_cast&)
		{
			return std::nullopt;
		}
	}

	/// @brief キーが存在するか確認する
	/// @param key キー名
	[[nodiscard]] bool has(const std::string& key) const
	{
		return m_data.contains(key);
	}

	/// @brief キーを削除する
	/// @param key キー名
	void remove(const std::string& key)
	{
		m_data.erase(key);
	}

	/// @brief 全データをクリアする
	void clear()
	{
		m_data.clear();
	}

private:
	std::unordered_map<std::string, std::any> m_data;
};

// ── Node（基底） ──────────────────────────────────────────────────

/// @brief ビヘイビアツリーのノード基底クラス
class Node
{
public:
	virtual ~Node() = default;

	/// @brief ノードを実行する
	/// @param bb ブラックボード
	/// @return 実行結果
	[[nodiscard]] virtual Status tick(Blackboard& bb) = 0;
};

// ── Composite ─────────────────────────────────────────────────

/// @brief Sequenceノード — 全子ノードがSuccessで成功
///
/// 子ノードを順に実行し、全てSuccessなら成功。
/// FailureまたはRunningが返されたら即座にそのステータスを返す。
class Sequence : public Node
{
public:
	/// @brief 子ノードを追加する
	/// @param child 子ノード
	void addChild(std::unique_ptr<Node> child)
	{
		m_children.push_back(std::move(child));
	}

	[[nodiscard]] Status tick(Blackboard& bb) override
	{
		for (auto& child : m_children)
		{
			const Status s = child->tick(bb);
			if (s != Status::Success) return s;
		}
		return Status::Success;
	}

private:
	std::vector<std::unique_ptr<Node>> m_children;
};

/// @brief Selectorノード — 最初のSuccessで成功
///
/// 子ノードを順に実行し、最初にSuccessを返した時点で成功。
/// 全てFailureなら失敗。Runningが返されたら即座にRunningを返す。
class Selector : public Node
{
public:
	/// @brief 子ノードを追加する
	/// @param child 子ノード
	void addChild(std::unique_ptr<Node> child)
	{
		m_children.push_back(std::move(child));
	}

	[[nodiscard]] Status tick(Blackboard& bb) override
	{
		for (auto& child : m_children)
		{
			const Status s = child->tick(bb);
			if (s != Status::Failure) return s;
		}
		return Status::Failure;
	}

private:
	std::vector<std::unique_ptr<Node>> m_children;
};

// ── Decorator ─────────────────────────────────────────────────

/// @brief Inverterデコレータ — Success↔Failureを反転
class Inverter : public Node
{
public:
	/// @param child 子ノード
	explicit Inverter(std::unique_ptr<Node> child)
		: m_child(std::move(child)) {}

	[[nodiscard]] Status tick(Blackboard& bb) override
	{
		const Status s = m_child->tick(bb);
		if (s == Status::Success) return Status::Failure;
		if (s == Status::Failure) return Status::Success;
		return Status::Running;
	}

private:
	std::unique_ptr<Node> m_child;
};

/// @brief Repeaterデコレータ — N回繰り返し
class Repeater : public Node
{
public:
	/// @param child 子ノード
	/// @param count 繰り返し回数
	Repeater(std::unique_ptr<Node> child, int count)
		: m_child(std::move(child)), m_count(count) {}

	[[nodiscard]] Status tick(Blackboard& bb) override
	{
		for (int i = 0; i < m_count; ++i)
		{
			const Status s = m_child->tick(bb);
			if (s == Status::Running) return Status::Running;
		}
		return Status::Success;
	}

private:
	std::unique_ptr<Node> m_child;
	int m_count;
};

/// @brief Succeederデコレータ — 常にSuccessを返す
class Succeeder : public Node
{
public:
	/// @param child 子ノード
	explicit Succeeder(std::unique_ptr<Node> child)
		: m_child(std::move(child)) {}

	[[nodiscard]] Status tick(Blackboard& bb) override
	{
		[[maybe_unused]] const auto s = m_child->tick(bb);
		return Status::Success;
	}

private:
	std::unique_ptr<Node> m_child;
};

// ── Leaf ──────────────────────────────────────────────────────

/// @brief Actionノード — 関数を実行するリーフ
class Action : public Node
{
public:
	/// @param func 実行する関数
	explicit Action(std::function<Status(Blackboard&)> func)
		: m_func(std::move(func)) {}

	[[nodiscard]] Status tick(Blackboard& bb) override
	{
		return m_func(bb);
	}

private:
	std::function<Status(Blackboard&)> m_func;
};

/// @brief Conditionノード — 条件判定するリーフ
class Condition : public Node
{
public:
	/// @param pred 判定関数（trueでSuccess、falseでFailure）
	explicit Condition(std::function<bool(Blackboard&)> pred)
		: m_pred(std::move(pred)) {}

	[[nodiscard]] Status tick(Blackboard& bb) override
	{
		return m_pred(bb) ? Status::Success : Status::Failure;
	}

private:
	std::function<bool(Blackboard&)> m_pred;
};

// ── Builder ──────────────────────────────────────────────────

/// @brief ビヘイビアツリーのfluent Builder
///
/// @code
/// auto tree = sgc::bt::Builder()
///     .selector()
///         .sequence()
///             .condition([](auto& bb) { return bb.get<int>("hp").value_or(0) > 50; })
///             .action([](auto& bb) { bb.set("state", std::string("attack")); return sgc::bt::Status::Success; })
///         .end()
///         .action([](auto& bb) { bb.set("state", std::string("flee")); return sgc::bt::Status::Success; })
///     .end()
///     .build();
/// @endcode
class Builder
{
public:
	/// @brief Selectorの構築を開始する
	Builder& selector()
	{
		auto node = std::make_unique<Selector>();
		m_stack.push_back({CompositeType::SelectorType, node.get()});
		m_pending.push_back(std::move(node));
		return *this;
	}

	/// @brief Sequenceの構築を開始する
	Builder& sequence()
	{
		auto node = std::make_unique<Sequence>();
		m_stack.push_back({CompositeType::SequenceType, node.get()});
		m_pending.push_back(std::move(node));
		return *this;
	}

	/// @brief 現在のCompositeを閉じる
	Builder& end()
	{
		if (m_stack.empty()) return *this;

		const auto frame = m_stack.back();
		m_stack.pop_back();

		// pendingからこのノードを見つけて取り出す
		std::unique_ptr<Node> compositeNode;
		for (auto it = m_pending.begin(); it != m_pending.end(); ++it)
		{
			if (it->get() == frame.node)
			{
				compositeNode = std::move(*it);
				m_pending.erase(it);
				break;
			}
		}

		if (compositeNode)
		{
			addToParentOrRoot(std::move(compositeNode));
		}

		return *this;
	}

	/// @brief Actionノードを追加する
	/// @param func 実行する関数
	Builder& action(std::function<Status(Blackboard&)> func)
	{
		addToParentOrRoot(std::make_unique<Action>(std::move(func)));
		return *this;
	}

	/// @brief Conditionノードを追加する
	/// @param pred 判定関数
	Builder& condition(std::function<bool(Blackboard&)> pred)
	{
		addToParentOrRoot(std::make_unique<Condition>(std::move(pred)));
		return *this;
	}

	/// @brief Inverterデコレータを追加する（次のノードに適用）
	Builder& inverter()
	{
		m_nextDecorator = DecoratorType::InverterType;
		return *this;
	}

	/// @brief Repeaterデコレータを追加する（次のノードに適用）
	/// @param count 繰り返し回数
	Builder& repeater(int count)
	{
		m_nextDecorator = DecoratorType::RepeaterType;
		m_repeaterCount = count;
		return *this;
	}

	/// @brief ツリーを構築する
	/// @return ルートノード
	[[nodiscard]] std::unique_ptr<Node> build()
	{
		// 閉じていないcompositeを閉じる
		while (!m_stack.empty())
		{
			end();
		}

		if (m_root)
		{
			return std::move(m_root);
		}

		// 単一ノードの場合
		if (!m_pending.empty())
		{
			return std::move(m_pending.front());
		}

		return nullptr;
	}

private:
	enum class CompositeType { SequenceType, SelectorType };
	enum class DecoratorType { None, InverterType, RepeaterType };

	struct StackFrame
	{
		CompositeType type;
		Node* node;
	};

	std::vector<StackFrame> m_stack;
	std::vector<std::unique_ptr<Node>> m_pending;
	std::unique_ptr<Node> m_root;
	DecoratorType m_nextDecorator{DecoratorType::None};
	int m_repeaterCount{0};

	/// @brief ノードを親Compositeまたはルートに追加する
	void addToParentOrRoot(std::unique_ptr<Node> node)
	{
		// デコレータが設定されていたら適用
		if (m_nextDecorator == DecoratorType::InverterType)
		{
			node = std::make_unique<Inverter>(std::move(node));
			m_nextDecorator = DecoratorType::None;
		}
		else if (m_nextDecorator == DecoratorType::RepeaterType)
		{
			node = std::make_unique<Repeater>(std::move(node), m_repeaterCount);
			m_nextDecorator = DecoratorType::None;
		}

		if (!m_stack.empty())
		{
			const auto& frame = m_stack.back();
			if (frame.type == CompositeType::SequenceType)
			{
				static_cast<Sequence*>(frame.node)->addChild(std::move(node));
			}
			else
			{
				static_cast<Selector*>(frame.node)->addChild(std::move(node));
			}
		}
		else
		{
			m_root = std::move(node);
		}
	}
};

} // namespace sgc::bt
