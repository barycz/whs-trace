cmake_minimum_required (VERSION 2.8)
project (trace_client_DLL)

set (CMAKE_MODULE_PATH "w:/code/externals_code/dbgtoolkit/src/;${CMAKE_MODULE_PATH}")

find_package (DbgToolkit REQUIRED)
find_package (Threads)

message ("Current dir: ${CMAKE_CURRENT_SOURCE_DIR}")
message ("DbgToolkit includes: ${DBGTOOLKIT_INCLUDE_DIRS}")
message ("DbgToolkit srcs: ${DBGTOOLKIT_SOURCE_DIRS}")
message ("bin: ${CMAKE_BINARY_DIR}")
message ("bin2: ${CMAKE_CURRENT_BINARY_DIR}")
message ("bin3: ${PROJECT_BINARY_DIR}/${CMAKE_BUILD_TYPE}")

#add_definitions ("-DTRACE_CONFIG_INCLUDE=\"${CMAKE_CURRENT_SOURCE_DIR}/wh_config.h\"")
#add_definitions ("-DTRACE_LEVELS_INCLUDE=\"${CMAKE_CURRENT_SOURCE_DIR}/wh_levels.h\"")
#add_definitions ("-DTRACE_CONTEXTS_INCLUDE=\"${CMAKE_CURRENT_SOURCE_DIR}/wh_contexts.h\"")
add_definitions ("-DTRACE_ENABLED")
add_definitions ("-DTRACE_DLL")
add_definitions ("-DTRACE_WINDOWS_SOCKET_FAILOVER_TO_FILE")

add_definitions ("-DPROFILE_ENABLED")
add_definitions ("-DPROFILE_DLL")

include_directories (${DBGTOOLKIT_INCLUDE_DIRS})
add_library (trace_client_x64 SHARED "${DBGTOOLKIT_SOURCE_DIRS}/trace_client/trace.cpp" "${DBGTOOLKIT_SOURCE_DIRS}/trace_client/profile.cpp")


install(TARGETS trace_client_x64
		RUNTIME DESTINATION bin
		LIBRARY DESTINATION lib
		ARCHIVE DESTINATION lib)
INSTALL(FILES "${CMAKE_BINARY_DIR}/RelWithDebInfo/trace_client_x64.pdb" DESTINATION bin)
#INSTALL(FILES "${CPACK_BUILD_DIR}/${CMAKE_BUILD_TYPE}/trace_client_x64.pdb" DESTINATION bin)
