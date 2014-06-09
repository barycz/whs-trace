cmake_minimum_required (VERSION 2.6)
project (trace_client_dll)

set (CMAKE_VERBOSE_MAKEFILE on)

add_definitions("-DTRACE_ENABLED")
add_definitions("-DTRACE_CONFIG_INCLUDE=\"../tests/dll_project_simulator/user_config.h\"")
add_definitions("-DTRACE_LEVELS_INCLUDE=\"../tests/dll_project_simulator/user_levels.h\"")
add_definitions("-DTRACE_CONTEXTS_INCLUDE=\"../tests/dll_project_simulator/user_contexts.h\"")

add_subdirectory (dll1)
add_subdirectory (dll2)
add_subdirectory (trace_client)

include_directories (${TRACE_CLIENT_SOURCE_DIR}/trace_client)
link_directories (${TRACE_CLIENT_BINARY_DIR}/trace_client)

add_executable (main main.cpp)
if (MINGW)
	target_link_libraries (main trace_client "c:/devel/QtSDK/mingw/lib/libws2_32.a")
endif()
target_link_libraries (main trace_client user_dll_1 user_dll_2)

