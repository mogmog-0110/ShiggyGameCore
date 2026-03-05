#pragma once

/// @file InputAdapter.hpp
/// @brief Siv3D入力 → sgc::ActionMap ブリッジ
///
/// Siv3Dのキー・マウス入力をsgcのActionMapに接続する。
/// 毎フレームupdate()を呼ぶことで、Siv3Dのキー状態を
/// ActionMapに反映する。
///
/// @code
/// sgc::ActionMap actions;
/// sgc::siv3d::InputAdapter input;
/// input.bind(s3d::KeySpace, "jump"_hash);
/// input.bind(s3d::KeyA, "moveLeft"_hash);
///
/// // 毎フレーム
/// input.update(actions);
/// if (actions.isPressed("jump"_hash)) { /* ジャンプ */ }
/// @endcode

#include <Siv3D.hpp>
#include <vector>

#include "sgc/input/ActionMap.hpp"
#include "sgc/siv3d/TypeConvert.hpp"

namespace sgc::siv3d
{

/// @brief Siv3D入力 → ActionMap ブリッジ
///
/// Siv3DのInputオブジェクトをsgcのActionMapに接続し、
/// 毎フレームのキー状態を橋渡しする。
class InputAdapter
{
public:
	/// @brief Siv3Dキーをアクションにバインドする
	/// @param key Siv3Dの入力キー
	/// @param action アクションID
	void bind(const s3d::Input& key, ActionId action)
	{
		m_keyBindings.push_back({key, action});
	}

	/// @brief 毎フレームの入力更新
	///
	/// Siv3Dのキー状態を収集し、ActionMapに反映する。
	/// Siv3Dのメインループ内で毎フレーム呼び出す。
	///
	/// @param actionMap 更新対象のアクションマップ
	void update(ActionMap& actionMap) const
	{
		std::vector<int> pressedKeys;
		pressedKeys.reserve(m_keyBindings.size());

		for (const auto& binding : m_keyBindings)
		{
			if (binding.key.pressed())
			{
				pressedKeys.push_back(static_cast<int>(binding.key.code()));
			}
		}

		actionMap.update(pressedKeys);
	}

	/// @brief 初回セットアップ時にActionMapへキーコードを登録する
	///
	/// bind()で追加されたバインディングのキーコードを
	/// ActionMapにも登録する。update()の前に1回呼ぶ。
	///
	/// @param actionMap 登録対象のアクションマップ
	void setupBindings(ActionMap& actionMap) const
	{
		for (const auto& binding : m_keyBindings)
		{
			actionMap.bind(binding.action, static_cast<int>(binding.key.code()));
		}
	}

	/// @brief マウスカーソル座標を sgc::Vec2f で取得する
	/// @return マウス座標（float精度）
	[[nodiscard]] static Vec2f mousePos()
	{
		return fromSiv(s3d::Cursor::PosF());
	}

	/// @brief マウスカーソルの前フレームからの移動量を取得する
	/// @return マウス移動量（float精度）
	[[nodiscard]] static Vec2f mouseDelta()
	{
		return fromSiv(s3d::Cursor::DeltaF());
	}

	/// @brief 全バインディングをクリアする
	void clear()
	{
		m_keyBindings.clear();
	}

private:
	/// @brief キーバインディング情報
	struct KeyBinding
	{
		s3d::Input key;      ///< Siv3Dの入力キー
		ActionId action;     ///< 対応するアクションID
	};

	std::vector<KeyBinding> m_keyBindings;  ///< キーバインディング一覧
};

} // namespace sgc::siv3d
