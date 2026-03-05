#pragma once

/// @file Serialize.hpp
/// @brief 簡易JSONシリアライズ/デシリアライズ
///
/// Visitableコンセプトに基づくビジター方式のシリアライズ。
/// sgc数学型（Vec2, Vec3, Color等）の組み込みサポートを提供する。
/// 配列（JsonArrayWriter / vector<T>）にも対応。

#include <charconv>
#include <cmath>
#include <concepts>
#include <sstream>
#include <string>
#include <string_view>
#include <type_traits>
#include <unordered_map>
#include <vector>

namespace sgc
{

// ── 内部ユーティリティ ────────────────────────────────────────

namespace detail
{

/// @brief 浮動小数点を文字列化する
[[nodiscard]] inline std::string formatFloat(float v)
{
	if (std::isnan(v)) return "null";
	if (std::isinf(v)) return "null";
	std::ostringstream oss;
	oss << v;
	return oss.str();
}

/// @brief 倍精度浮動小数点を文字列化する
[[nodiscard]] inline std::string formatDouble(double v)
{
	if (std::isnan(v)) return "null";
	if (std::isinf(v)) return "null";
	std::ostringstream oss;
	oss << v;
	return oss.str();
}

/// @brief 文字列をJSONエスケープする
[[nodiscard]] inline std::string escapeString(const std::string& s)
{
	std::string result;
	result.reserve(s.size());
	for (const char c : s)
	{
		switch (c)
		{
		case '"': result += "\\\""; break;
		case '\\': result += "\\\\"; break;
		case '\n': result += "\\n"; break;
		case '\r': result += "\\r"; break;
		case '\t': result += "\\t"; break;
		default: result += c; break;
		}
	}
	return result;
}

} // namespace detail

// ── 前方宣言 ────────────────────────────────────────────────

class JsonWriter;
class JsonReader;

// ── Visitable コンセプト ────────────────────────────────────────

/// @brief visit(visitor)メソッドを持つ型
template <typename T, typename Visitor>
concept Visitable = requires(T t, Visitor v) {
	t.visit(v);
};

/// @brief const visit(visitor)メソッドを持つ型
template <typename T, typename Visitor>
concept ConstVisitable = requires(const T t, Visitor v) {
	t.visit(v);
};

// ── JsonArrayWriter ────────────────────────────────────────────

/// @brief JSON配列シリアライザ
///
/// @code
/// sgc::JsonArrayWriter arr;
/// arr.push(1);
/// arr.push(2);
/// arr.push(3);
/// std::string json = arr.toString(); // "[1,2,3]"
/// @endcode
class JsonArrayWriter
{
public:
	/// @brief 整数値を追加する
	void push(int value)
	{
		m_elements.push_back(std::to_string(value));
	}

	/// @brief 浮動小数点値を追加する
	void push(float value)
	{
		m_elements.push_back(detail::formatFloat(value));
	}

	/// @brief 倍精度浮動小数点値を追加する
	void push(double value)
	{
		m_elements.push_back(detail::formatDouble(value));
	}

	/// @brief 真偽値を追加する
	void push(bool value)
	{
		m_elements.push_back(value ? "true" : "false");
	}

	/// @brief 文字列を追加する
	void push(const std::string& value)
	{
		m_elements.push_back("\"" + detail::escapeString(value) + "\"");
	}

	/// @brief ネストされたオブジェクトを追加する
	void push(const JsonWriter& nested);

	/// @brief JSON配列文字列を生成する
	/// @return JSON配列文字列（例: "[1,2,3]"）
	[[nodiscard]] std::string toString() const
	{
		std::string result = "[";
		for (std::size_t i = 0; i < m_elements.size(); ++i)
		{
			if (i > 0) result += ",";
			result += m_elements[i];
		}
		result += "]";
		return result;
	}

private:
	std::vector<std::string> m_elements;
};

// ── JsonWriter ──────────────────────────────────────────────────

/// @brief JSONシリアライザ（ビジター実装）
///
/// @code
/// sgc::JsonWriter w;
/// w.write("name", std::string("player"));
/// w.write("x", 1.5f);
/// std::string json = w.toString();
/// @endcode
class JsonWriter
{
public:
	/// @brief 整数値を書き込む
	void write(const std::string& name, int value)
	{
		m_fields.push_back({name, std::to_string(value)});
	}

	/// @brief 浮動小数点値を書き込む
	void write(const std::string& name, float value)
	{
		m_fields.push_back({name, detail::formatFloat(value)});
	}

	/// @brief 倍精度浮動小数点値を書き込む
	void write(const std::string& name, double value)
	{
		m_fields.push_back({name, detail::formatDouble(value)});
	}

	/// @brief 真偽値を書き込む
	void write(const std::string& name, bool value)
	{
		m_fields.push_back({name, value ? "true" : "false"});
	}

	/// @brief 文字列を書き込む
	void write(const std::string& name, const std::string& value)
	{
		m_fields.push_back({name, "\"" + detail::escapeString(value) + "\""});
	}

	/// @brief オブジェクトを書き込む（ネスト）
	void write(const std::string& name, const JsonWriter& nested)
	{
		m_fields.push_back({name, nested.toString()});
	}

	/// @brief 配列を書き込む
	void write(const std::string& name, const JsonArrayWriter& arr)
	{
		m_fields.push_back({name, arr.toString()});
	}

	/// @brief プリミティブ型のvectorを書き込む
	/// @tparam T int, float, double のいずれか
	template <typename T>
		requires (std::same_as<T, int> || std::same_as<T, float> || std::same_as<T, double>)
	void write(const std::string& name, const std::vector<T>& vec)
	{
		JsonArrayWriter arr;
		for (const auto& v : vec)
		{
			arr.push(v);
		}
		m_fields.push_back({name, arr.toString()});
	}

	/// @brief ConstVisitable型を書き込む（汎用ネスト書き込み）
	/// @tparam T visit(JsonWriter&)を持つ型
	template <typename T>
		requires ConstVisitable<T, JsonWriter>
			&& (!std::same_as<T, JsonWriter>)
			&& (!std::same_as<T, JsonArrayWriter>)
	void write(const std::string& name, const T& obj)
	{
		JsonWriter nested;
		obj.visit(nested);
		m_fields.push_back({name, nested.toString()});
	}

	/// @brief ConstVisitable型のvectorを書き込む
	/// @tparam T visit(JsonWriter&)を持つ型
	template <typename T>
		requires ConstVisitable<T, JsonWriter>
			&& (!std::same_as<T, int>) && (!std::same_as<T, float>) && (!std::same_as<T, double>)
	void write(const std::string& name, const std::vector<T>& vec)
	{
		JsonArrayWriter arr;
		for (const auto& obj : vec)
		{
			JsonWriter nested;
			obj.visit(nested);
			arr.push(nested);
		}
		m_fields.push_back({name, arr.toString()});
	}

	/// @brief JSON文字列を生成する
	/// @return JSON文字列
	[[nodiscard]] std::string toString() const
	{
		std::string result = "{";
		for (std::size_t i = 0; i < m_fields.size(); ++i)
		{
			if (i > 0) result += ",";
			result += "\"" + m_fields[i].name + "\":" + m_fields[i].value;
		}
		result += "}";
		return result;
	}

private:
	struct Field
	{
		std::string name;
		std::string value;
	};

	std::vector<Field> m_fields;
};

// ── JsonArrayWriter 遅延定義 ─────────────────────────────────

inline void JsonArrayWriter::push(const JsonWriter& nested)
{
	m_elements.push_back(nested.toString());
}

// ── JsonReader ──────────────────────────────────────────────────

/// @brief JSONデシリアライザ（ビジター実装）
///
/// @code
/// sgc::JsonReader r;
/// r.fromString("{\"x\":1.5,\"name\":\"player\"}");
/// float x = 0;
/// r.read("x", x);
/// std::string name;
/// r.read("name", name);
/// @endcode
class JsonReader
{
public:
	/// @brief JSON文字列を解析する
	/// @param json JSON文字列
	void fromString(const std::string& json)
	{
		m_values.clear();
		m_rawObjects.clear();
		m_rawArrays.clear();
		m_hasError = false;
		m_errorMessage.clear();

		auto trimmed = json;
		// 空白を除去して先頭が'{' であることを確認
		std::size_t start = 0;
		while (start < trimmed.size() && (trimmed[start] == ' ' || trimmed[start] == '\t' || trimmed[start] == '\n' || trimmed[start] == '\r'))
			++start;

		if (start >= trimmed.size() || trimmed[start] != '{')
		{
			m_hasError = true;
			m_errorMessage = "Invalid JSON: expected '{' at start";
			return;
		}
		parseObject(json, 0);
	}

	/// @brief パースエラーが発生したかを返す
	/// @return エラーがあればtrue
	[[nodiscard]] bool hasError() const noexcept { return m_hasError; }

	/// @brief エラーメッセージを返す
	/// @return エラーメッセージ（エラーがなければ空文字列）
	[[nodiscard]] const std::string& errorMessage() const noexcept { return m_errorMessage; }

	/// @brief 整数値を読み込む
	void read(const std::string& name, int& value) const
	{
		const auto it = m_values.find(name);
		if (it != m_values.end())
		{
			const auto& s = it->second;
			auto result = std::from_chars(s.data(), s.data() + s.size(), value);
			if (result.ec != std::errc{}) value = 0;
		}
	}

	/// @brief 浮動小数点値を読み込む
	void read(const std::string& name, float& value) const
	{
		const auto it = m_values.find(name);
		if (it != m_values.end())
		{
			const auto& s = it->second;
			if (s == "null") { value = 0.0f; return; }
			try { value = std::stof(s); }
			catch (...) { value = 0.0f; }
		}
	}

	/// @brief 倍精度浮動小数点値を読み込む
	void read(const std::string& name, double& value) const
	{
		const auto it = m_values.find(name);
		if (it != m_values.end())
		{
			const auto& s = it->second;
			if (s == "null") { value = 0.0; return; }
			try { value = std::stod(s); }
			catch (...) { value = 0.0; }
		}
	}

	/// @brief 真偽値を読み込む
	void read(const std::string& name, bool& value) const
	{
		const auto it = m_values.find(name);
		if (it != m_values.end())
		{
			value = (it->second == "true");
		}
	}

	/// @brief 文字列を読み込む
	void read(const std::string& name, std::string& value) const
	{
		const auto it = m_values.find(name);
		if (it != m_values.end())
		{
			value = unescapeString(it->second);
		}
	}

	/// @brief ネストされたオブジェクトを読み込む
	void read(const std::string& name, JsonReader& nested) const
	{
		const auto it = m_rawObjects.find(name);
		if (it != m_rawObjects.end())
		{
			nested.fromString(it->second);
		}
	}

	/// @brief int配列を読み込む
	void read(const std::string& name, std::vector<int>& vec) const
	{
		const auto it = m_rawArrays.find(name);
		if (it == m_rawArrays.end()) return;
		vec.clear();
		auto elements = parseArrayElements(it->second);
		for (const auto& s : elements)
		{
			int v = 0;
			std::from_chars(s.data(), s.data() + s.size(), v);
			vec.push_back(v);
		}
	}

	/// @brief float配列を読み込む
	void read(const std::string& name, std::vector<float>& vec) const
	{
		const auto it = m_rawArrays.find(name);
		if (it == m_rawArrays.end()) return;
		vec.clear();
		auto elements = parseArrayElements(it->second);
		for (const auto& s : elements)
		{
			try { vec.push_back(std::stof(s)); }
			catch (...) { vec.push_back(0.0f); }
		}
	}

	/// @brief double配列を読み込む
	void read(const std::string& name, std::vector<double>& vec) const
	{
		const auto it = m_rawArrays.find(name);
		if (it == m_rawArrays.end()) return;
		vec.clear();
		auto elements = parseArrayElements(it->second);
		for (const auto& s : elements)
		{
			try { vec.push_back(std::stod(s)); }
			catch (...) { vec.push_back(0.0); }
		}
	}

	/// @brief Visitable型を読み込む（汎用ネスト読み込み）
	/// @tparam T visit(JsonReader&)を持つ型
	template <typename T>
		requires Visitable<T, JsonReader>
			&& (!std::same_as<T, JsonReader>)
	void read(const std::string& name, T& obj) const
	{
		const auto it = m_rawObjects.find(name);
		if (it != m_rawObjects.end())
		{
			JsonReader nested;
			nested.fromString(it->second);
			obj.visit(nested);
		}
	}

	/// @brief Visitable型のvectorを読み込む
	/// @tparam T visit(JsonReader&)を持つ型
	template <typename T>
		requires Visitable<T, JsonReader>
			&& (!std::same_as<T, int>) && (!std::same_as<T, float>) && (!std::same_as<T, double>)
	void read(const std::string& name, std::vector<T>& vec) const
	{
		const auto it = m_rawArrays.find(name);
		if (it == m_rawArrays.end()) return;
		vec.clear();
		auto elements = parseArrayElements(it->second);
		for (const auto& elem : elements)
		{
			T obj{};
			JsonReader nested;
			nested.fromString(elem);
			obj.visit(nested);
			vec.push_back(std::move(obj));
		}
	}

	/// @brief 指定キーが存在するか確認する
	[[nodiscard]] bool has(const std::string& name) const
	{
		return m_values.contains(name) || m_rawObjects.contains(name) || m_rawArrays.contains(name);
	}

private:
	std::unordered_map<std::string, std::string> m_values;
	std::unordered_map<std::string, std::string> m_rawObjects;
	std::unordered_map<std::string, std::string> m_rawArrays;
	bool m_hasError{false};               ///< パースエラーフラグ
	std::string m_errorMessage;           ///< エラーメッセージ

	/// @brief 空白をスキップする
	[[nodiscard]] static std::size_t skipWhitespace(const std::string& s, std::size_t pos)
	{
		while (pos < s.size() && (s[pos] == ' ' || s[pos] == '\t' || s[pos] == '\n' || s[pos] == '\r'))
		{
			++pos;
		}
		return pos;
	}

	/// @brief 文字列リテラルを解析する（開始引用符の位置から）
	/// @return {解析された文字列, 終了位置（閉じ引用符の次）}
	[[nodiscard]] static std::pair<std::string, std::size_t> parseString(const std::string& s, std::size_t pos)
	{
		if (pos >= s.size() || s[pos] != '"') return {"", pos};
		++pos;

		std::string result;
		while (pos < s.size() && s[pos] != '"')
		{
			if (s[pos] == '\\' && pos + 1 < s.size())
			{
				++pos;
				switch (s[pos])
				{
				case '"': result += '"'; break;
				case '\\': result += '\\'; break;
				case 'n': result += '\n'; break;
				case 'r': result += '\r'; break;
				case 't': result += '\t'; break;
				default: result += s[pos]; break;
				}
			}
			else
			{
				result += s[pos];
			}
			++pos;
		}
		if (pos < s.size()) ++pos; // 閉じ引用符をスキップ
		return {result, pos};
	}

	/// @brief 値の種別
	enum class ValueKind { Primitive, Object, Array };

	/// @brief 値を解析する（位置を返す）
	struct ParsedValue
	{
		std::string value;
		std::size_t endPos;
		ValueKind kind;
	};

	[[nodiscard]] static ParsedValue parseValue(const std::string& s, std::size_t pos)
	{
		pos = skipWhitespace(s, pos);
		if (pos >= s.size()) return {"", pos, ValueKind::Primitive};

		// 文字列
		if (s[pos] == '"')
		{
			auto [str, end] = parseString(s, pos);
			return {str, end, ValueKind::Primitive};
		}

		// ネストされたオブジェクト
		if (s[pos] == '{')
		{
			int depth = 0;
			std::size_t start = pos;
			while (pos < s.size())
			{
				if (s[pos] == '{') ++depth;
				else if (s[pos] == '}') { --depth; if (depth == 0) { ++pos; break; } }
				else if (s[pos] == '"')
				{
					// 文字列内のブレースをスキップ
					++pos;
					while (pos < s.size() && s[pos] != '"')
					{
						if (s[pos] == '\\') ++pos;
						++pos;
					}
				}
				++pos;
			}
			return {s.substr(start, pos - start), pos, ValueKind::Object};
		}

		// 配列
		if (s[pos] == '[')
		{
			int depth = 0;
			std::size_t start = pos;
			while (pos < s.size())
			{
				if (s[pos] == '[') ++depth;
				else if (s[pos] == ']') { --depth; if (depth == 0) { ++pos; break; } }
				else if (s[pos] == '"')
				{
					++pos;
					while (pos < s.size() && s[pos] != '"')
					{
						if (s[pos] == '\\') ++pos;
						++pos;
					}
				}
				++pos;
			}
			return {s.substr(start, pos - start), pos, ValueKind::Array};
		}

		// 数値、true、false、null
		std::size_t start = pos;
		while (pos < s.size() && s[pos] != ',' && s[pos] != '}' && s[pos] != ']' && s[pos] != ' ')
		{
			++pos;
		}
		return {s.substr(start, pos - start), pos, ValueKind::Primitive};
	}

	/// @brief JSONオブジェクトを解析する
	void parseObject(const std::string& json, std::size_t pos)
	{
		pos = skipWhitespace(json, pos);
		if (pos >= json.size() || json[pos] != '{') return;
		++pos;

		while (pos < json.size())
		{
			pos = skipWhitespace(json, pos);
			if (pos >= json.size() || json[pos] == '}') break;

			// キー
			auto [key, keyEnd] = parseString(json, pos);
			pos = skipWhitespace(json, keyEnd);
			if (pos >= json.size() || json[pos] != ':') break;
			++pos;

			// 値
			auto parsed = parseValue(json, pos);
			pos = parsed.endPos;

			switch (parsed.kind)
			{
			case ValueKind::Object:
				m_rawObjects[key] = parsed.value;
				break;
			case ValueKind::Array:
				m_rawArrays[key] = parsed.value;
				break;
			default:
				m_values[key] = parsed.value;
				break;
			}

			pos = skipWhitespace(json, pos);
			if (pos < json.size() && json[pos] == ',') ++pos;
		}
	}

	/// @brief 配列文字列からトップレベル要素を分割する
	/// @param arrayStr "[1,2,3]" 形式の文字列
	/// @return 各要素の文字列
	[[nodiscard]] static std::vector<std::string> parseArrayElements(const std::string& arrayStr)
	{
		std::vector<std::string> elements;
		if (arrayStr.size() < 2 || arrayStr.front() != '[' || arrayStr.back() != ']')
			return elements;

		// 先頭の'['と末尾の']'を除去
		std::size_t pos = 1;
		const std::size_t end = arrayStr.size() - 1;

		while (pos < end)
		{
			pos = skipWhitespace(arrayStr, pos);
			if (pos >= end) break;

			auto parsed = parseValue(arrayStr, pos);
			if (!parsed.value.empty())
			{
				elements.push_back(parsed.value);
			}
			pos = skipWhitespace(arrayStr, parsed.endPos);
			if (pos < end && arrayStr[pos] == ',') ++pos;
		}
		return elements;
	}

	/// @brief エスケープされた文字列を元に戻す
	[[nodiscard]] static std::string unescapeString(const std::string& s)
	{
		// parseStringが既にアンエスケープ済みの値を返すので、そのまま返す
		return s;
	}
};

// ── 便利関数 ────────────────────────────────────────────────────

/// @brief Visitable型をJSON文字列に変換する
/// @tparam T シリアライズする型
/// @param obj オブジェクト
/// @return JSON文字列
template <typename T>
[[nodiscard]] std::string toJson(const T& obj)
	requires ConstVisitable<T, JsonWriter>
{
	JsonWriter writer;
	obj.visit(writer);
	return writer.toString();
}

/// @brief JSON文字列からVisitable型を復元する
/// @tparam T デシリアライズする型
/// @param json JSON文字列
/// @return 復元されたオブジェクト
template <typename T>
[[nodiscard]] T fromJson(const std::string& json)
	requires std::default_initializable<T> && Visitable<T, JsonReader>
{
	JsonReader reader;
	reader.fromString(json);
	T obj{};
	obj.visit(reader);
	return obj;
}

} // namespace sgc

// Result.hppに依存するため、sgcネームスペース閉じの後にインクルード
#include "sgc/types/Result.hpp"

namespace sgc
{

/// @brief JSON文字列からVisitable型をResult<T>で復元する
///
/// パースエラー時にはエラーメッセージ付きのResult<T>を返す。
///
/// @tparam T デシリアライズする型
/// @param json JSON文字列
/// @return 成功時はT、失敗時はError
///
/// @code
/// auto result = sgc::fromJsonResult<PlayerData>(jsonString);
/// if (result)
/// {
///     auto& player = result.value();
/// }
/// else
/// {
///     // result.error().message にエラー詳細
/// }
/// @endcode
template <typename T>
[[nodiscard]] Result<T> fromJsonResult(const std::string& json)
	requires std::default_initializable<T> && Visitable<T, JsonReader>
{
	JsonReader reader;
	reader.fromString(json);
	if (reader.hasError())
	{
		return {ERROR_TAG, Error{reader.errorMessage()}};
	}
	T obj{};
	obj.visit(reader);
	return obj;
}

} // namespace sgc
