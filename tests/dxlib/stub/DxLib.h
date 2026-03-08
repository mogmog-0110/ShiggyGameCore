#pragma once

/// @file DxLib.h
/// @brief DxLibスタブ（テスト用）
///
/// DxLibの関数と定数をスタブとして定義する。
/// 描画コールを記録し、入力状態をモックできる。

#include <cstring>
#include <vector>

// ── DxLib定数 ────────────────────────────────────────────

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#define DX_BLENDMODE_ALPHA   1
#define DX_BLENDMODE_NOBLEND 0

#define MOUSE_INPUT_LEFT   0x0001
#define MOUSE_INPUT_RIGHT  0x0002
#define MOUSE_INPUT_MIDDLE 0x0004

#define KEY_INPUT_SPACE 57
#define KEY_INPUT_A     30
#define KEY_INPUT_W     17
#define KEY_INPUT_S     31
#define KEY_INPUT_D     32

#define DX_FONTTYPE_ANTIALIASING_EDGE_4X4 23

#define DX_PLAYTYPE_LOOP 3
#define DX_PLAYTYPE_BACK 2

// ── スタブ記録・モック状態 ──────────────────────────────

namespace dxlib_stub
{

/// @brief 描画コールの種別
enum class DrawType
{
	BoxAA,
	Box,
	CircleAA,
	LineAA,
	TriangleAA,
	BlendMode,
	StringToHandle,
	SoundPlay,
	SoundStop,
	SoundVolume,
	SoundLoad,
	SoundDelete,
	SoundCheck
};

/// @brief 描画コール記録
struct DrawCall
{
	DrawType type;
	float params[8]{};       ///< 関数パラメータ（可変長）
	unsigned int color{0};   ///< カラー値
	int fillFlag{0};         ///< 塗りつぶしフラグ
};

/// @brief 描画コール記録リストを取得する
inline std::vector<DrawCall>& drawCalls()
{
	static std::vector<DrawCall> calls;
	return calls;
}

/// @brief モック用キー状態バッファを取得する
inline char* keyState()
{
	static char state[256] = {};
	return state;
}

/// @brief モック用マウスX座標
inline int& mouseX()
{
	static int x = 0;
	return x;
}

/// @brief モック用マウスY座標
inline int& mouseY()
{
	static int y = 0;
	return y;
}

/// @brief モック用マウスボタン状態
inline int& mouseInput()
{
	static int input = 0;
	return input;
}

/// @brief モック用前フレームキー状態バッファを取得する
inline char* prevKeyState()
{
	static char state[256] = {};
	return state;
}

/// @brief フォント作成記録
struct FontRecord
{
	int handle;       ///< フォントハンドル
	int size;         ///< フォントサイズ
	int fontType;     ///< フォントタイプ
	bool deleted;     ///< 削除済みか
};

/// @brief フォント記録リストを取得する
inline std::vector<FontRecord>& fontRecords()
{
	static std::vector<FontRecord> records;
	return records;
}

/// @brief 次のフォントハンドルカウンタ
inline int& nextFontHandle()
{
	static int handle = 1;
	return handle;
}

/// @brief ProcessMessage()の戻り値を制御する
inline int& processMessageResult()
{
	static int r = 0;
	return r;
}

/// @brief ProcessMessage()の呼び出し回数を取得する
inline int& processMessageCallCount()
{
	static int c = 0;
	return c;
}

/// @brief サウンド記録
struct SoundRecord
{
	int handle;        ///< サウンドハンドル
	int volume;        ///< ボリューム（0-255）
	bool playing;      ///< 再生中か
	bool deleted;      ///< 削除済みか
};

/// @brief サウンド記録リストを取得する
inline std::vector<SoundRecord>& soundRecords()
{
	static std::vector<SoundRecord> records;
	return records;
}

/// @brief 次のサウンドハンドルカウンタ
inline int& nextSoundHandle()
{
	static int handle = 1;
	return handle;
}

/// @brief 全スタブ状態をリセットする
inline void reset()
{
	drawCalls().clear();
	std::memset(keyState(), 0, 256);
	std::memset(prevKeyState(), 0, 256);
	mouseX() = 0;
	mouseY() = 0;
	mouseInput() = 0;
	fontRecords().clear();
	nextFontHandle() = 1;
	soundRecords().clear();
	nextSoundHandle() = 1;
	processMessageResult() = 0;
	processMessageCallCount() = 0;
}

} // namespace dxlib_stub

// ── DxLib関数スタブ ──────────────────────────────────────

inline void DrawBoxAA(float x1, float y1, float x2, float y2,
	unsigned int color, int fillFlag)
{
	dxlib_stub::drawCalls().push_back({
		dxlib_stub::DrawType::BoxAA,
		{x1, y1, x2, y2},
		color, fillFlag
	});
}

inline void DrawCircleAA(float x, float y, float r, int segments,
	unsigned int color, int fillFlag)
{
	dxlib_stub::drawCalls().push_back({
		dxlib_stub::DrawType::CircleAA,
		{x, y, r, static_cast<float>(segments)},
		color, fillFlag
	});
}

inline void DrawLineAA(float x1, float y1, float x2, float y2,
	unsigned int color, float thickness)
{
	dxlib_stub::drawCalls().push_back({
		dxlib_stub::DrawType::LineAA,
		{x1, y1, x2, y2, thickness},
		color, 0
	});
}

inline void DrawTriangleAA(float x1, float y1, float x2, float y2,
	float x3, float y3, unsigned int color, int fillFlag)
{
	dxlib_stub::drawCalls().push_back({
		dxlib_stub::DrawType::TriangleAA,
		{x1, y1, x2, y2, x3, y3},
		color, fillFlag
	});
}

inline void SetDrawBlendMode(int mode, int param)
{
	dxlib_stub::drawCalls().push_back({
		dxlib_stub::DrawType::BlendMode,
		{static_cast<float>(mode), static_cast<float>(param)},
		0, 0
	});
}

inline void DrawBox(int x1, int y1, int x2, int y2,
	unsigned int color, int fillFlag)
{
	dxlib_stub::drawCalls().push_back({
		dxlib_stub::DrawType::Box,
		{static_cast<float>(x1), static_cast<float>(y1),
		 static_cast<float>(x2), static_cast<float>(y2)},
		color, fillFlag
	});
}

inline int GetHitKeyStateAll(char* keyStateBuf)
{
	std::memcpy(keyStateBuf, dxlib_stub::keyState(), 256);
	return 0;
}

inline int GetMousePoint(int* x, int* y)
{
	*x = dxlib_stub::mouseX();
	*y = dxlib_stub::mouseY();
	return 0;
}

inline int GetMouseInput()
{
	return dxlib_stub::mouseInput();
}

// ── フォント関数スタブ ──────────────────────────────────

inline int CreateFontToHandle(const char* /*fontName*/, int size,
	int /*thickness*/, int fontType = -1)
{
	const int handle = dxlib_stub::nextFontHandle()++;
	dxlib_stub::fontRecords().push_back({handle, size, fontType, false});
	return handle;
}

inline int DeleteFontToHandle(int handle)
{
	for (auto& rec : dxlib_stub::fontRecords())
	{
		if (rec.handle == handle)
		{
			rec.deleted = true;
			return 0;
		}
	}
	return -1;
}

inline int DrawStringToHandle(int x, int y, const char* /*text*/,
	unsigned int color, int handle)
{
	dxlib_stub::drawCalls().push_back({
		dxlib_stub::DrawType::StringToHandle,
		{static_cast<float>(x), static_cast<float>(y),
		 static_cast<float>(handle)},
		color, 0
	});
	return 0;
}

inline int GetDrawStringWidthToHandle(const char* /*text*/, int strLen, int /*handle*/)
{
	return strLen * 12;  // 近似値: 1文字あたり12ピクセル
}

// ── ゲームループ関数スタブ ──────────────────────────────

inline int ProcessMessage()
{
	++dxlib_stub::processMessageCallCount();
	return dxlib_stub::processMessageResult();
}

inline void ClearDrawScreen() { /* no-op */ }

inline void ScreenFlip() { /* no-op */ }

// ── サウンド関数スタブ ──────────────────────────────────

inline int LoadSoundMem(const char* /*path*/)
{
	const int handle = dxlib_stub::nextSoundHandle()++;
	dxlib_stub::soundRecords().push_back({handle, 255, false, false});
	return handle;
}

inline int DeleteSoundMem(int handle)
{
	for (auto& rec : dxlib_stub::soundRecords())
	{
		if (rec.handle == handle)
		{
			rec.deleted = true;
			rec.playing = false;
			return 0;
		}
	}
	return -1;
}

inline int PlaySoundMem(int handle, int /*playType*/)
{
	for (auto& rec : dxlib_stub::soundRecords())
	{
		if (rec.handle == handle)
		{
			rec.playing = true;
			return 0;
		}
	}
	return -1;
}

inline int StopSoundMem(int handle)
{
	for (auto& rec : dxlib_stub::soundRecords())
	{
		if (rec.handle == handle)
		{
			rec.playing = false;
			return 0;
		}
	}
	return -1;
}

inline int ChangeVolumeSoundMem(int volume, int handle)
{
	for (auto& rec : dxlib_stub::soundRecords())
	{
		if (rec.handle == handle)
		{
			rec.volume = volume;
			return 0;
		}
	}
	return -1;
}

inline int CheckSoundMem(int handle)
{
	for (const auto& rec : dxlib_stub::soundRecords())
	{
		if (rec.handle == handle)
		{
			return rec.playing ? 1 : 0;
		}
	}
	return 0;
}
