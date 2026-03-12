#pragma once

/// @file RigidBody2D.hpp
/// @brief 2D剛体（簡易物理シミュレーション用）
///
/// 基本的な力・衝撃・ドラッグ・反発を扱う軽量な2D剛体構造体。
/// フレームワーク非依存で、ECSコンポーネントとしても使用可能。
///
/// @code
/// using namespace sgc::physics;
///
/// RigidBody2Df body;
/// body.position = {100.0f, 200.0f};
/// body.mass = 5.0f;
/// body.drag = 0.01f;
///
/// // 重力を適用
/// body.applyForce({0.0f, 9.8f * body.mass});
///
/// // 物理ステップ更新
/// body.integrate(1.0f / 60.0f);
/// @endcode

#include "sgc/physics/AABB2DCollision.hpp"
#include "sgc/physics/CollisionFilter.hpp"

namespace sgc::physics
{

/// @brief 2D剛体（簡易物理シミュレーション用）
/// @tparam T 浮動小数点型
///
/// 位置・速度・加速度のセミインプリシットオイラー積分を行う。
/// integrate()呼び出し後に加速度は自動リセットされる。
template <FloatingPoint T>
struct RigidBody2D
{
	Vec2<T> position{};             ///< 位置
	Vec2<T> velocity{};             ///< 速度
	Vec2<T> acceleration{};         ///< 加速度（毎ステップリセット）
	T mass{static_cast<T>(1)};      ///< 質量
	T drag{};                       ///< 空気抵抗係数 [0, 1]
	T restitution{};                ///< 反発係数 [0, 1]
	bool isStatic{false};           ///< 静的オブジェクトか
	CollisionFilter filter{};       ///< 衝突フィルター（レイヤー＋マスク）

	/// @brief 力を加える（F = ma に基づく）
	/// @param force 適用する力ベクトル
	///
	/// @code
	/// body.applyForce({0.0f, 9.8f * body.mass}); // 重力
	/// @endcode
	constexpr void applyForce(const Vec2<T>& force) noexcept
	{
		if (isStatic || mass <= T{0}) return;
		acceleration = acceleration + force / mass;
	}

	/// @brief 衝撃（瞬間的な速度変化）を加える
	/// @param impulse 衝撃ベクトル
	///
	/// @code
	/// body.applyImpulse({0.0f, -500.0f}); // ジャンプ
	/// @endcode
	constexpr void applyImpulse(const Vec2<T>& impulse) noexcept
	{
		if (isStatic || mass <= T{0}) return;
		velocity = velocity + impulse / mass;
	}

	/// @brief 物理ステップを更新する（セミインプリシットオイラー法）
	/// @param dt タイムステップ幅（秒）
	///
	/// 速度に加速度を加算し、ドラッグを適用した後、位置を更新する。
	/// 加速度は自動的にリセットされる。
	constexpr void integrate(T dt) noexcept
	{
		if (isStatic) return;
		velocity = velocity + acceleration * dt;
		velocity = velocity * (T{1} - drag);
		position = position + velocity * dt;
		acceleration = {};  // 加速度をリセット
	}

	/// @brief AABBを取得する（位置中心、指定ハーフサイズ）
	/// @param halfSize AABBの半分のサイズ
	/// @return 位置を中心としたAABB
	[[nodiscard]] constexpr AABB2<T> bounds(const Vec2<T>& halfSize) const noexcept
	{
		return {position - halfSize, position + halfSize};
	}
};

/// @brief 2つの剛体間の衝突応答を計算する
///
/// 質量比に基づいて分離し、反発係数を用いた衝撃を適用する。
/// 静的オブジェクトは移動しない（質量無限大として扱う）。
///
/// @tparam T 浮動小数点型
/// @param a 1つ目の剛体
/// @param b 2つ目の剛体
/// @param collision detectAABBCollisionの結果
///
/// @code
/// auto collision = detectAABBCollision(a.bounds(halfA), b.bounds(halfB));
/// if (collision.colliding)
/// {
///     resolveRigidBodyCollision(a, b, collision);
/// }
/// @endcode
template <FloatingPoint T>
constexpr void resolveRigidBodyCollision(
	RigidBody2D<T>& a, RigidBody2D<T>& b,
	const CollisionResult2D<T>& collision) noexcept
{
	if (!collision.colliding) return;

	// 両方静的なら何もしない
	if (a.isStatic && b.isStatic) return;

	const Vec2<T>& normal = collision.normal;
	const T penetration = collision.penetration;

	// ── 位置分離（質量比に基づく） ──
	if (a.isStatic)
	{
		// Aが静的 → Bだけ移動
		b.position = b.position + normal * penetration;
	}
	else if (b.isStatic)
	{
		// Bが静的 → Aだけ移動
		a.position = a.position - normal * penetration;
	}
	else
	{
		// 両方動的 → 質量比で分離
		const T totalMass = a.mass + b.mass;
		const T ratioA = b.mass / totalMass;
		const T ratioB = a.mass / totalMass;
		a.position = a.position - normal * (penetration * ratioA);
		b.position = b.position + normal * (penetration * ratioB);
	}

	// ── 速度応答（反発係数ベースの衝撃） ──
	const Vec2<T> relativeVelocity = a.velocity - b.velocity;
	const T velocityAlongNormal = relativeVelocity.dot(normal);

	// 離れる方向に動いていたら衝撃不要
	if (velocityAlongNormal < T{0}) return;

	// 反発係数（小さい方を採用）
	const T e = (a.restitution < b.restitution) ? a.restitution : b.restitution;

	// 逆質量の計算（静的は逆質量0）
	const T invMassA = a.isStatic ? T{0} : T{1} / a.mass;
	const T invMassB = b.isStatic ? T{0} : T{1} / b.mass;
	const T invMassSum = invMassA + invMassB;

	if (invMassSum <= T{0}) return;

	// 衝撃の大きさ
	const T j = -(T{1} + e) * velocityAlongNormal / invMassSum;

	// 衝撃を適用
	const Vec2<T> impulse = normal * j;
	if (!a.isStatic) a.velocity = a.velocity + impulse * invMassA;
	if (!b.isStatic) b.velocity = b.velocity - impulse * invMassB;
}

using RigidBody2Df = RigidBody2D<float>;

} // namespace sgc::physics
