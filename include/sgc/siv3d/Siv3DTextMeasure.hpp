#pragma once

/// @file Siv3DTextMeasure.hpp
/// @brief ITextMeasure の Siv3D 実装
///
/// フォントサイズごとにs3d::Fontを管理し、
/// ITextMeasureインターフェースでテキスト計測を行う。
/// 未登録サイズは最近接の登録済みサイズにフォールバックする。
///
/// @note このファイルはSiv3Dに依存するため、CI対象外。
///
/// @code
/// sgc::siv3d::Siv3DTextMeasure textMeasure;
/// textMeasure.registerFont(24, s3d::Typeface::Regular);
/// textMeasure.registerFont(48, s3d::Typeface::Heavy);
///
/// auto size = textMeasure.measure("Hello", 24.0f);
/// float h = textMeasure.lineHeight(24.0f);
/// @endcode

#include <Siv3D.hpp>
#include <cmath>
#include <unordered_map>

#include "sgc/graphics/ITextMeasure.hpp"
#include "sgc/siv3d/TypeConvert.hpp"

namespace sgc::siv3d
{

/// @brief ITextMeasure の Siv3D 実装
///
/// フォントサイズ（int）をキーにフォントを管理する。
/// 事前にregisterFont()でフォントを登録し、measure/lineHeightで計測する。
/// 未登録サイズが要求された場合、最も近い登録済みサイズのフォントを使う。
class Siv3DTextMeasure : public ITextMeasure
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

	/// @brief テキストの描画サイズを計測する
	/// @param text テキスト文字列
	/// @param fontSize フォントサイズ
	/// @return テキストの描画サイズ
	[[nodiscard]] Vec2f measure(
		std::string_view text, float fontSize) const override
	{
		const auto& font = findNearestFont(static_cast<int>(fontSize));
		const auto region = font(s3d::Unicode::FromUTF8(text)).region();
		return fromSiv(region.size);
	}

	/// @brief 1行のテキストの高さを取得する
	/// @param fontSize フォントサイズ
	/// @return 行の高さ
	[[nodiscard]] float lineHeight(float fontSize) const override
	{
		const auto& font = findNearestFont(static_cast<int>(fontSize));
		return static_cast<float>(font.height());
	}

private:
	std::unordered_map<int, s3d::Font> m_fonts;  ///< フォントサイズ→Font マップ

	/// @brief 指定サイズまたは最近接サイズのフォントを取得する
	///
	/// 完全一致があればそれを返す。なければ登録済みサイズの中から
	/// 絶対差が最小のものを選ぶ。フォントが一つも登録されていない場合は
	/// デフォルトフォント（サイズ16）を生成して返す。
	///
	/// @param fontSize 要求フォントサイズ
	/// @return フォントへの参照
	[[nodiscard]] const s3d::Font& findNearestFont(int fontSize) const
	{
		// 完全一致
		const auto it = m_fonts.find(fontSize);
		if (it != m_fonts.end())
		{
			return it->second;
		}

		// 最近接サイズを探索
		if (m_fonts.empty())
		{
			// フォールバック: デフォルトフォントを生成（通常ありえない）
			static const s3d::Font fallback{16, s3d::Typeface::Regular};
			return fallback;
		}

		int bestSize = m_fonts.begin()->first;
		int bestDiff = std::abs(fontSize - bestSize);
		for (const auto& [size, font] : m_fonts)
		{
			const int diff = std::abs(fontSize - size);
			if (diff < bestDiff)
			{
				bestDiff = diff;
				bestSize = size;
			}
		}
		return m_fonts.at(bestSize);
	}
};

} // namespace sgc::siv3d
