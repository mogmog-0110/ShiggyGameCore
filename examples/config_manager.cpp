/// @file config_manager.cpp
/// @brief ConfigManagerでゲーム設定の保存・読込を実演するサンプル

#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>

#include "sgc/config/ConfigManager.hpp"
#include "sgc/core/JsonReader.hpp"

/// @brief 設定内容を見やすく表示する
void printConfig(const sgc::ConfigManager& config, const std::string& label)
{
	std::cout << "=== " << label << " ===" << std::endl;
	std::cout << "  volume      : " << config.getFloatOr("volume", -1.0f) << std::endl;
	std::cout << "  fullscreen  : " << (config.getBoolOr("fullscreen", false) ? "true" : "false") << std::endl;
	std::cout << "  playerName  : " << config.getStringOr("playerName", "(none)") << std::endl;
	std::cout << "  difficulty  : " << config.getIntOr("difficulty", -1) << std::endl;
	std::cout << "  vsync       : " << (config.getBoolOr("vsync", false) ? "true" : "false") << std::endl;
	std::cout << "  entries     : " << config.size() << std::endl;
	std::cout << std::endl;
}

int main()
{
	std::cout << "--- ConfigManager Demo ---" << std::endl;
	std::cout << std::endl;

	// ── 1. 設定値の登録 ────────────────────────────────────
	sgc::ConfigManager config;
	config.set("volume", 0.75f);
	config.set("fullscreen", true);
	config.set("playerName", std::string("Shiggy"));
	config.set("difficulty", 3);
	config.set("vsync", true);

	printConfig(config, "Initial Settings");

	// ── 2. デフォルト値付き取得 ─────────────────────────────
	std::cout << "=== Default Value Fallback ===" << std::endl;
	std::cout << "  language (not set, default='ja'): "
		<< config.getStringOr("language", "ja") << std::endl;
	std::cout << "  fov (not set, default=90): "
		<< config.getIntOr("fov", 90) << std::endl;
	std::cout << std::endl;

	// ── 3. hasKey / remove ──────────────────────────────────
	std::cout << "=== hasKey / remove ===" << std::endl;
	std::cout << "  hasKey('vsync')  : "
		<< (config.hasKey("vsync") ? "true" : "false") << std::endl;

	config.remove("vsync");
	std::cout << "  after remove('vsync'): "
		<< (config.hasKey("vsync") ? "true" : "false") << std::endl;
	std::cout << "  vsync fallback   : "
		<< (config.getBoolOr("vsync", false) ? "true" : "false") << std::endl;
	std::cout << std::endl;

	// ── 4. JSONシリアライズ ──────────────────────────────────
	const std::string json = config.toJson();
	std::cout << "=== Serialized JSON ===" << std::endl;
	std::cout << "  " << json << std::endl;
	std::cout << std::endl;

	// ── 5. ファイルへの保存・読み込み ──────────────────────────
	const std::string tempPath = (std::filesystem::temp_directory_path() / "sgc_config_demo.json").string();
	std::cout << "=== Save / Load File ===" << std::endl;
	std::cout << "  temp file: " << tempPath << std::endl;

	/// 保存
	const bool saved = config.saveToFile(tempPath);
	std::cout << "  save result: " << (saved ? "OK" : "FAILED") << std::endl;

	/// 全クリア
	config.clear();
	std::cout << "  after clear: entries=" << config.size() << std::endl;

	/// ファイルから復元
	/// @note ConfigManager::fromJson/loadFromFileは現在キー列挙APIがないため
	///       自動復元は非対応。JsonReaderを直接使用して手動で復元する。
	sgc::JsonReader reader;
	{
		std::ifstream ifs(tempPath);
		const std::string content(
			(std::istreambuf_iterator<char>(ifs)),
			std::istreambuf_iterator<char>());
		reader.fromString(content);
	}

	/// JsonReaderから各キーを手動で読み取り、ConfigManagerに再設定
	float volume = 0.0f;
	reader.read("volume", volume);
	config.set("volume", volume);

	bool fullscreen = false;
	reader.read("fullscreen", fullscreen);
	config.set("fullscreen", fullscreen);

	std::string playerName;
	reader.read("playerName", playerName);
	config.set("playerName", playerName);

	int difficulty = 0;
	reader.read("difficulty", difficulty);
	config.set("difficulty", difficulty);

	printConfig(config, "Restored from File");

	/// 一時ファイルを削除
	std::filesystem::remove(tempPath);
	std::cout << "  temp file cleaned up." << std::endl;

	return 0;
}
