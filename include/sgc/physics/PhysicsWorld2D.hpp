#pragma once

/// @file PhysicsWorld2D.hpp
/// @brief 2D物理ワールドの具象実装
///
/// IPhysicsWorld2Dインターフェースの実装。
/// 重力適用、セミインプリシットオイラー積分、N^2ブロードフェーズのAABB衝突検出を行う。
///
/// @code
/// sgc::physics::PhysicsWorld2Df world;
/// world.setGravity({0.0f, 980.0f});
///
/// sgc::physics::RigidBody2Df player;
/// player.position = {50.0f, 0.0f};
/// player.mass = 1.0f;
/// player.restitution = 0.3f;
/// world.addBody(&player, {16.0f, 16.0f});
///
/// sgc::physics::RigidBody2Df ground;
/// ground.position = {50.0f, 300.0f};
/// ground.isStatic = true;
/// world.addBody(&ground, {200.0f, 16.0f});
///
/// world.step(1.0f / 60.0f);
/// @endcode

#include <algorithm>
#include <vector>

#include "sgc/physics/IPhysicsWorld.hpp"
#include "sgc/physics/AABB2DCollision.hpp"

namespace sgc::physics
{

/// @brief 剛体エントリ（剛体ポインタとAABBハーフサイズのペア）
/// @tparam T 浮動小数点型
template <FloatingPoint T>
struct BodyEntry
{
	RigidBody2D<T>* body{nullptr};  ///< 剛体へのポインタ
	Vec2<T> halfSize{};              ///< AABBの半分のサイズ
};

/// @brief 2D物理ワールドの具象実装
/// @tparam T 浮動小数点型
///
/// step()では以下の順序で処理を行う:
/// 1. 全動的剛体に重力を適用
/// 2. 全剛体を積分（位置・速度更新）
/// 3. N^2衝突検出＋衝突応答
template <FloatingPoint T>
class PhysicsWorld2D final : public IPhysicsWorld2D<T>
{
public:
	/// @brief デフォルトコンストラクタ
	PhysicsWorld2D() = default;

	/// @brief 重力を指定して構築する
	/// @param gravity 重力ベクトル
	explicit PhysicsWorld2D(const Vec2<T>& gravity)
		: m_gravity(gravity)
	{
	}

	/// @brief 剛体をワールドに追加する
	/// @param body 追加する剛体へのポインタ
	/// @param halfSize AABBの半分のサイズ
	void addBody(RigidBody2D<T>* body, const Vec2<T>& halfSize) override
	{
		if (!body) return;
		m_bodies.push_back(BodyEntry<T>{body, halfSize});
	}

	/// @brief 剛体をワールドから除去する
	/// @param body 除去する剛体へのポインタ
	void removeBody(RigidBody2D<T>* body) override
	{
		m_bodies.erase(
			std::remove_if(m_bodies.begin(), m_bodies.end(),
				[body](const BodyEntry<T>& entry) { return entry.body == body; }),
			m_bodies.end());
	}

	/// @brief 物理シミュレーションを1ステップ進める
	/// @param dt タイムステップ幅（秒）
	///
	/// 重力適用 → 積分 → 衝突検出＋応答 の順で処理する。
	void step(T dt) override
	{
		// 1. 全動的剛体に重力を適用
		for (auto& entry : m_bodies)
		{
			if (!entry.body->isStatic)
			{
				entry.body->applyForce(m_gravity * entry.body->mass);
			}
		}

		// 2. 全剛体を積分
		for (auto& entry : m_bodies)
		{
			entry.body->integrate(dt);
		}

		// 3. N^2衝突検出＋応答
		const auto count = m_bodies.size();
		for (std::size_t i = 0; i < count; ++i)
		{
			for (std::size_t j = i + 1; j < count; ++j)
			{
				auto& a = m_bodies[i];
				auto& b = m_bodies[j];

				// 両方静的なら衝突判定不要
				if (a.body->isStatic && b.body->isStatic) continue;

				const auto aabbA = a.body->bounds(a.halfSize);
				const auto aabbB = b.body->bounds(b.halfSize);

				const auto collision = detectAABBCollision(aabbA, aabbB);
				if (collision.colliding)
				{
					resolveRigidBodyCollision(*a.body, *b.body, collision);
				}
			}
		}
	}

	/// @brief 重力ベクトルを設定する
	/// @param gravity 重力ベクトル
	void setGravity(const Vec2<T>& gravity) override
	{
		m_gravity = gravity;
	}

	/// @brief 現在の重力ベクトルを取得する
	/// @return 重力ベクトル
	[[nodiscard]] Vec2<T> gravity() const override
	{
		return m_gravity;
	}

	/// @brief 登録されている剛体数を取得する
	/// @return 剛体数
	[[nodiscard]] std::size_t bodyCount() const override
	{
		return m_bodies.size();
	}

	/// @brief 全剛体をワールドから除去する
	void clear() override
	{
		m_bodies.clear();
	}

	/// @brief 登録済み剛体エントリ一覧を取得する（読み取り専用）
	/// @return 剛体エントリのベクタ参照
	[[nodiscard]] const std::vector<BodyEntry<T>>& bodies() const noexcept
	{
		return m_bodies;
	}

private:
	std::vector<BodyEntry<T>> m_bodies;       ///< 登録済み剛体エントリ
	Vec2<T> m_gravity{T{0}, T{0}};            ///< 重力ベクトル
};

/// @brief float版 PhysicsWorld2D
using PhysicsWorld2Df = PhysicsWorld2D<float>;

} // namespace sgc::physics
