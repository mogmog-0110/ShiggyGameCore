#pragma once

/// @file View.hpp
/// @brief ECSクエリビュー — 再利用可能なコンポーネントクエリ
///
/// ストレージポインタをキャッシュし、繰り返しのクエリを高速化する。
/// World::view<Components...>() で生成する。
///
/// @note シングルスレッド前提。マルチスレッドでの使用は未サポート。
///
/// @code
/// auto view = world.view<Position, Velocity>();
/// view.each([](Position& pos, Velocity& vel) {
///     pos.x += vel.dx;
///     pos.y += vel.dy;
/// });
/// @endcode

#include <cstddef>
#include <tuple>
#include <vector>

#include "sgc/ecs/ComponentStorage.hpp"
#include "sgc/ecs/Entity.hpp"

namespace sgc::ecs
{

// 前方宣言
template <typename T>
class TypedStorage;

/// @brief 再利用可能なECSクエリビュー
///
/// ストレージポインタをキャッシュして毎フレームのハッシュマップ検索を排除する。
/// 最小ストレージからイテレートし、残りのコンポーネントの存在をチェックする。
///
/// @tparam Components クエリ対象のコンポーネント型
template <typename... Components>
class View
{
public:
	/// @brief ビューを構築する（World::view()から呼ばれる）
	/// @param storages 各コンポーネントのTypedStorageポインタ
	/// @param generations 世代番号配列への参照
	explicit View(std::tuple<TypedStorage<Components>*...> storages,
		const std::vector<Generation>& generations)
		: m_storages(storages)
		, m_generations(generations)
	{
	}

	/// @brief 全てのコンポーネントを持つエンティティに対して関数を実行する
	/// @tparam Func コールバック型 (Components&...)
	/// @param func コールバック関数
	template <typename Func>
	void each(Func&& func)
	{
		if (!isValid()) return;

		// 最初のコンポーネントのストレージをイテレート基準にする
		auto* firstStorage = std::get<0>(m_storages);
		const auto& entities = firstStorage->storage().entities();

		for (std::size_t i = 0; i < entities.size(); ++i)
		{
			const EntityId id = entities[i];
			if (id >= m_generations.size()) continue;

			// 残りのコンポーネントが全て存在するか確認
			if (!hasAllComponents<1>(id)) continue;

			// コールバック実行
			func(*std::get<TypedStorage<Components>*>(m_storages)->storage().get(id)...);
		}
	}

	/// @brief ビューが有効か（全ストレージが存在するか）
	/// @return 全ストレージが非nullならtrue
	[[nodiscard]] bool isValid() const noexcept
	{
		return allStoragesValid();
	}

private:
	std::tuple<TypedStorage<Components>*...> m_storages;
	const std::vector<Generation>& m_generations;

	/// @brief 全ストレージポインタが有効か確認する
	[[nodiscard]] bool allStoragesValid() const noexcept
	{
		return (std::get<TypedStorage<Components>*>(m_storages) && ...);
	}

	/// @brief インデックスI以降のコンポーネントを全て持つか確認する
	template <std::size_t I>
	[[nodiscard]] bool hasAllComponents(EntityId id) const
	{
		if constexpr (I >= sizeof...(Components))
		{
			return true;
		}
		else
		{
			auto* storage = std::get<I>(m_storages);
			if (!storage->storage().has(id)) return false;
			return hasAllComponents<I + 1>(id);
		}
	}
};

} // namespace sgc::ecs
