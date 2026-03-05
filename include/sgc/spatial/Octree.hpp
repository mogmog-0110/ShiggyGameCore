#pragma once

/// @file Octree.hpp
/// @brief 八分木（Octree）による3D空間分割
///
/// 3Dゲームのブロードフェーズ衝突検出や近傍探索に使用する。
/// Quadtreeの3D拡張版。

#include <array>
#include <cstddef>
#include <vector>

#include "sgc/math/Geometry.hpp"
#include "sgc/math/Frustum.hpp"

namespace sgc
{

/// @brief 八分木
/// @tparam T 浮動小数点型
/// @tparam MaxDepth 最大深度
/// @tparam MaxPerNode ノードあたりの最大要素数
///
/// @code
/// sgc::Octreef ot(sgc::AABB3f{{-100,-100,-100}, {100,100,100}});
/// auto h = ot.insert(sgc::AABB3f{{10,10,10}, {20,20,20}});
/// auto results = ot.queryRange(sgc::AABB3f{{0,0,0}, {50,50,50}});
/// @endcode
template <FloatingPoint T, int MaxDepth = 8, int MaxPerNode = 8>
class Octree
{
public:
	/// @brief 要素ハンドル型
	using Handle = std::size_t;

	/// @brief 全体の境界を指定して構築する
	explicit Octree(const AABB3<T>& bounds)
	{
		m_nodes.push_back(Node{bounds});
	}

	/// @brief 要素を挿入する
	Handle insert(const AABB3<T>& bounds)
	{
		const Handle handle = m_elements.size();
		m_elements.push_back({bounds, true});
		insertIntoNode(0, handle);
		return handle;
	}

	/// @brief 要素を削除する
	void remove(Handle handle)
	{
		if (handle < m_elements.size())
		{
			m_elements[handle].active = false;
		}
	}

	/// @brief 要素の位置を更新する
	void update(Handle handle, const AABB3<T>& newBounds)
	{
		if (handle < m_elements.size())
		{
			m_elements[handle].bounds = newBounds;
			m_elements[handle].active = true;
		}
	}

	/// @brief 範囲内の要素を検索する
	[[nodiscard]] std::vector<Handle> queryRange(const AABB3<T>& range) const
	{
		std::vector<Handle> results;
		queryRangeImpl(0, range, results);
		return results;
	}

	/// @brief 点を含む要素を検索する
	[[nodiscard]] std::vector<Handle> queryPoint(const Vec3<T>& point) const
	{
		std::vector<Handle> results;
		const AABB3<T> pointBox{point, point};
		queryRangeImpl(0, pointBox, results);
		return results;
	}

	/// @brief 球内の要素を検索する
	[[nodiscard]] std::vector<Handle> querySphere(const Vec3<T>& center, T radius) const
	{
		const AABB3<T> range{
			{center.x - radius, center.y - radius, center.z - radius},
			{center.x + radius, center.y + radius, center.z + radius}
		};
		auto candidates = queryRange(range);

		std::vector<Handle> results;
		const T r2 = radius * radius;
		for (const auto h : candidates)
		{
			const auto& bounds = m_elements[h].bounds;
			auto clampAxis = [](T val, T minV, T maxV) -> T
			{
				if (val < minV) return minV;
				if (val > maxV) return maxV;
				return val;
			};
			const Vec3<T> closest{
				clampAxis(center.x, bounds.min.x, bounds.max.x),
				clampAxis(center.y, bounds.min.y, bounds.max.y),
				clampAxis(center.z, bounds.min.z, bounds.max.z)
			};
			if ((center - closest).lengthSquared() <= r2)
			{
				results.push_back(h);
			}
		}
		return results;
	}

	/// @brief フラスタム内の要素を検索する
	[[nodiscard]] std::vector<Handle> queryFrustum(const Frustum<T>& frustum) const
	{
		std::vector<Handle> results;
		queryFrustumImpl(0, frustum, results);
		return results;
	}

	/// @brief 全要素を削除する
	void clear()
	{
		const auto bounds = m_nodes[0].bounds;
		m_nodes.clear();
		m_elements.clear();
		m_nodes.push_back(Node{bounds});
	}

	/// @brief アクティブな要素数を返す
	[[nodiscard]] std::size_t size() const noexcept
	{
		std::size_t count = 0;
		for (const auto& e : m_elements)
		{
			if (e.active) ++count;
		}
		return count;
	}

private:
	struct Element
	{
		AABB3<T> bounds;
		bool active;
	};

	struct Node
	{
		AABB3<T> bounds;
		std::vector<Handle> elements;
		std::array<int, 8> children;
		int depth{0};

		explicit Node(const AABB3<T>& b) : bounds(b)
		{
			children.fill(-1);
		}

		[[nodiscard]] bool isLeaf() const noexcept { return children[0] == -1; }
	};

	std::vector<Node> m_nodes;
	std::vector<Element> m_elements;

	void insertIntoNode(int nodeIdx, Handle handle)
	{
		if (m_nodes[nodeIdx].isLeaf())
		{
			m_nodes[nodeIdx].elements.push_back(handle);
			if (static_cast<int>(m_nodes[nodeIdx].elements.size()) > MaxPerNode
				&& m_nodes[nodeIdx].depth < MaxDepth)
			{
				subdivide(nodeIdx);
			}
			return;
		}

		const auto elemBounds = m_elements[handle].bounds;
		for (int i = 0; i < 8; ++i)
		{
			const int childIdx = m_nodes[nodeIdx].children[i];
			if (childIdx != -1 && m_nodes[childIdx].bounds.intersects(elemBounds))
			{
				insertIntoNode(childIdx, handle);
				return;
			}
		}
		m_nodes[nodeIdx].elements.push_back(handle);
	}

	void subdivide(int nodeIdx)
	{
		const auto bounds = m_nodes[nodeIdx].bounds;
		const int depth = m_nodes[nodeIdx].depth;
		const auto c = bounds.center();

		const AABB3<T> octants[8] = {
			{bounds.min, c},
			{{c.x, bounds.min.y, bounds.min.z}, {bounds.max.x, c.y, c.z}},
			{{bounds.min.x, c.y, bounds.min.z}, {c.x, bounds.max.y, c.z}},
			{{c.x, c.y, bounds.min.z}, {bounds.max.x, bounds.max.y, c.z}},
			{{bounds.min.x, bounds.min.y, c.z}, {c.x, c.y, bounds.max.z}},
			{{c.x, bounds.min.y, c.z}, {bounds.max.x, c.y, bounds.max.z}},
			{{bounds.min.x, c.y, c.z}, {c.x, bounds.max.y, bounds.max.z}},
			{c, bounds.max}
		};

		const int firstChild = static_cast<int>(m_nodes.size());
		m_nodes.reserve(m_nodes.size() + 8);

		for (int i = 0; i < 8; ++i)
		{
			m_nodes[nodeIdx].children[i] = firstChild + i;
			Node child(octants[i]);
			child.depth = depth + 1;
			m_nodes.push_back(child);
		}

		auto existing = std::move(m_nodes[nodeIdx].elements);
		m_nodes[nodeIdx].elements.clear();

		for (const auto h : existing)
		{
			if (!m_elements[h].active) continue;
			bool placed = false;
			for (int i = 0; i < 8; ++i)
			{
				const int childIdx = m_nodes[nodeIdx].children[i];
				if (m_nodes[childIdx].bounds.intersects(m_elements[h].bounds))
				{
					m_nodes[childIdx].elements.push_back(h);
					placed = true;
					break;
				}
			}
			if (!placed) m_nodes[nodeIdx].elements.push_back(h);
		}
	}

	void queryRangeImpl(int nodeIdx, const AABB3<T>& range, std::vector<Handle>& results) const
	{
		const auto& node = m_nodes[nodeIdx];
		if (!node.bounds.intersects(range)) return;

		for (const auto h : node.elements)
		{
			if (m_elements[h].active && m_elements[h].bounds.intersects(range))
			{
				results.push_back(h);
			}
		}

		if (!node.isLeaf())
		{
			for (int i = 0; i < 8; ++i)
			{
				if (node.children[i] != -1)
				{
					queryRangeImpl(node.children[i], range, results);
				}
			}
		}
	}

	void queryFrustumImpl(int nodeIdx, const Frustum<T>& frustum, std::vector<Handle>& results) const
	{
		const auto& node = m_nodes[nodeIdx];
		if (!frustum.intersectsAABB(node.bounds)) return;

		for (const auto h : node.elements)
		{
			if (m_elements[h].active && frustum.intersectsAABB(m_elements[h].bounds))
			{
				results.push_back(h);
			}
		}

		if (!node.isLeaf())
		{
			for (int i = 0; i < 8; ++i)
			{
				if (node.children[i] != -1)
				{
					queryFrustumImpl(node.children[i], frustum, results);
				}
			}
		}
	}
};

// ── エイリアス ──────────────────────────────────────────────────

using Octreef = Octree<float>;    ///< float版 Octree
using Octreed = Octree<double>;   ///< double版 Octree

} // namespace sgc
