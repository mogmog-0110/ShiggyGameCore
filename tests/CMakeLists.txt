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
	math/TestVectorOps.cpp
	math/TestBezier.cpp
	math/TestSpline.cpp
	math/TestExpressionTree.cpp
	math/TestNodeGraph.cpp

	# Patterns
	patterns/TestEventDispatcher.cpp
	patterns/TestStateMachine.cpp
	patterns/TestObjectPool.cpp
	patterns/TestCommand.cpp
	patterns/TestObserver.cpp
	patterns/TestServiceLocator.cpp
	patterns/TestStrategy.cpp
	patterns/TestFactory.cpp
	patterns/TestDecorator.cpp

	# Animation
	animation/TestTween.cpp
	animation/TestTweenTimeline.cpp
	animation/TestTweenCallbacks.cpp

	# Spatial
	spatial/TestGrid.cpp
	spatial/TestQuadtree.cpp
	spatial/TestOctree.cpp

	# AI
	ai/TestBehaviorTree.cpp
	ai/TestUtilityAI.cpp
	ai/TestGOAP.cpp
	ai/TestAStar.cpp

	# ECS
	ecs/TestECS.cpp
	ecs/TestSystem.cpp
	ecs/TestCommandBuffer.cpp
	ecs/TestViewExclude.cpp
	ecs/TestSystemTraits.cpp
	ecs/TestEntityTemplate.cpp
	ecs/TestMessageBus.cpp

	# Graphics
	graphics/TestRenderer.cpp
	graphics/TestTextMeasure.cpp
	graphics/TestTextRenderer.cpp

	# Graphics3D
	graphics3d/TestCamera3D.cpp
	graphics3d/TestMeshFactory.cpp
	graphics3d/TestMaterial.cpp
	graphics3d/TestRenderer3D.cpp

	# Input
	input/TestActionMap.cpp
	input/TestInputBuffer.cpp
	input/TestInputProvider.cpp
	input/TestInputModeManager.cpp
	input/TestModeAwareInput.cpp
	input/TestGamepadProvider.cpp

	# Effects
	effects/TestParticleSystem.cpp
	effects/TestEmitterShape.cpp
	effects/TestAffectorPresets.cpp
	effects/TestParticlePool.cpp

	# Resource
	resource/TestResourceManager.cpp
	resource/TestFileWatcher.cpp
	resource/TestHotReloader.cpp

	# Scene
	scene/TestScene.cpp
	scene/TestSceneIntegration.cpp
	scene/TestAppBase.cpp
	scene/TestTransform.cpp
	scene/TestCamera.cpp
	scene/TestTransition.cpp

	# Config
	config/TestConfigManager.cpp

	# Containers
	containers/TestBitSet.cpp
	containers/TestSparseSet.cpp
	containers/TestFreeList.cpp
	containers/TestPriorityQueue.cpp
	containers/TestSpatialHashMap.cpp

	# Debug
	debug/TestFpsCounter.cpp
	debug/TestDebugOverlay.cpp
	debug/TestMemoryStats.cpp
	debug/TestTweakable.cpp

	# Net
	net/TestMessageChannel.cpp
	net/TestStateSync.cpp

	# Network
	network/TestNetworkEntity.cpp
	network/TestPredictionBuffer.cpp
	network/TestNetworkCodec.cpp

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
	ui/TestToggleSwitch.cpp
	ui/TestRadioButton.cpp
	ui/TestPanel.cpp
	ui/TestBadge.cpp
	ui/TestStackLayout.cpp
	ui/TestTooltip.cpp
	ui/TestScrollbar.cpp
	ui/TestToast.cpp
	ui/TestTextLayout.cpp
	ui/TestFontSizeSnap.cpp
	ui/TestModal.cpp
	ui/TestTabBar.cpp
	ui/TestListView.cpp
	ui/TestComboBox.cpp
	ui/TestTextInput.cpp
	ui/TestTable.cpp
	ui/TestTreeView.cpp
	ui/TestColorPicker.cpp
	ui/TestNumericInput.cpp
	ui/TestRangeSlider.cpp
	ui/TestBreadcrumb.cpp
	ui/TestBlimLayout.cpp
	ui/TestFlexLayout.cpp
	ui/TestResponsiveLayout.cpp

	# Audio
	audio/TestAudioPlayer.cpp
	audio/TestAudioBus.cpp
	audio/TestSpatialAudio.cpp

	# Physics
	physics/TestFixedTimestep.cpp
	physics/TestCollisionUtils.cpp
	physics/TestAABB2DCollision.cpp
	physics/TestRigidBody2D.cpp
	physics/TestRayCast2D.cpp
	physics/TestCollisionShape3D.cpp
	physics/TestRigidBody3D.cpp
	physics/TestRayCast3D.cpp
	physics/TestForceField.cpp

	# Data
	data/TestDataNode.cpp
	data/TestJsonConfig.cpp
	data/TestBinarySerializer.cpp

	# i18n
	i18n/TestStringTable.cpp
	i18n/TestLanguageManager.cpp
	i18n/TestPluralRules.cpp
	i18n/TestDateTimeFormat.cpp
	i18n/TestTextDirection.cpp

	# Tilemap
	tilemap/TestTilemap.cpp
	tilemap/TestAutoTile.cpp

	# Dialogue
	dialogue/TestDialogueSystem.cpp
	dialogue/TestDialogueBuilder.cpp

	# Save
	save/TestSaveSystem.cpp
	save/TestSaveMigration.cpp

	# Procedural
	procedural/TestDungeonGenerator.cpp
	procedural/TestTerrainGenerator.cpp

	# Replay
	replay/TestReplaySystem.cpp

	# Asset
	asset/TestAssetPipeline.cpp
	asset/TestSpriteSheet.cpp

	# Scripting
	scripting/TestScriptBindings.cpp

	# ImGui
	imgui/TestImGuiAdapter.cpp

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
	dxlib/TestDxLibTextMeasure.cpp
)
target_include_directories(sgc_dxlib_adapter_tests BEFORE PRIVATE
	${CMAKE_CURRENT_SOURCE_DIR}/dxlib/stub
)
target_link_libraries(sgc_dxlib_adapter_tests PRIVATE sgc::sgc Catch2::Catch2WithMain)
catch_discover_tests(sgc_dxlib_adapter_tests)

# Siv3D adapter tests (with stub Siv3D.hpp)
add_executable(sgc_siv3d_adapter_tests
	siv3d/TestSiv3DTypeConvert.cpp
	siv3d/TestSiv3DRenderer.cpp
	siv3d/TestSiv3DTextRenderer.cpp
	siv3d/TestSiv3DInputProvider.cpp
	siv3d/TestSiv3DAudioPlayer.cpp
	siv3d/TestSiv3DSceneAdapter.cpp
	siv3d/TestSiv3DGamepadProvider.cpp
)
target_include_directories(sgc_siv3d_adapter_tests BEFORE PRIVATE
	${CMAKE_CURRENT_SOURCE_DIR}/siv3d/stub
)
target_link_libraries(sgc_siv3d_adapter_tests PRIVATE sgc::sgc Catch2::Catch2WithMain)
catch_discover_tests(sgc_siv3d_adapter_tests)

# clang-tidy validation target (single TU including all public headers)
add_executable(sgc_tidy_check tidy/TidyAllHeaders.cpp)
target_link_libraries(sgc_tidy_check PRIVATE sgc::sgc)
