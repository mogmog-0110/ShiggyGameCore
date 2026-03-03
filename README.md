# ShiggyGameCore

C++20 ゲームユーティリティライブラリ。Siv3D / DxLib 等のゲームフレームワーク向けに、デザインパターン・数学ユーティリティ・汎用ツールを提供するヘッダーオンリーライブラリ。

## 要件

- **C++20** 対応コンパイラ
  - MSVC (Visual Studio 2022)
  - GCC 12+
  - Clang 15+
- **CMake** 3.21+

## クイックスタート

```bash
# ビルド＆テスト
cmake --preset default
cmake --build build
ctest --test-dir build
```

## プロジェクトへの組み込み

### FetchContent

```cmake
include(FetchContent)
FetchContent_Declare(
    sgc
    GIT_REPOSITORY https://github.com/yourname/ShiggyGameCore.git
    GIT_TAG main
)
FetchContent_MakeAvailable(sgc)

target_link_libraries(your_target PRIVATE sgc::sgc)
```

### サブディレクトリ

```cmake
add_subdirectory(path/to/ShiggyGameCore)
target_link_libraries(your_target PRIVATE sgc::sgc)
```

## ライセンス

MIT License
