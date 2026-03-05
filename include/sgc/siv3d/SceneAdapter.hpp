#pragma once

/// @file SceneAdapter.hpp
/// @brief Siv3Dゲームループ × sgc::SceneManager 統合
///
/// Siv3Dの `while (System::Update())` ループと
/// sgcのSceneManagerを統合するAppクラス。
/// シーン間で共有データを持ち回ることができる。
///
/// @code
/// struct GameData { int score = 0; s3d::Font font; };
///
/// class TitleScene : public sgc::siv3d::AppScene<GameData> {
/// public:
///     using AppScene::AppScene;
///     void update(float dt) override { /* ... */ }
///     void draw() const override { /* ... */ }
/// };
///
/// void Main() {
///     sgc::siv3d::App<GameData> app;
///     app.addScene<TitleScene>();
///     app.run();
/// }
/// @endcode

#include <Siv3D.hpp>
#include <concepts>
#include <utility>

#include "sgc/core/Timer.hpp"
#include "sgc/scene/SceneManager.hpp"
#include "sgc/siv3d/DrawAdapter.hpp"

namespace sgc::siv3d
{

// 前方宣言
template <typename SharedData>
class App;

/// @brief Appで使用するシーン基底クラス
/// @tparam SharedData シーン間共有データ型
///
/// sgc::Sceneを継承しつつ、共有データへのアクセスを提供する。
/// 各シーンはこのクラスを継承する。
template <typename SharedData>
class AppScene : public sgc::Scene
{
public:
	/// @brief コンストラクタ
	/// @param app アプリケーションへの参照
	explicit AppScene(App<SharedData>& app) : m_app(app) {}

	/// @brief 仮想デストラクタ
	~AppScene() override = default;

	/// @brief 共有データへの参照を取得する
	/// @return 共有データ
	[[nodiscard]] SharedData& getData() noexcept { return m_app.getData(); }

	/// @brief 共有データへのconst参照を取得する
	/// @return 共有データ（読み取り専用）
	[[nodiscard]] const SharedData& getData() const noexcept { return m_app.getData(); }

	/// @brief シーンマネージャへの参照を取得する（シーン遷移用）
	/// @return シーンマネージャ
	[[nodiscard]] SceneManager& getSceneManager() noexcept { return m_app.getSceneManager(); }

protected:
	App<SharedData>& m_app;  ///< アプリケーション参照
};

/// @brief Siv3Dアプリケーションクラス
/// @tparam SharedData シーン間で共有するデータ型
///
/// Siv3Dのメインループとsgcのシーンマネージャを統合する。
/// addScene()で最初のシーンを登録し、run()でゲームループを開始する。
template <typename SharedData>
class App
{
public:
	/// @brief コンストラクタ
	/// @param data 共有データの初期値
	explicit App(const SharedData& data = {})
		: m_sharedData(data)
	{
	}

	/// @brief シーンを登録してスタックに積む
	/// @tparam S AppScene<SharedData>派生のシーンクラス
	template <typename S>
		requires std::derived_from<S, AppScene<SharedData>>
	void addScene()
	{
		m_sceneManager.template push<S>(*this);
	}

	/// @brief メインループを実行する
	///
	/// Siv3Dの `while (System::Update())` を内包し、
	/// 毎フレームSceneManagerのupdate()とdraw()を呼び出す。
	/// シーンスタックが空になるか、Siv3Dが終了するまでループする。
	void run()
	{
		m_clock.reset();

		while (s3d::System::Update())
		{
			const float dt = m_clock.tick();

			// シーン更新
			m_sceneManager.update(dt);

			// シーンが空になったら終了
			if (m_sceneManager.empty())
			{
				break;
			}

			// シーン描画
			m_sceneManager.draw();

			// フェードオーバーレイ描画
			const auto fadeState = m_sceneManager.fadeState();
			if (fadeState != FadeState::None)
			{
				const float progress = m_sceneManager.getFadeProgress();
				const float alpha = (fadeState == FadeState::FadingOut) ? progress : (1.0f - progress);
				drawFadeOverlay(alpha);
			}
		}
	}

	/// @brief 共有データへの参照を取得する
	/// @return 共有データ
	[[nodiscard]] SharedData& getData() noexcept { return m_sharedData; }

	/// @brief 共有データへのconst参照を取得する
	/// @return 共有データ（読み取り専用）
	[[nodiscard]] const SharedData& getData() const noexcept { return m_sharedData; }

	/// @brief シーンマネージャへの参照を取得する
	/// @return シーンマネージャ
	[[nodiscard]] SceneManager& getSceneManager() noexcept { return m_sceneManager; }

	/// @brief シーンマネージャへのconst参照を取得する
	/// @return シーンマネージャ（読み取り専用）
	[[nodiscard]] const SceneManager& getSceneManager() const noexcept { return m_sceneManager; }

	/// @brief デルタクロックへの参照を取得する（タイムスケール調整用）
	/// @return デルタクロック
	[[nodiscard]] DeltaClock& getClock() noexcept { return m_clock; }

private:
	SceneManager m_sceneManager;   ///< シーンマネージャ
	SharedData m_sharedData;       ///< シーン間共有データ
	DeltaClock m_clock;            ///< フレーム間デルタタイム計測
};

} // namespace sgc::siv3d
