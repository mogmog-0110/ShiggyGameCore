#pragma once

/// @file Siv3DTextRenderer.hpp
/// @brief ITextRenderer の Siv3D 実装
///
/// フォントサイズごとにs3d::Fontを管理し、
/// ITextRendererインターフェースでテキスト描画を行う。
///
/// @note このファイルはSiv3Dに依存するため、CI対象外。
///
/// @code
/// sgc::siv3d::Siv3DTextRenderer textRenderer;
/// textRenderer.registerFont(60, s3d::Typeface::Heavy);
/// textRenderer.registerFont(24, s3d::Typeface::Regular);
///
/// sgc::ITextRenderer& r = textRenderer;
/// r.drawText("Score: 100", 24.0f, {10, 10}, sgc::Colorf::white());
/// r.drawTextCentered("TITLE", 60.0f, {400, 300}, sgc::Colorf::white());
/// @endcode

#include <Siv3D.hpp>
#include <stdexcept>
#include <unordered_map>

#include "sgc/graphics/ITextRenderer.hpp"
#include "sgc/siv3d/TypeConvert.hpp"

namespace sgc::siv3d
{

/// @brief ITextRenderer の Siv3D 実装
///
/// フォントサイズ（int）をキーにフォントを管理する。
/// 事前にregisterFont()でフォントを登録し、drawText/drawTextCenteredで描画する。
class Siv3DTextRenderer : public ITextRenderer
{
public:
	/// @brief フォントをタイプフェース指定で登録する
	/// @param fontSize フォントサイズ（ピクセル）
	/// @param typeface Siv3Dタイプフェース
	void registerFont(int fontSize, s3d::Typeface typeface = s3d::Typeface::Regular)
	{
		m_fonts.insert_or_assign(fontSize, s3d::Font{fontSize, typeface});
	}

	/// @brief 既存のフォントを登録する
	/// @param fontSize フォントサイズ（ピクセル）
	/// @param font Siv3Dフォント
	void registerFont(int fontSize, const s3d::Font& font)
	{
		m_fonts.insert_or_assign(fontSize, font);
	}

	/// @brief テキストを左上基準で描画する
	void drawText(
		std::string_view text, float fontSize,
		const Vec2f& pos, const Colorf& color) override
	{
		const auto& font = getFont(static_cast<int>(fontSize));
		font(s3d::Unicode::FromUTF8(text)).draw(toSiv(pos), toSivColorF(color));
	}

	/// @brief テキストを中央揃えで描画する
	void drawTextCentered(
		std::string_view text, float fontSize,
		const Vec2f& center, const Colorf& color) override
	{
		const auto& font = getFont(static_cast<int>(fontSize));
		font(s3d::Unicode::FromUTF8(text)).drawAt(toSiv(center), toSivColorF(color));
	}

private:
	std::unordered_map<int, s3d::Font> m_fonts;  ///< フォントサイズ→Font マップ

	/// @brief 指定サイズのフォントを取得する
	/// @param fontSize フォントサイズ
	/// @return フォントへの参照
	/// @throw std::out_of_range 未登録のサイズの場合
	[[nodiscard]] const s3d::Font& getFont(int fontSize) const
	{
		const auto it = m_fonts.find(fontSize);
		if (it == m_fonts.end())
		{
			throw std::out_of_range("Siv3DTextRenderer: font not registered for size");
		}
		return it->second;
	}
};

} // namespace sgc::siv3d
