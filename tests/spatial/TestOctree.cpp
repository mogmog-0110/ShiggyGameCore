/// @file TestOctree.cpp
/// @brief Octree.hpp のユニットテスト

#include <catch2/catch_test_macros.hpp>

#include "sgc/spatial/Octree.hpp"

TEST_CASE("Octree insert and size", "[spatial][octree]")
{
	sgc::Octreef ot(sgc::AABB3f{{0, 0, 0}, {100, 100, 100}});
	REQUIRE(ot.size() == 0);

	ot.insert(sgc::AABB3f{{10, 10, 10}, {20, 20, 20}});
	REQUIRE(ot.size() == 1);
}

TEST_CASE("Octree remove", "[spatial][octree]")
{
	sgc::Octreef ot(sgc::AABB3f{{0, 0, 0}, {100, 100, 100}});
	auto h = ot.insert(sgc::AABB3f{{10, 10, 10}, {20, 20, 20}});
	ot.remove(h);
	REQUIRE(ot.size() == 0);
}

TEST_CASE("Octree queryRange finds elements", "[spatial][octree]")
{
	sgc::Octreef ot(sgc::AABB3f{{0, 0, 0}, {100, 100, 100}});
	ot.insert(sgc::AABB3f{{10, 10, 10}, {20, 20, 20}});
	ot.insert(sgc::AABB3f{{80, 80, 80}, {90, 90, 90}});

	auto results = ot.queryRange(sgc::AABB3f{{0, 0, 0}, {50, 50, 50}});
	REQUIRE(results.size() == 1);
}

TEST_CASE("Octree queryPoint", "[spatial][octree]")
{
	sgc::Octreef ot(sgc::AABB3f{{0, 0, 0}, {100, 100, 100}});
	ot.insert(sgc::AABB3f{{10, 10, 10}, {20, 20, 20}});

	auto hit = ot.queryPoint({15, 15, 15});
	REQUIRE(hit.size() == 1);

	auto miss = ot.queryPoint({99, 99, 99});
	REQUIRE(miss.empty());
}

TEST_CASE("Octree querySphere", "[spatial][octree]")
{
	sgc::Octreef ot(sgc::AABB3f{{0, 0, 0}, {100, 100, 100}});
	ot.insert(sgc::AABB3f{{10, 10, 10}, {20, 20, 20}});
	ot.insert(sgc::AABB3f{{80, 80, 80}, {90, 90, 90}});

	auto results = ot.querySphere({15, 15, 15}, 10.0f);
	REQUIRE(results.size() == 1);
}

TEST_CASE("Octree queryFrustum", "[spatial][octree]")
{
	sgc::Octreef ot(sgc::AABB3f{{-100, -100, -200}, {100, 100, 0}});
	ot.insert(sgc::AABB3f{{-5, -5, -50}, {5, 5, -40}});    // 視錐台内
	ot.insert(sgc::AABB3f{{90, 90, 90}, {95, 95, 95}});     // 視錐台外

	// 正射影のフラスタムを作成
	const auto view = sgc::Mat4f::lookAt({0, 0, 0}, {0, 0, -1}, {0, 1, 0});
	const auto proj = sgc::Mat4f::orthographic(-10.0f, 10.0f, -10.0f, 10.0f, 0.1f, 100.0f);
	const auto frustum = sgc::Frustumf::fromViewProjection(proj * view);

	auto results = ot.queryFrustum(frustum);
	REQUIRE(results.size() == 1);
}

TEST_CASE("Octree clear", "[spatial][octree]")
{
	sgc::Octreef ot(sgc::AABB3f{{0, 0, 0}, {100, 100, 100}});
	ot.insert(sgc::AABB3f{{10, 10, 10}, {20, 20, 20}});
	ot.clear();
	REQUIRE(ot.size() == 0);
}

TEST_CASE("Octree many elements triggers subdivision", "[spatial][octree]")
{
	sgc::Octreef ot(sgc::AABB3f{{0, 0, 0}, {1000, 1000, 1000}});

	for (int i = 0; i < 30; ++i)
	{
		const float v = static_cast<float>(i * 30);
		ot.insert(sgc::AABB3f{{v, v, v}, {v + 5, v + 5, v + 5}});
	}
	REQUIRE(ot.size() == 30);

	auto results = ot.queryRange(sgc::AABB3f{{0, 0, 0}, {100, 100, 100}});
	REQUIRE_FALSE(results.empty());
}

TEST_CASE("Octree empty tree query returns empty", "[spatial][octree]")
{
	sgc::Octreef ot(sgc::AABB3f{{0, 0, 0}, {100, 100, 100}});

	auto rangeResults = ot.queryRange(sgc::AABB3f{{0, 0, 0}, {100, 100, 100}});
	REQUIRE(rangeResults.empty());

	auto pointResults = ot.queryPoint({50, 50, 50});
	REQUIRE(pointResults.empty());

	auto sphereResults = ot.querySphere({50, 50, 50}, 10.0f);
	REQUIRE(sphereResults.empty());

	REQUIRE(ot.size() == 0);
}
