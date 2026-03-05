#pragma once

/// @file EventDispatcher.hpp
/// @brief 型安全なイベントディスパッチャー
///
/// イベント型をキーとして、リスナーを登録・発火するシステム。
/// 仮想関数を使わず、std::function + std::type_indexで実現する。

#include <any>
#include <cstdint>
#include <functional>
#include <typeindex>
#include <unordered_map>
#include <vector>

namespace sgc
{

/// @brief イベントリスナーの登録ID（解除に使用）
using ListenerId = std::uint64_t;

/// @brief 型安全なイベントディスパッチャー
///
/// 任意のイベント型に対してリスナーを登録し、イベント発火時に通知する。
///
/// @code
/// struct DamageEvent { int amount; };
/// struct HealEvent { int amount; };
///
/// sgc::EventDispatcher dispatcher;
///
/// auto id = dispatcher.on<DamageEvent>([](const DamageEvent& e) {
///     // ダメージ処理
/// });
///
/// dispatcher.emit(DamageEvent{50});
/// dispatcher.off(id);
/// @endcode
class EventDispatcher
{
public:
	/// @brief イベントリスナーを登録する
	/// @tparam EventType イベントの型
	/// @param callback イベント受信時に呼ばれるコールバック
	/// @return リスナーID（off()で解除に使用）
	template <typename EventType>
	ListenerId on(std::function<void(const EventType&)> callback)
	{
		const auto id = m_nextId++;
		const auto typeIdx = std::type_index(typeid(EventType));

		m_listeners[typeIdx].push_back({
			id,
			[cb = std::move(callback)](const std::any& event)
			{
				cb(std::any_cast<const EventType&>(event));
			}
		});

		return id;
	}

	/// @brief リスナーを解除する
	/// @param id on()で取得したリスナーID
	void off(ListenerId id)
	{
		for (auto& [typeIdx, entries] : m_listeners)
		{
			auto it = std::remove_if(entries.begin(), entries.end(),
				[id](const ListenerEntry& entry) { return entry.id == id; });
			entries.erase(it, entries.end());
		}
	}

	/// @brief イベントを発火する
	///
	/// コールバック内でoff()が呼ばれても安全（リスナーリストをコピーしてからディスパッチ）。
	///
	/// @tparam EventType イベントの型
	/// @param event 発火するイベント
	template <typename EventType>
	void emit(const EventType& event)
	{
		const auto typeIdx = std::type_index(typeid(EventType));
		const auto it = m_listeners.find(typeIdx);
		if (it == m_listeners.end()) return;

		// コピーしてからディスパッチ（コールバック内でoff()されても安全）
		const auto listeners = it->second;
		for (const auto& entry : listeners)
		{
			entry.callback(event);
		}
	}

	/// @brief 指定イベント型のリスナーを全て解除する
	/// @tparam EventType イベントの型
	template <typename EventType>
	void clearListeners()
	{
		m_listeners.erase(std::type_index(typeid(EventType)));
	}

	/// @brief 全リスナーを解除する
	void clearAll()
	{
		m_listeners.clear();
	}

private:
	/// @brief リスナーエントリ（型消去済み）
	struct ListenerEntry
	{
		ListenerId id;
		std::function<void(const std::any&)> callback;
	};

	std::unordered_map<std::type_index, std::vector<ListenerEntry>> m_listeners;
	ListenerId m_nextId = 1;
};

} // namespace sgc
