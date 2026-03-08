## サンプルプログラム

set(EXAMPLE_SOURCES
	ecs_basic.cpp
	serialize_roundtrip.cpp
	game_loop.cpp
	tween_animation.cpp
	state_machine.cpp
	event_system.cpp
	physics_sim.cpp
	raycast_demo.cpp
	config_manager.cpp
	object_pool.cpp
	behavior_tree.cpp
	scene_transition.cpp
	spatial_query.cpp
	coroutine_demo.cpp
	allocator_demo.cpp
	service_locator_demo.cpp
	profiler_demo.cpp
	math_showcase.cpp
)

foreach(src ${EXAMPLE_SOURCES})
	get_filename_component(name ${src} NAME_WE)
	add_executable(example_${name} ${src})
	target_link_libraries(example_${name} PRIVATE sgc::sgc)
endforeach()
