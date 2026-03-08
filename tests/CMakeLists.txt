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
	core/TestStringUtils.cpp
	core/TestHash.cpp
	core/TestHandle.cpp
	core/TestRingBuffer.cpp
	core/TestThreadPool.cpp
	core/TestProfiler.cpp

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
	math/TestSimdVec.cpp

	# Patterns
	patterns/TestEventDispatcher.cpp
	patterns/TestStateMachine.cpp
	patterns/TestObjectPool.cpp
	patterns/TestCommand.cpp
	patterns/TestObserver.cpp
	patterns/TestServiceLocator.cpp

	# Animation
	animation/TestTween.cpp
	animation/TestTweenTimeline.cpp

	# Spatial
	spatial/TestGrid.cpp
	spatial/TestQuadtree.cpp
	spatial/TestOctree.cpp

	# AI
	ai/TestBehaviorTree.cpp

	# ECS
	ecs/TestECS.cpp
	ecs/TestSystem.cpp
	ecs/TestCommandBuffer.cpp
	ecs/TestViewExclude.cpp
	ecs/TestSystemTraits.cpp

	# Graphics
	graphics/TestRenderer.cpp
	graphics/TestTextMeasure.cpp
	graphics/TestTextRenderer.cpp

	# Input
	input/TestActionMap.cpp
	input/TestInputBuffer.cpp
	input/TestInputProvider.cpp
	input/TestInputModeManager.cpp

	# Effects
	effects/TestParticleSystem.cpp

	# Resource
	resource/TestResourceManager.cpp
	resource/TestFileWatcher.cpp

	# Scene
	scene/TestScene.cpp
	scene/TestSceneIntegration.cpp
	scene/TestAppBase.cpp
	scene/TestTransform.cpp
	scene/TestCamera.cpp

	# Config
	config/TestConfigManager.cpp

	# Debug
	debug/TestFpsCounter.cpp
	debug/TestDebugOverlay.cpp

	# Net
	net/TestMessageChannel.cpp
	net/TestStateSync.cpp

	# Memory
	memory/TestArenaAllocator.cpp
	memory/TestPoolAllocator.cpp
	memory/TestStackAllocator.cpp
	memory/TestFreeListAllocator.cpp
	memory/TestStlAllocatorAdapter.cpp

	# UI
	ui/TestAnchor.cpp
	ui/TestWidgetState.cpp
	ui/TestHudLayout.cpp
	ui/TestButton.cpp
	ui/TestTheme.cpp
	ui/TestPendingAction.cpp
	ui/TestSlider.cpp
	ui/TestCheckbox.cpp
	ui/TestProgressBar.cpp

	# Audio
	audio/TestAudioPlayer.cpp

	# Physics
	physics/TestFixedTimestep.cpp
	physics/TestCollisionUtils.cpp
	physics/TestAABB2DCollision.cpp
	physics/TestRigidBody2D.cpp
	physics/TestRayCast2D.cpp

	# DxLib (TypeConvert only - DxLib.h not required)
	dxlib/TestDxLibTypeConvert.cpp

	# Benchmarks
	bench/BenchmarkMath.cpp
	bench/BenchmarkECS.cpp
)

target_link_libraries(sgc_tests PRIVATE
	sgc::sgc
	Catch2::Catch2WithMain
)

list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)
include(CTest)
include(Catch)
catch_discover_tests(sgc_tests)

# DxLib adapter tests (with stub DxLib.h)
add_executable(sgc_dxlib_adapter_tests
	dxlib/TestDxLibRenderer.cpp
	dxlib/TestDxLibInputProvider.cpp
	dxlib/TestDxLibTextRenderer.cpp
	dxlib/TestDxLibSceneAdapter.cpp
	dxlib/TestDxLibAudioPlayer.cpp
)
target_include_directories(sgc_dxlib_adapter_tests BEFORE PRIVATE
	${CMAKE_CURRENT_SOURCE_DIR}/dxlib/stub
)
target_link_libraries(sgc_dxlib_adapter_tests PRIVATE sgc::sgc Catch2::Catch2WithMain)
catch_discover_tests(sgc_dxlib_adapter_tests)

# clang-tidy validation target (single TU including all public headers)
add_executable(sgc_tidy_check tidy/TidyAllHeaders.cpp)
target_link_libraries(sgc_tidy_check PRIVATE sgc::sgc)
