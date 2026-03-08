/// @file math_showcase.cpp
/// @brief Vec・Mat・Quaternion・Easing等の数学ユーティリティを実演するサンプル
///
/// sgcの数学ライブラリの主要機能を一通り紹介する。

#include <cmath>
#include <iomanip>
#include <iostream>
#include <numbers>

#include "sgc/math/Vec2.hpp"
#include "sgc/math/Vec3.hpp"
#include "sgc/math/Mat3.hpp"
#include "sgc/math/Quaternion.hpp"
#include "sgc/math/Easing.hpp"
#include "sgc/math/Interpolation.hpp"
#include "sgc/math/Geometry.hpp"

/// @brief 度をラジアンに変換するヘルパー
static constexpr float toRad(float degrees)
{
	return degrees * std::numbers::pi_v<float> / 180.0f;
}

/// @brief Vec2f を見やすく表示する
static void printVec2(const char* label, const sgc::Vec2f& v)
{
	std::cout << "  " << label << ": (" << v.x << ", " << v.y << ")\n";
}

/// @brief Vec3f を見やすく表示する
static void printVec3(const char* label, const sgc::Vec3f& v)
{
	std::cout << "  " << label << ": (" << v.x << ", " << v.y << ", " << v.z << ")\n";
}

int main()
{
	std::cout << std::fixed << std::setprecision(4);
	std::cout << "=== sgc Math Showcase ===\n\n";

	// ────────────────────────────────────────────────────
	// 1. Vec2f: 2Dベクトル演算
	// ────────────────────────────────────────────────────
	std::cout << "[1] Vec2f - 2D Vector Operations:\n";
	{
		const sgc::Vec2f a{3.0f, 4.0f};
		const sgc::Vec2f b{1.0f, 2.0f};

		printVec2("a", a);
		printVec2("b", b);
		printVec2("a + b", a + b);
		printVec2("a - b", a - b);
		printVec2("a * 2", a * 2.0f);

		std::cout << "  dot(a, b):    " << a.dot(b) << "\n";
		std::cout << "  cross(a, b):  " << a.cross(b) << "\n";
		std::cout << "  |a|:          " << a.length() << "\n";

		printVec2("normalize(a)", a.normalized());
		std::cout << "  distance(a, b): " << a.distanceTo(b) << "\n";

		/// 反射（壁の法線が上向きの場合）
		const sgc::Vec2f velocity{1.0f, -1.0f};
		const sgc::Vec2f normal{0.0f, 1.0f};
		printVec2("reflect", velocity.reflect(normal));

		/// 回転（90度反時計回り）
		const sgc::Vec2f right{1.0f, 0.0f};
		printVec2("rotate 90deg", right.rotate(toRad(90.0f)));
	}
	std::cout << "\n";

	// ────────────────────────────────────────────────────
	// 2. Vec3f: 3Dベクトル演算
	// ────────────────────────────────────────────────────
	std::cout << "[2] Vec3f - 3D Vector Operations:\n";
	{
		const sgc::Vec3f forward{0.0f, 0.0f, -1.0f};
		const sgc::Vec3f up{0.0f, 1.0f, 0.0f};

		printVec3("forward", forward);
		printVec3("up", up);

		/// 外積: forward x up = right (右手座標系)
		const auto right = forward.cross(up);
		printVec3("forward x up (right)", right);

		std::cout << "  dot(forward, up): " << forward.dot(up) << "\n";

		const sgc::Vec3f diagonal{1.0f, 1.0f, 1.0f};
		printVec3("normalized(1,1,1)", diagonal.normalized());
		std::cout << "  |diagonal|: " << diagonal.length() << "\n";

		/// 軸まわりの回転（Y軸回りに90度）
		const sgc::Vec3f point{1.0f, 0.0f, 0.0f};
		const auto rotated = point.rotateAroundAxis(sgc::Vec3f::unitY(), toRad(90.0f));
		printVec3("rotate (1,0,0) around Y by 90deg", rotated);
	}
	std::cout << "\n";

	// ────────────────────────────────────────────────────
	// 3. Mat3f: 2D変換行列
	// ────────────────────────────────────────────────────
	std::cout << "[3] Mat3f - 2D Transformation Matrices:\n";
	{
		/// 45度回転行列
		const auto rotMat = sgc::Mat3f::rotation(toRad(45.0f));
		const sgc::Vec2f point{1.0f, 0.0f};
		const auto rotated = rotMat.transformPoint(point);
		printVec2("Rotate (1,0) by 45deg", rotated);

		/// 平行移動 + 回転の合成
		const auto transform = sgc::Mat3f::translation({10.0f, 5.0f})
							 * sgc::Mat3f::rotation(toRad(90.0f))
							 * sgc::Mat3f::scaling({2.0f, 2.0f});

		const sgc::Vec2f origin{1.0f, 0.0f};
		const auto result = transform.transformPoint(origin);
		printVec2("Scale(2) -> Rot(90) -> Trans(10,5) applied to (1,0)", result);

		/// 行列式と逆行列
		const auto mat = sgc::Mat3f::rotation(toRad(30.0f));
		std::cout << "  det(rotation30): " << mat.determinant() << "\n";

		const auto inv = mat.inversed();
		const auto identity = mat * inv;
		std::cout << "  mat * inv = identity? diag=("
				  << identity.m[0][0] << ", " << identity.m[1][1] << ", " << identity.m[2][2] << ")\n";
	}
	std::cout << "\n";

	// ────────────────────────────────────────────────────
	// 4. Quaternion: 3D回転
	// ────────────────────────────────────────────────────
	std::cout << "[4] Quaternion - 3D Rotation:\n";
	{
		/// Y軸回りに90度回転するクォータニオン
		const auto rot90Y = sgc::Quaternionf::fromAxisAngle(
			sgc::Vec3f::unitY(), toRad(90.0f));
		std::cout << "  rot90Y: (x=" << rot90Y.x << ", y=" << rot90Y.y
				  << ", z=" << rot90Y.z << ", w=" << rot90Y.w << ")\n";

		/// ベクトルの回転
		const sgc::Vec3f forwardVec{0.0f, 0.0f, -1.0f};
		const auto rotatedVec = rot90Y.rotate(forwardVec);
		printVec3("Rotate (0,0,-1) by 90deg around Y", rotatedVec);

		/// 回転の合成（Y90 + X45）
		const auto rotX45 = sgc::Quaternionf::fromAxisAngle(
			sgc::Vec3f::unitX(), toRad(45.0f));
		const auto combined = rot90Y * rotX45;
		const auto euler = combined.toEuler();
		std::cout << "  Combined euler (rad): ("
				  << euler.x << ", " << euler.y << ", " << euler.z << ")\n";

		/// Slerp: 2つの回転間の球面線形補間
		std::cout << "  Slerp between identity and rot90Y:\n";
		const auto qIdentity = sgc::Quaternionf::identity();
		for (float t = 0.0f; t <= 1.01f; t += 0.25f)
		{
			const auto q = sgc::Quaternionf::slerp(qIdentity, rot90Y, t);
			const auto v = q.rotate(sgc::Vec3f{0.0f, 0.0f, -1.0f});
			std::cout << "    t=" << t << " -> ("
					  << v.x << ", " << v.y << ", " << v.z << ")\n";
		}
	}
	std::cout << "\n";

	// ────────────────────────────────────────────────────
	// 5. Easing: イージング関数
	// ────────────────────────────────────────────────────
	std::cout << "[5] Easing Functions (t = 0.0 to 1.0):\n";
	{
		std::cout << "  t     | linear | inQuad | outCubic | inOutQuad | outBounce\n";
		std::cout << "  ------+--------+--------+----------+-----------+----------\n";

		for (float t = 0.0f; t <= 1.001f; t += 0.2f)
		{
			const float clamped = (t > 1.0f) ? 1.0f : t;
			std::cout << "  " << std::setw(5) << clamped
					  << " | " << std::setw(6) << sgc::easing::linear(clamped)
					  << " | " << std::setw(6) << sgc::easing::inQuad(clamped)
					  << " | " << std::setw(8) << sgc::easing::outCubic(clamped)
					  << " | " << std::setw(9) << sgc::easing::inOutQuad(clamped)
					  << " | " << std::setw(9) << sgc::easing::outBounce(clamped)
					  << "\n";
		}

		/// 実用例: HPバーのアニメーション
		std::cout << "\n  HP bar animation (outCubic, 100 -> 30):\n    ";
		for (float t = 0.0f; t <= 1.001f; t += 0.1f)
		{
			const float clamped = (t > 1.0f) ? 1.0f : t;
			const float hp = sgc::lerp(100.0f, 30.0f, sgc::easing::outCubic(clamped));
			std::cout << static_cast<int>(hp) << " ";
		}
		std::cout << "\n";
	}
	std::cout << "\n";

	// ────────────────────────────────────────────────────
	// 6. Interpolation: 補間関数
	// ────────────────────────────────────────────────────
	std::cout << "[6] Interpolation Functions:\n";
	{
		/// lerp
		std::cout << "  lerp(0, 100, 0.75) = " << sgc::lerp(0.0f, 100.0f, 0.75f) << "\n";

		/// inverseLerp
		std::cout << "  inverseLerp(0, 100, 60) = " << sgc::inverseLerp(0.0f, 100.0f, 60.0f) << "\n";

		/// remap（温度変換: 0-100 -> 32-212）
		const float celsius = 37.0f;
		const float fahrenheit = sgc::remap(0.0f, 100.0f, 32.0f, 212.0f, celsius);
		std::cout << "  remap " << celsius << "C -> " << fahrenheit << "F\n";

		/// smoothstep
		std::cout << "  smoothstep samples: ";
		for (float x = 0.0f; x <= 1.001f; x += 0.25f)
		{
			const float clamped = (x > 1.0f) ? 1.0f : x;
			std::cout << sgc::smoothstep(0.0f, 1.0f, clamped) << " ";
		}
		std::cout << "\n";

		/// moveTowards
		float current = 0.0f;
		const float target = 10.0f;
		std::cout << "  moveTowards(0 -> 10, delta=3): ";
		for (int step = 0; step < 5; ++step)
		{
			current = sgc::moveTowards(current, target, 3.0f);
			std::cout << current << " ";
		}
		std::cout << "\n";
	}
	std::cout << "\n";

	// ────────────────────────────────────────────────────
	// 7. Geometry: AABB と Circle
	// ────────────────────────────────────────────────────
	std::cout << "[7] Geometry - AABB & Circle:\n";
	{
		/// AABB2f
		const sgc::AABB2f box{{10.0f, 10.0f}, {50.0f, 40.0f}};
		std::cout << "  AABB: min=(" << box.min.x << "," << box.min.y
				  << ") max=(" << box.max.x << "," << box.max.y << ")\n";
		printVec2("center", box.center());
		printVec2("size", box.size());
		std::cout << "  area: " << box.area() << "\n";

		const sgc::Vec2f insidePoint{30.0f, 25.0f};
		const sgc::Vec2f outsidePoint{60.0f, 25.0f};
		std::cout << "  contains(30,25): " << (box.contains(insidePoint) ? "yes" : "no") << "\n";
		std::cout << "  contains(60,25): " << (box.contains(outsidePoint) ? "yes" : "no") << "\n";

		/// AABB同士の交差判定
		const sgc::AABB2f box2{{40.0f, 30.0f}, {70.0f, 60.0f}};
		const sgc::AABB2f box3{{100.0f, 100.0f}, {120.0f, 120.0f}};
		std::cout << "  box intersects box2: " << (box.intersects(box2) ? "yes" : "no") << "\n";
		std::cout << "  box intersects box3: " << (box.intersects(box3) ? "yes" : "no") << "\n";

		/// Circle
		std::cout << "\n";
		const sgc::Circlef circle{{30.0f, 25.0f}, 15.0f};
		std::cout << "  Circle: center=(" << circle.center.x << "," << circle.center.y
				  << ") r=" << circle.radius << "\n";
		std::cout << "  area: " << circle.area() << "\n";
		std::cout << "  contains(30,25): " << (circle.contains({30.0f, 25.0f}) ? "yes" : "no") << "\n";
		std::cout << "  contains(50,50): " << (circle.contains({50.0f, 50.0f}) ? "yes" : "no") << "\n";

		/// 円とAABBの交差判定
		std::cout << "  circle intersects box: " << (circle.intersects(box) ? "yes" : "no") << "\n";
		std::cout << "  circle intersects box3: " << (circle.intersects(box3) ? "yes" : "no") << "\n";

		/// 円同士の交差判定
		const sgc::Circlef circle2{{60.0f, 25.0f}, 20.0f};
		const sgc::Circlef circle3{{200.0f, 200.0f}, 5.0f};
		std::cout << "  circle intersects circle2: " << (circle.intersects(circle2) ? "yes" : "no") << "\n";
		std::cout << "  circle intersects circle3: " << (circle.intersects(circle3) ? "yes" : "no") << "\n";
	}

	std::cout << "\n=== Demo Complete ===\n";
	return 0;
}
