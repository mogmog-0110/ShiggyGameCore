#pragma once

/// @file Siv3DTextMeasure.hpp
/// @brief ITextMeasure の Siv3D 実装
///
/// Siv3DのFontを使用してテキストサイズを計測する。
///
/// @note このファイルはSiv3Dに依存するため、CI対象外。

#include <Siv3D.hpp>

#include "sgc/graphics/ITextMeasure.hpp"
#include "sgc/siv3d/TypeConvert.hpp"

namespace sgc::siv3d
{

/// @brief ITextMeasure の Siv3D 実装
///
/// Siv3Dの `s3d::Font` を保持し、テキスト計測を行う。
/// フォントサイズ別のFont切り替えは利用者の責任。
class Siv3DTextMeasure : public ITextMeasure
{
public:
	/// @brief フォントを指定して構築する
	/// @param font Siv3Dフォント
	explicit Siv3DTextMeasure(const s3d::Font& font)
		: m_font(font) {}

	/// @brief テキストの描画サイズを計測する
	[[nodiscard]] Vec2f measure(
		std::string_view text, float /*fontSize*/) const override
	{
		const auto region = m_font(s3d::Unicode::FromUTF8(text)).region();
		return fromSiv(region.size);
	}

	/// @brief 1行のテキストの高さを取得する
	[[nodiscard]] float lineHeight(float /*fontSize*/) const override
	{
		return static_cast<float>(m_font.height());
	}

private:
	s3d::Font m_font;  ///< Siv3Dフォント
};

} // namespace sgc::siv3d
