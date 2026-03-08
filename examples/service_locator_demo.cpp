/// @file service_locator_demo.cpp
/// @brief ServiceLocatorでサービスの登録と取得を実演するサンプル
///
/// ServiceLocatorパターンはグローバルシングルトンの代替として、
/// テスト時にサービスの差し替えが容易になる設計パターンである。

#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "sgc/patterns/ServiceLocator.hpp"

// ────────────────────────────────────────────────────────
// サービスインターフェース定義
// ────────────────────────────────────────────────────────

/// @brief オーディオサービスのインターフェース
struct IAudioService
{
	virtual ~IAudioService() = default;
	virtual void play(const std::string& name) = 0;
	virtual void setVolume(float volume) = 0;
	virtual float getVolume() const = 0;
};

/// @brief ログサービスのインターフェース
struct ILogService
{
	virtual ~ILogService() = default;
	virtual void info(const std::string& message) = 0;
	virtual void warn(const std::string& message) = 0;
	virtual void error(const std::string& message) = 0;
};

/// @brief 入力サービスのインターフェース
struct IInputService
{
	virtual ~IInputService() = default;
	virtual bool isKeyDown(const std::string& key) const = 0;
};

// ────────────────────────────────────────────────────────
// 具体的なサービス実装
// ────────────────────────────────────────────────────────

/// @brief SDL風のオーディオサービス実装
class SdlAudioService : public IAudioService
{
public:
	void play(const std::string& name) override
	{
		std::cout << "    [SDL Audio] Playing: " << name
				  << " (volume: " << m_volume << ")\n";
	}

	void setVolume(float volume) override
	{
		m_volume = volume;
	}

	float getVolume() const override
	{
		return m_volume;
	}

private:
	float m_volume = 1.0f;
};

/// @brief テスト用のスタブオーディオサービス（音を鳴らさない）
class NullAudioService : public IAudioService
{
public:
	void play(const std::string& name) override
	{
		std::cout << "    [Null Audio] (muted) " << name << "\n";
		m_playedSounds.push_back(name);
	}

	void setVolume(float volume) override
	{
		m_volume = volume;
	}

	float getVolume() const override
	{
		return m_volume;
	}

	/// @brief テスト用: 再生されたサウンド一覧を返す
	const std::vector<std::string>& playedSounds() const
	{
		return m_playedSounds;
	}

private:
	float m_volume = 0.0f;
	std::vector<std::string> m_playedSounds;
};

/// @brief コンソール出力のログサービス
class ConsoleLogService : public ILogService
{
public:
	void info(const std::string& message) override
	{
		std::cout << "    [INFO]  " << message << "\n";
	}

	void warn(const std::string& message) override
	{
		std::cout << "    [WARN]  " << message << "\n";
	}

	void error(const std::string& message) override
	{
		std::cout << "    [ERROR] " << message << "\n";
	}
};

/// @brief ダミーの入力サービス
class DummyInputService : public IInputService
{
public:
	bool isKeyDown(const std::string& key) const override
	{
		/// デモ用: "Space" だけ押されている扱い
		return key == "Space";
	}
};

// ────────────────────────────────────────────────────────
// サービスを利用するゲームロジック
// ────────────────────────────────────────────────────────

/// @brief ServiceLocatorを使ってサービスにアクセスするゲームシステム例
void simulateGameFrame(const sgc::ServiceLocator& locator)
{
	auto& audio = locator.get<IAudioService>();
	auto& log = locator.get<ILogService>();

	log.info("Game frame started");
	audio.play("bgm_battle");

	if (locator.has<IInputService>())
	{
		const auto& input = locator.get<IInputService>();
		if (input.isKeyDown("Space"))
		{
			log.info("Player pressed Space -> Attack!");
			audio.play("sfx_sword");
		}
	}
	else
	{
		log.warn("InputService not available");
	}
}

// ────────────────────────────────────────────────────────
// メイン
// ────────────────────────────────────────────────────────

int main()
{
	std::cout << "=== sgc ServiceLocator Demo ===\n\n";

	sgc::ServiceLocator locator;

	// ── 1. サービスの登録 ───────────────────────────────
	std::cout << "[1] Register services:\n";
	{
		locator.provide<IAudioService>(std::make_shared<SdlAudioService>());
		locator.provide<ILogService>(std::make_shared<ConsoleLogService>());
		locator.provide<IInputService>(std::make_shared<DummyInputService>());

		std::cout << "  IAudioService registered: "
				  << (locator.has<IAudioService>() ? "yes" : "no") << "\n";
		std::cout << "  ILogService registered:   "
				  << (locator.has<ILogService>() ? "yes" : "no") << "\n";
		std::cout << "  IInputService registered: "
				  << (locator.has<IInputService>() ? "yes" : "no") << "\n";
	}
	std::cout << "\n";

	// ── 2. サービスの使用 ───────────────────────────────
	std::cout << "[2] Use services (SDL Audio):\n";
	{
		auto& audio = locator.get<IAudioService>();
		audio.setVolume(0.8f);
		simulateGameFrame(locator);
	}
	std::cout << "\n";

	// ── 3. サービスの差し替え（テスト用途など） ─────────
	std::cout << "[3] Replace IAudioService with NullAudioService:\n";
	{
		auto nullAudio = std::make_shared<NullAudioService>();
		locator.provide<IAudioService>(nullAudio);

		std::cout << "  (Audio service replaced)\n";
		simulateGameFrame(locator);

		/// テスト用: NullAudioServiceで再生された音を確認
		std::cout << "  Sounds played via NullAudio: ";
		for (const auto& s : nullAudio->playedSounds())
		{
			std::cout << "\"" << s << "\" ";
		}
		std::cout << "\n";
	}
	std::cout << "\n";

	// ── 4. サービスの削除 ───────────────────────────────
	std::cout << "[4] Remove IInputService:\n";
	{
		locator.remove<IInputService>();
		std::cout << "  IInputService registered: "
				  << (locator.has<IInputService>() ? "yes" : "no") << "\n";
		std::cout << "  Running game frame without InputService:\n";
		simulateGameFrame(locator);
	}
	std::cout << "\n";

	// ── 5. 未登録サービスへのアクセス（例外処理） ───────
	std::cout << "[5] Access unregistered service (exception handling):\n";
	{
		try
		{
			(void)locator.get<IInputService>();
		}
		catch (const std::runtime_error& e)
		{
			std::cout << "  Caught exception: " << e.what() << "\n";
		}
	}
	std::cout << "\n";

	// ── 6. 全サービスのクリア ───────────────────────────
	std::cout << "[6] Clear all services:\n";
	{
		locator.clear();
		std::cout << "  IAudioService: "
				  << (locator.has<IAudioService>() ? "yes" : "no") << "\n";
		std::cout << "  ILogService:   "
				  << (locator.has<ILogService>() ? "yes" : "no") << "\n";
		std::cout << "  All services cleared.\n";
	}

	std::cout << "\n=== Demo Complete ===\n";
	return 0;
}
