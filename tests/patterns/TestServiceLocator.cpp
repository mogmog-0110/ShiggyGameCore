/// @file TestServiceLocator.cpp
/// @brief ServiceLocator.hpp のユニットテスト

#include <catch2/catch_test_macros.hpp>

#include "sgc/patterns/ServiceLocator.hpp"

#include <string>

namespace
{
struct IAudio
{
	virtual ~IAudio() = default;
	virtual std::string name() const = 0;
};

struct MockAudio : IAudio
{
	std::string name() const override { return "MockAudio"; }
};
} // namespace

TEST_CASE("ServiceLocator provide and get", "[patterns][service]")
{
	sgc::ServiceLocator locator;
	locator.provide<IAudio>(std::make_shared<MockAudio>());

	REQUIRE(locator.has<IAudio>());
	REQUIRE(locator.get<IAudio>().name() == "MockAudio");
}

TEST_CASE("ServiceLocator has returns false for unregistered", "[patterns][service]")
{
	sgc::ServiceLocator locator;
	REQUIRE_FALSE(locator.has<IAudio>());
}

TEST_CASE("ServiceLocator get throws for unregistered", "[patterns][service]")
{
	sgc::ServiceLocator locator;
	REQUIRE_THROWS_AS(locator.get<IAudio>(), std::runtime_error);
}

TEST_CASE("ServiceLocator remove unregisters service", "[patterns][service]")
{
	sgc::ServiceLocator locator;
	locator.provide<IAudio>(std::make_shared<MockAudio>());
	locator.remove<IAudio>();

	REQUIRE_FALSE(locator.has<IAudio>());
}

TEST_CASE("ServiceLocator clear removes all", "[patterns][service]")
{
	sgc::ServiceLocator locator;
	locator.provide<IAudio>(std::make_shared<MockAudio>());
	locator.clear();

	REQUIRE_FALSE(locator.has<IAudio>());
}
