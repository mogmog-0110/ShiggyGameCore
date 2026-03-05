include(FetchContent)

FetchContent_Declare(
	Catch2
	GIT_REPOSITORY https://github.com/catchorg/Catch2.git
	GIT_TAG v3.8.0
	GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(Catch2)

add_executable(sgc_tests
	# Core
	TestCore.cpp
	core/TestAssert.cpp
	core/TestScopeGuard.cpp
	core/TestFlags.cpp
	core/TestTypeId.cpp
	core/TestTimer.cpp
	core/TestRandom.cpp
	core/TestLogger.cpp
	core/TestCoroutine.cpp
	core/TestSerialize.cpp

	# Types
	types/TestConcepts.cpp
	types/TestResult.cpp
	types/TestTypeList.cpp
	types/TestTypeTraits.cpp
	types/TestColor.cpp

	# Math
	math/TestMathConstants.cpp
	math/TestVec2.cpp
	math/TestVec3.cpp
	math/TestVec4.cpp
	math/TestMat3.cpp
	math/TestMat4.cpp
	math/TestQuaternion.cpp
	math/TestEasing.cpp
	math/TestInterpolation.cpp
	math/TestGeometry.cpp
	math/TestRect.cpp
	math/TestPlane.cpp
	math/TestRay3.cpp
	math/TestFrustum.cpp
	math/TestIntersection.cpp
	math/TestNoise.cpp

	# Patterns
	patterns/TestEventDispatcher.cpp
	patterns/TestStateMachine.cpp
	patterns/TestObjectPool.cpp
	patterns/TestCommand.cpp
	patterns/TestObserver.cpp
	patterns/TestServiceLocator.cpp

	# Animation
	animation/TestTween.cpp

	# Spatial
	spatial/TestGrid.cpp
	spatial/TestQuadtree.cpp
	spatial/TestOctree.cpp

	# AI
	ai/TestBehaviorTree.cpp

	# ECS
	ecs/TestECS.cpp

	# Memory
	memory/TestArenaAllocator.cpp
	memory/TestPoolAllocator.cpp
)

target_link_libraries(sgc_tests PRIVATE
	sgc::sgc
	Catch2::Catch2WithMain
)

list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)
include(CTest)
include(Catch)
catch_discover_tests(sgc_tests)
