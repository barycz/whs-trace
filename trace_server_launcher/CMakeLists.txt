cmake_minimum_required (VERSION 2.8)
project (trace_server_launcher)
set (CMAKE_MODULE_PATH "~/devel/DbgToolkit;${CMAKE_MODULE_PATH}")

find_package (DbgToolkit REQUIRED)
find_package (Threads)

message ("Current dir: ${CMAKE_CURRENT_SOURCE_DIR}")
message ("DbgToolkit includes: ${DBGTOOLKIT_INCLUDE_DIRS}")
message ("DbgToolkit srcs: ${DBGTOOLKIT_SOURCE_DIRS}")

add_definitions ("-DTRACE_ENABLED")
add_definitions ("-DTRACE_STATIC")
include_directories (${DBGTOOLKIT_INCLUDE_DIRS})
add_library (trace_client_lib STATIC "${DBGTOOLKIT_SOURCE_DIRS}/trace_client/trace.cpp")
add_executable (trace_server_launcher launcher.cpp main.cpp)
target_link_libraries (trace_server_launcher trace_client_lib ${CMAKE_THREAD_LIBS_INIT}) 
