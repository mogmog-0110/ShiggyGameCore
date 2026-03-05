## サンプルプログラム

set(EXAMPLE_SOURCES
	ecs_basic.cpp
	serialize_roundtrip.cpp
	game_loop.cpp
)

foreach(src ${EXAMPLE_SOURCES})
	get_filename_component(name ${src} NAME_WE)
	add_executable(example_${name} ${src})
	target_link_libraries(example_${name} PRIVATE sgc::sgc)
endforeach()
