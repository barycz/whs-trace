cmake_minimum_required (VERSION 2.8)
project (simple_client)
#set (CMAKE_MODULE_PATH "~/devel/DbgToolkit;${CMAKE_MODULE_PATH}")

find_package (DbgToolkit REQUIRED)
find_package (Threads)

message ("Current dir: ${CMAKE_CURRENT_SOURCE_DIR}")
message ("DbgToolkit includes: ${DBGTOOLKIT_INCLUDE_DIRS}")
message ("DbgToolkit srcs: ${DBGTOOLKIT_SOURCE_DIRS}")

add_definitions ("-DTRACE_CONFIG_INCLUDE=\"${CMAKE_CURRENT_SOURCE_DIR}/sample_config.h\"")
add_definitions ("-DTRACE_LEVELS_INCLUDE=\"${CMAKE_CURRENT_SOURCE_DIR}/sample_levels.h\"")
add_definitions ("-DTRACE_CONTEXTS_INCLUDE=\"${CMAKE_CURRENT_SOURCE_DIR}/sample_contexts.h\"")
add_definitions ("-DTRACE_ENABLED")
add_definitions ("-DTRACE_STATIC")
include_directories (${DBGTOOLKIT_INCLUDE_DIRS})
add_library (trace_client_lib STATIC "${DBGTOOLKIT_SOURCE_DIRS}/trace_client/trace.cpp")

add_executable (simple_client main.cpp)

if (MINGW)
	target_link_libraries (simple_client trace_client "c:/devel/QtSDK/mingw/lib/libws2_32.a")
endif()
target_link_libraries (simple_client trace_client_lib ${CMAKE_THREAD_LIBS_INIT}) 
