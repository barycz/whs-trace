cmake_minimum_required (VERSION 2.8)
project (profile_client)
set (CMAKE_MODULE_PATH "~/devel/DbgToolkit;${CMAKE_MODULE_PATH}")

find_package (DbgToolkit REQUIRED)
find_package (Threads)

message ("Current dir: ${CMAKE_CURRENT_SOURCE_DIR}")
message ("DbgToolkit includes: ${DBGTOOLKIT_INCLUDE_DIRS}")
message ("DbgToolkit srcs: ${DBGTOOLKIT_SOURCE_DIRS}")

add_definitions ("-DPROFILE_ENABLED")
add_definitions ("-DPROFILE_STATIC")
include_directories (${DBGTOOLKIT_INCLUDE_DIRS})
add_library (fprof_client_lib STATIC "${DBGTOOLKIT_SOURCE_DIRS}/trace_client/profile.cpp" "${DBGTOOLKIT_SOURCE_DIRS}/trace_client/trace.cpp")

add_executable (profile_client profile_client.cpp)

if (MINGW)
	target_link_libraries (profile_client fprof_client_lib "c:/devel/QtSDK/mingw/lib/libws2_32.a")
endif()
target_link_libraries (profile_client fprof_client_lib ${CMAKE_THREAD_LIBS_INIT}) 
