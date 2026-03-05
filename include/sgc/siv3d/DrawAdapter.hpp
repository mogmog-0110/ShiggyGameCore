#pragma once

/// @file DrawAdapter.hpp
/// @brief sgc型を使ったSiv3D描画ラッパー
///
/// sgcの型（Vec2f, AABB2f, Colorf等）を直接使って
/// Siv3Dの描画関数を呼び出せるようにする薄いラッパー群。
///
/// @code
/// sgc::AABB2f rect{{10, 10}, {100, 60}};
/// sgc::siv3d::drawRect(rect, sgc::Colorf::red());
/// sgc::siv3d::drawCircle({400, 300}, 20.0f, sgc::Colorf::blue());
/// @endcode

#include <Siv3D.hpp>

#include "sgc/math/Geometry.hpp"
#include "sgc/math/Vec2.hpp"
#include "sgc/siv3d/TypeConvert.hpp"
#include "sgc/types/Color.hpp"

namespace sgc::siv3d
{

// ── 矩形描画 ──────────────────────────────────────────

/// @brief 塗りつぶし矩形を描画する
/// @param rect 矩形（sgc::AABB2f）
/// @param color 塗りつぶし色
inline void drawRect(const AABB2f& rect, const Colorf& color)
{
	toSivRect(rect).draw(toSivColorF(color));
}

/// @brief 矩形の枠線を描画する
/// @param rect 矩形（sgc::AABB2f）
/// @param thickness 線の太さ（ピクセル）
/// @param color 線の色
inline void drawRectFrame(const AABB2f& rect, float thickness, const Colorf& color)
{
	toSivRect(rect).drawFrame(static_cast<double>(thickness), toSivColorF(color));
}

/// @brief 角丸矩形を描画する
/// @param rect 矩形（sgc::AABB2f）
/// @param round 角の丸み半径
/// @param color 塗りつぶし色
inline void drawRoundRect(const AABB2f& rect, float round, const Colorf& color)
{
	toSivRect(rect).rounded(static_cast<double>(round)).draw(toSivColorF(color));
}

// ── 円描画 ─────────────────────────────────────────────

/// @brief 塗りつぶし円を描画する
/// @param center 中心座標
/// @param radius 半径
/// @param color 塗りつぶし色
inline void drawCircle(const Vec2f& center, float radius, const Colorf& color)
{
	s3d::Circle{toSiv(center), static_cast<double>(radius)}.draw(toSivColorF(color));
}

/// @brief 円の枠線を描画する
/// @param center 中心座標
/// @param radius 半径
/// @param thickness 線の太さ
/// @param color 線の色
inline void drawCircleFrame(const Vec2f& center, float radius, float thickness, const Colorf& color)
{
	s3d::Circle{toSiv(center), static_cast<double>(radius)}
		.drawFrame(static_cast<double>(thickness), toSivColorF(color));
}

// ── 線描画 ─────────────────────────────────────────────

/// @brief 線分を描画する
/// @param from 始点
/// @param to 終点
/// @param thickness 線の太さ
/// @param color 線の色
inline void drawLine(const Vec2f& from, const Vec2f& to, float thickness, const Colorf& color)
{
	s3d::Line{toSiv(from), toSiv(to)}.draw(static_cast<double>(thickness), toSivColorF(color));
}

// ── 三角形描画 ─────────────────────────────────────────

/// @brief 塗りつぶし三角形を描画する
/// @param p0 頂点0
/// @param p1 頂点1
/// @param p2 頂点2
/// @param color 塗りつぶし色
inline void drawTriangle(const Vec2f& p0, const Vec2f& p1, const Vec2f& p2, const Colorf& color)
{
	s3d::Triangle{toSiv(p0), toSiv(p1), toSiv(p2)}.draw(toSivColorF(color));
}

// ── テキスト描画 ───────────────────────────────────────

/// @brief テキストを描画する
/// @param font Siv3Dのフォント
/// @param text 表示テキスト
/// @param pos 描画位置（左上）
/// @param color テキスト色
inline void drawText(
	const s3d::Font& font,
	const s3d::String& text,
	const Vec2f& pos,
	const Colorf& color)
{
	font(text).draw(toSiv(pos), toSivColorF(color));
}

/// @brief テキストを中央揃えで描画する
/// @param font Siv3Dのフォント
/// @param text 表示テキスト
/// @param center 中央座標
/// @param color テキスト色
inline void drawTextCentered(
	const s3d::Font& font,
	const s3d::String& text,
	const Vec2f& center,
	const Colorf& color)
{
	font(text).drawAt(toSiv(center), toSivColorF(color));
}

// ── オーバーレイ・背景 ────────────────────────────────

/// @brief フェード用の半透明オーバーレイを描画する
/// @param alpha 不透明度 [0,1]（0=透明、1=完全遮蔽）
/// @param color ベース色（デフォルト: 黒）
inline void drawFadeOverlay(float alpha, const Colorf& color = Colorf::black())
{
	const auto overlayColor = color.withAlpha(alpha);
	s3d::Scene::Rect().draw(toSivColorF(overlayColor));
}

/// @brief 背景色をクリアする
/// @param color 背景色
inline void clearBackground(const Colorf& color)
{
	s3d::Scene::SetBackground(toSivColorF(color));
}

} // namespace sgc::siv3d
