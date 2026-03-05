/// @file serialize_roundtrip.cpp
/// @brief JSONシリアライズ/デシリアライズのサンプル
///
/// Vec2/Colorの変換、配列の書き出しと読み込み、エラーハンドリングを実演する。

#include <iostream>
#include <vector>

#include "sgc/core/Serialize.hpp"
#include "sgc/math/Vec2.hpp"
#include "sgc/types/Color.hpp"

/// @brief ゲーム設定を表すデータ型
struct GameConfig
{
	std::string title;
	int width{0};
	int height{0};
	bool fullscreen{false};

	void visit(sgc::JsonWriter& w) const
	{
		w.write("title", title);
		w.write("width", width);
		w.write("height", height);
		w.write("fullscreen", fullscreen);
	}

	void visit(sgc::JsonReader& r)
	{
		r.read("title", title);
		r.read("width", width);
		r.read("height", height);
		r.read("fullscreen", fullscreen);
	}
};

int main()
{
	// 1. 基本的なシリアライズ/デシリアライズ
	std::cout << "=== Basic roundtrip ===\n";
	GameConfig config;
	config.title = "My Game";
	config.width = 1920;
	config.height = 1080;
	config.fullscreen = true;

	const auto json = sgc::toJson(config);
	std::cout << "JSON: " << json << "\n";

	const auto restored = sgc::fromJson<GameConfig>(json);
	std::cout << "Title: " << restored.title << "\n";
	std::cout << "Size: " << restored.width << "x" << restored.height << "\n";
	std::cout << "Fullscreen: " << (restored.fullscreen ? "yes" : "no") << "\n\n";

	// 2. 数学型のroundtrip
	std::cout << "=== Vec2f roundtrip ===\n";
	sgc::Vec2f pos{42.5f, 128.0f};
	const auto vecJson = sgc::toJson(pos);
	std::cout << "JSON: " << vecJson << "\n";

	const auto restoredPos = sgc::fromJson<sgc::Vec2f>(vecJson);
	std::cout << "Restored: (" << restoredPos.x << ", " << restoredPos.y << ")\n\n";

	// 3. Color型のroundtrip
	std::cout << "=== Colorf roundtrip ===\n";
	sgc::Colorf color{0.2f, 0.8f, 0.5f, 1.0f};
	const auto colorJson = sgc::toJson(color);
	std::cout << "JSON: " << colorJson << "\n";

	const auto restoredColor = sgc::fromJson<sgc::Colorf>(colorJson);
	std::cout << "Restored: RGBA(" << restoredColor.r << ", "
		<< restoredColor.g << ", " << restoredColor.b << ", "
		<< restoredColor.a << ")\n\n";

	// 4. 配列のroundtrip
	std::cout << "=== Vector roundtrip ===\n";
	sgc::JsonWriter arrWriter;
	std::vector<int> scores{100, 85, 92, 78};
	arrWriter.write("scores", scores);
	const auto arrJson = arrWriter.toString();
	std::cout << "JSON: " << arrJson << "\n";

	sgc::JsonReader arrReader;
	arrReader.fromString(arrJson);
	std::vector<int> restoredScores;
	arrReader.read("scores", restoredScores);
	std::cout << "Restored scores:";
	for (const auto s : restoredScores)
	{
		std::cout << " " << s;
	}
	std::cout << "\n\n";

	// 5. Vec2f配列のroundtrip
	std::cout << "=== Vector<Vec2f> roundtrip ===\n";
	sgc::JsonWriter vecArrWriter;
	std::vector<sgc::Vec2f> waypoints{{0.0f, 0.0f}, {10.0f, 5.0f}, {20.0f, 0.0f}};
	vecArrWriter.write("path", waypoints);
	const auto pathJson = vecArrWriter.toString();
	std::cout << "JSON: " << pathJson << "\n";

	sgc::JsonReader vecArrReader;
	vecArrReader.fromString(pathJson);
	std::vector<sgc::Vec2f> restoredPath;
	vecArrReader.read("path", restoredPath);
	for (const auto& wp : restoredPath)
	{
		std::cout << "  (" << wp.x << ", " << wp.y << ")\n";
	}
	std::cout << "\n";

	// 6. fromJsonResultによるエラーハンドリング
	std::cout << "=== Error handling (fromJsonResult) ===\n";
	auto goodResult = sgc::fromJsonResult<GameConfig>(json);
	if (goodResult)
	{
		std::cout << "Success: " << goodResult.value().title << "\n";
	}

	auto badResult = sgc::fromJsonResult<GameConfig>("not valid json");
	if (badResult.hasError())
	{
		std::cout << "Error: " << badResult.error().message << "\n";
	}

	return 0;
}
