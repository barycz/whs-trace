cmake_minimum_required(VERSION 2.8)
project(profile_server)

find_package(Qt4 REQUIRED)

include("${CMAKE_CURRENT_SOURCE_DIR}/FindReleaseAndDebug.cmake")

set (LIBRARY_OUTPUT_PATH
   ${PROJECT_BINARY_DIR}/Bin
   CACHE PATH
   "Single Directory for all Libraries"
   )

set (EXECUTABLE_OUTPUT_PATH
   ${PROJECT_BINARY_DIR}/Bin
   CACHE PATH
   "Single Directory for all Executables."
   )


add_definitions("-DPROFILE_ENABLED")
add_definitions("-DPROFILE_STATIC")

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/..)

find_package (Threads)

set(CMAKE_INCLUDE_PATH ${CMAKE_INCLUDE_PATH} "c:/devel/boost_1_48_0")
set(CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH} "c:/devel/boost_1_48_0")

set(Boost_DEBUG 1)
set(Boost_USE_STATIC_LIBS 1)
find_package(Boost)
if (Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIR})
    add_definitions( "-DHAS_BOOST" )
endif()

set(BOOST_LIBS thread date_time system regex)
find_package(Boost COMPONENTS ${BOOST_LIBS} REQUIRED)

find_package (DbgToolkit REQUIRED)
message ("Current dir: ${CMAKE_CURRENT_SOURCE_DIR}")
message ("DbgToolkit includes: ${DBGTOOLKIT_INCLUDE_DIRS}")
message ("DbgToolkit srcs: ${DBGTOOLKIT_SOURCE_DIRS}")

include_directories (${DBGTOOLKIT_INCLUDE_DIRS})

set(profile_server_SOURCES
	main.cpp
	mainwindow.cpp
    connection.cpp
	view.cpp
	mygraphicsview.cpp
	bar.cpp)

set(profile_server_HEADERS
	mainwindow.h 
    server.h
    connection.h
	view.h
	mygraphicsview.h
	bar.h)

#set(profile_server_UI mainwindow.ui settings.ui)
set(profile_server_RESOURCES resources.qrc)
set(QT_USE_QTNETWORK true)
set(QT_USE_QTOPENGL true)
if (WIN32)
	set (QT_USE_QTMAIN TRUE)
endif (WIN32) 

#qt4_wrap_ui(profile_server_UI_HDRS ${profile_server_UI})
qt4_wrap_cpp(profile_server_MOC ${profile_server_HEADERS})
qt4_add_resources(profile_server_RESOURCES_RCC ${profile_server_RESOURCES})
include(${QT_USE_FILE})
add_definitions(${QT_DEFINITIONS})
include_directories(${CMAKE_BINARY_DIR})

add_definitions(${QT_DEFINITIONS})
add_definitions(-DQT_PLUGIN)
add_definitions(-DQT_NO_DEBUG)
add_definitions(-DQT_STATIC)
add_definitions(-DSTATIC)
message("Plugin libs ${QT_PLUGINS_DIR}")


macro(QT_ADD_IMG_PLUGINS target)

  IF(QT_CONFIG MATCHES "static")
    add_definitions(-DQT_STATICPLUGIN)
   
    # Links against the static plugins
    FIND_RELEASE_AND_DEBUG(QJPEG qjpeg qjpegd "${QT_PLUGINS_DIR}/imageformats")
    FIND_RELEASE_AND_DEBUG(QGIF qgif qgifd "${QT_PLUGINS_DIR}/imageformats")
    FIND_RELEASE_AND_DEBUG(QMNG qmng qmngd "${QT_PLUGINS_DIR}/imageformats")
    FIND_RELEASE_AND_DEBUG(QICO qico qicod "${QT_PLUGINS_DIR}/imageformats")
	FIND_RELEASE_AND_DEBUG(QPNG qpng qpndd "${QT_PLUGINS_DIR}/imageformats")
    FIND_RELEASE_AND_DEBUG(QSVG qsvg qsvgd "${QT_PLUGINS_DIR}/imageformats")
    FIND_RELEASE_AND_DEBUG(QTIFF qtiff qtiffd "${QT_PLUGINS_DIR}/imageformats")
	 #FIND_RELEASE_AND_DEBUG(QSVGR QtSvg QtSvgd "C:/devel/QtSDK/QtSources/4.7.4/lib")		# @TODO: fixed path

	message("ico: ${QICO_LIBRARY}")
	message("svg: ${QSVG_LIBRARY}")
	message("svgr: ${QSVGR_LIBRARY}")
  
    target_link_libraries(${target} ${QJPEG_LIBRARY} ${QGIF_LIBRARY}
		${QMNG_LIBRARY} ${QICO_LIBRARY} ${QTIFF_LIBRARY} ${QSVG_LIBRARY} ${QSVGR_LIBRARY} ${QPNG_LIBRARY})
  
  endif(QT_CONFIG MATCHES "static")

endmacro(QT_ADD_IMG_PLUGINS)

message("Generated headers: ${profile_server_UI_HDRS}")
message("Generated files ${CMAKE_BINARY_DIR}")
message("Generated files profile_server_MOC ${profile_server_MOC}")
message("Generated files profile_server_UI_HDRS ${profile_server_UI_HDRS}")
message("Generated files profile_server_UI_SRCS ${profile_server_UI_SRCS}")
message("Generated files profile_server_RESOURCES_RCC ${profile_server_RESOURCES_RCC}")
message("Generated files profile_server_SOURCES ${profile_server_SOURCES}")

#add_library (fprof_client_lib STATIC "${DBGTOOLKIT_SOURCE_DIRS}/trace_client/profile.cpp")

add_executable(profile_server WIN32
		${profile_server_MOC}
		#		${profile_server_UI_HDRS}
		#${profile_server_UI_SRCS}
		${profile_server_RESOURCES_RCC}
		${profile_server_SOURCES})

qt_add_img_plugins(profile_server)
target_link_libraries(profile_server ${QT_LIBRARIES} ${QICO} ${QSVG} ${QPNG} ${CMAKE_THREAD_LIBS_INIT} ${Boost_LIBRARIES})



