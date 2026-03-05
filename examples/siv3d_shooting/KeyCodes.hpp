#pragma once

/// @file KeyCodes.hpp
/// @brief サンプルゲーム用キーコード定数
///
/// フレームワーク非依存のキーコード。
/// Main.cppでSiv3DInputProviderに登録する際にこれらの定数を使用する。

namespace KeyCode
{

constexpr int ENTER = 1;   ///< 決定キー
constexpr int SPACE = 2;   ///< スペースキー
constexpr int LEFT  = 3;   ///< 左矢印
constexpr int RIGHT = 4;   ///< 右矢印
constexpr int UP    = 5;   ///< 上矢印
constexpr int DOWN  = 6;   ///< 下矢印
constexpr int A     = 7;   ///< Aキー
constexpr int D     = 8;   ///< Dキー
constexpr int W     = 9;   ///< Wキー
constexpr int S     = 10;  ///< Sキー
constexpr int Z     = 11;  ///< Zキー

} // namespace KeyCode
