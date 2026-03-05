# Siv3D シューティングゲームサンプル
#
# ビルド方法:
#   1. OpenSiv3Dをインストール済みであること
#   2. CMake構成時に -DSGC_BUILD_SIV3D_EXAMPLES=ON を指定
#   3. Siv3DのCMakeパッケージが見つかること（CMAKE_PREFIX_PATHで指定）
#
# 例:
#   cmake -B build -DSGC_BUILD_SIV3D_EXAMPLES=ON -DCMAKE_PREFIX_PATH="C:/path/to/Siv3D"
#   cmake --build build --target siv3d_shooting

cmake_minimum_required(VERSION 3.21)

find_package(Siv3D REQUIRED)

add_executable(siv3d_shooting Main.cpp)

target_link_libraries(siv3d_shooting
    PRIVATE
        sgc
        Siv3D::Siv3D
)

target_include_directories(siv3d_shooting
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

# C++20
target_compile_features(siv3d_shooting PRIVATE cxx_std_20)

# MSVC: UTF-8ソースファイル対応
if(MSVC)
    target_compile_options(siv3d_shooting PRIVATE /utf-8)
endif()
