cmake_minimum_required(VERSION 2.8.9)
project(trace_server)
set (CMAKE_VERBOSE_MAKEFILE on)
find_package(Qt5Widgets REQUIRED)
find_package(Qt5Core REQUIRED)
find_package(Qt5Network REQUIRED)
find_package(Qt5OpenGL REQUIRED)
find_package(Qt5Script REQUIRED)
find_program(QT_QMAKE_EXECUTABLE NAMES qmake qmake5 qmake-qt5 qmake-mac PATHS ${QTROOTS})
message("qmake: ${QT_QMAKE_EXECUTABLE}")

get_filename_component(QT_QMAKE_PATH ${QT_QMAKE_EXECUTABLE} PATH)
get_filename_component(QT_PLUGINS_PATH "${QT_QMAKE_PATH}/../plugins" ABSOLUTE)
get_filename_component(QT_LIB_PATH "${QT_QMAKE_PATH}/../lib" ABSOLUTE)
message("qt lib path: ${QT_LIB_PATH}")
message("qt plugins path: ${QT_PLUGINS_PATH}")

set(CMAKE_AUTOMOC ON)
set(CMAKE_PREFIX_PATH "${QTDIR}; ${WINSDK_LIB}")

include("${CMAKE_CURRENT_SOURCE_DIR}/FindPlugins.cmake")

set(LIBRARY_OUTPUT_PATH
   ${PROJECT_BINARY_DIR}/Bin
   CACHE PATH
   "Single Directory for all Libraries"
   )

set(EXECUTABLE_OUTPUT_PATH
   ${PROJECT_BINARY_DIR}/Bin
   CACHE PATH
   "Single Directory for all Executables."
   )

add_subdirectory("qwt")
add_subdirectory("qtsln")

add_definitions("-DTRACE_ENABLED")
add_definitions("-DTRACE_STATIC")

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/..)

set(Boost_DEBUG 0)
set(Boost_USE_STATIC_LIBS 1)
find_package(Boost)
if (Boost_FOUND)
	include_directories(${Boost_INCLUDE_DIR})
	add_definitions( "-DHAS_BOOST" )

	set(BOOST_LIBS thread date_time system regex serialization)
	find_package(Boost COMPONENTS ${BOOST_LIBS} REQUIRED)
endif()
message("boost: ${Boost_LIBRARIES}")


set(trace_server_SOURCES
	main.cpp
	qt_plugins.cpp
	mainwindow.cpp
	mainwindow_presets.cpp
	mainwindow_controlwidget.cpp
	mainwindow_connection.cpp
	mainwindow_docking.cpp
	mainwindow_find.cpp
	tableview.cpp
	treeview.cpp
	treeproxy.cpp
	delegates.cpp
	server.cpp
	server_filtering.cpp
	filterwidget.cpp
	connection.cpp
	connectionconfig.cpp
	connection_stream.cpp
	connection_setup.cpp
	connection_controlwidget.cpp
	connection_csv.cpp
	filterstate.cpp
	filterstate_serialize.cpp
	constants.cpp
	config.cpp
	dock.cpp
	dockwidget.cpp
	dockdelegates.cpp
	dockmanagerconfig.cpp
	dockmanager.cpp
	dockmanagerview.cpp
	dockmanagermodel.cpp
	syncwidgets.cpp
	combolist.cpp
	movabletabwidget.cpp
	timecombobox.cpp
	buttoncache.cpp
	controlbarcommon.cpp
	controlbardockedwidgets.cpp
	controlbarlog.cpp
	basetablemodel.cpp

	filterbase.cpp
	filtermgrbase.cpp
	filtermgr.cpp
	filter_script.cpp
	filter_string.cpp
	filter_regex.cpp
	filter_ctx.cpp
	filter_lvl.cpp
	filter_tid.cpp
	filter_fileline.cpp
	filter_row.cpp
	filter_time.cpp
	#filter_function.cpp
	#filter_dt.cpp
	colorizermgr.cpp
	#colorizer_script.cpp
	#colorizer_string.cpp
	colorizer_regex.cpp
	#colorizer_ctx.cpp
	#colorizer_lvl.cpp
	#colorizer_tid.cpp
	#colorizer_fileline.cpp
	colorizer_row.cpp
	#colorizer_time.cpp
	#colorizer_function.cpp
	#colorizer_dt.cpp

	warnimage.cpp
	colorizer_regex.cpp

	findwidget.cpp
	findconfig.cpp

	kde/kselectionproxymodel.cpp
	kde/kmodelindexproxymapper.cpp
	kde/klinkitemselectionmodel.cpp

	connection_log.cpp
	logs/logtablemodel.cpp
	logs/logtableview.cpp
	logs/filterproxymodel.cpp
	logs/findproxymodel.cpp
	logs/logselectionproxymodel.cpp
	logs/logwidget.cpp
	logs/logwidget_find.cpp
	logs/logwidget_filtering.cpp
	logs/logwidget_legacycfg.cpp
	logs/logwidget_actions.cpp
	logs/logwidget_view.cpp
	logs/logconfig.cpp
	logs/logctxmenu.cpp
	logs/logwidget_tools.cpp
	logs/logdelegate.cpp

	connection_plot.cpp
	plot/plotwidget.cpp
	plot/plotconfig.cpp
	plot/plottypes.cpp

	connection_table.cpp
	table/editableheaderview.cpp
	table/baseproxymodel.cpp
	table/sparseproxymodel.cpp
	table/tableconfig.cpp
	table/tablemodel.cpp
	table/tablewidget.cpp

	connection_gantt.cpp
	connection_frame.cpp
	gantt/arrow.cpp
	gantt/scrollzoomer.cpp
	gantt/scrollbar.cpp
	gantt/frameview.cpp
	gantt/frameviewconfig.cpp
	gantt/ganttconfig.cpp
	gantt/ganttitem.cpp
	gantt/ganttview.cpp
	gantt/ganttwidget.cpp
	gantt/histogram.cpp
	)

set(trace_server_HEADERS
	mainwindow.h
	tableview.h
	treeview.h
	treemodel.h
	treeproxy.h
	server.h
	connection.h
	../tlv_parser/tlv_parser.h
	../tlv_parser/tlv_encoder.h
	../tlv_parser/tlv_decoder.h
	../filters/nnode.hpp
	../filters/file_filter.hpp
	filterwidget.h
	filterstate.h
	delegates.h
	rendezvous.h
	utils.h
	dock.h
	dockwidget.h
	dockdelegates.h
	dockedwidgets.h
	dockmanagerconfig.h
	dockmanager.h
	dockmanagermodel.h
	dockmanagerview.h
	constants.h
	syncwidgets.h
	config.h
	combolist.h
	timecombobox.h
	movablelistmodel.h
	movabletabwidget.h
	warnimage.h
	buttoncache.h
	controlbarcommon.h
	controlbardockedwidgets.h
	controlbarlog.h
	basetablemodel.h

	findwidget.h
	findconfig.h

	filterbase.h
	filtermgrbase.h
	filtermgr.h
	filter_script.h
	filter_string.h
	filter_regex.h
	filter_ctx.h
	filter_lvl.h
	filter_tid.h
	filter_fileline.h
	filter_row.h
	filter_time.h
	#filter_function.h
	#filter_dt.h
	#colorizer_script.h
	#colorizer_string.h
	colorizermgr.h
	colorizer_regex.h
	colorizer_row.h
	#colorizer_ctx.h
	#colorizer_lvl.h
	#colorizer_tid.h
	#colorizer_fileline.h
	#colorizer_row.h
	#colorizer_time.h
	#colorizer_function.h
	#colorizer_dt.h


	logs/logtablemodel.h
	logs/logtableview.h
	logs/logselectionproxymodel.h
	logs/logwidget.h
	logs/logconfig.h
	logs/logctxmenu.h
	logs/filterproxymodel.h
	logs/findproxymodel.h
	logs/logdelegate.h

	kde/kbihash_p.h
	kde/kmodelindexproxymapper.h
	kde/kselectionproxymodel.h
	kde/kvoidpointerfactory_p.h
	kde/klinkitemselectionmodel.h

	plot/curves.h
	plot/plotwidget.h
	plot/plotconfig.h
	plot/plotctxmenu.h

	table/editableheaderview.h
	table/baseproxymodel.h
	table/sparseproxymodel.h
	table/tableconfig.h
	table/tablectxmenu.h
	table/tablemodel.h
	table/tablewidget.h

	gantt/arrow.h
	gantt/frameview.h
	gantt/frameviewconfig.h
	gantt/frameviewctxmenu.h
	gantt/scrollzoomer.h
	gantt/scrollbar.h
	gantt/ganttconfig.h
	gantt/ganttctxmenu.h
	gantt/ganttdata.h
	gantt/ganttitem.h
	gantt/ganttview.h
	gantt/ganttwidget.h
	gantt/histogram.h
	gantt/scalewidget.h
	)

set(trace_server_UI
	mainwindow.ui
	controlbarcommon.ui
	controlbardockedwidgets.ui
	controlbarlog.ui
	filterwidget.ui
	findwidget.ui
	combolist.ui
	timecombobox.ui
	settings.ui
	settingslog.ui
	settingsplot.ui
	settingstable.ui
	settingsgantt.ui
	settingsframeview.ui
	help.ui
	filter_string.ui
	filter_regex.ui
	filter_script.ui
	filter_tid.ui
	filter_lvl.ui
	filter_ctx.ui
	filter_fileline.ui
	filter_row.ui
	filter_time.ui
	colorizer_regex.ui
	colorizer_row.ui
	)

set(trace_server_RESOURCES resources.qrc)

qt5_wrap_ui(trace_server_UI_HDRS ${trace_server_UI})
qt5_wrap_cpp(trace_server_MOC ${trace_server_HEADERS})
qt5_add_resources(trace_server_RESOURCES_RCC ${trace_server_RESOURCES})

add_executable(trace_server WIN32
		${trace_server_UI_HDRS}
		${trace_server_UI_SRCS}
		${trace_server_RESOURCES_RCC}
		${trace_server_SOURCES})


include_directories(${QT_USE_FILE} ${Qt5Gui_PRIVATE_INCLUDE_DIRS})
add_definitions(${QT_DEFINITIONS})
include_directories(${CMAKE_BINARY_DIR})
include_directories(${trace_server_SOURCE_DIR})

add_definitions(${QT_DEFINITIONS})
add_definitions(-DQT_PLUGIN)
add_definitions(-DQT_NO_DEBUG)
add_definitions(-DQT_STATIC)
add_definitions(-DSTATIC)

####################### Platform specific ######################
if (WIN32)
	set (QT_USE_QTMAIN TRUE)
	add_definitions(-DNOMINMAX)
	add_definitions(-DWIN_LEAN_AND_MEAN)
	target_link_libraries(trace_server Qt5::WinMain)
endif (WIN32)
####################### Platform specific ######################


######################## Plugin section ########################
# Locate static plugins
add_definitions(-DQT_STATICPLUGIN)
get_filename_component(QT_IMG_PLUGINS_PATH "${QT_PLUGINS_PATH}/imageformats" ABSOLUTE)
get_filename_component(QT_PLATFORM_PLUGINS_PATH "${QT_PLUGINS_PATH}/platforms" ABSOLUTE)

FIND_PLUGINS(QICO          NAMES qico                   DEFAULT_SUFFIXES PATHS ${QT_IMG_PLUGINS_PATH})
message("plugin ico: ${QICO_LIBRARIES}")
target_link_libraries(trace_server ${QICO_LIBRARIES})

if (WIN32)
	FIND_PLUGINS(QWINDOWS      NAMES qwindows               DEFAULT_SUFFIXES PATHS ${QT_PLATFORM_PLUGINS_PATH})
	FIND_PLUGINS(QPLATFORM     NAMES Qt5PlatformSupport     DEFAULT_SUFFIXES PATHS ${QT_LIB_PATH})
	FIND_PLUGINS(QTRANSLATOR   NAMES translator             DEFAULT_SUFFIXES PATHS ${QT_LIB_PATH})
	FIND_PLUGINS(QTRANS_COMMON NAMES translator_common      DEFAULT_SUFFIXES PATHS ${QT_LIB_PATH})
	FIND_PLUGINS(QTRANS_HLSL   NAMES translator_hlsl        DEFAULT_SUFFIXES PATHS ${QT_LIB_PATH})
	FIND_PLUGINS(QPREPROC      NAMES preprocessor           DEFAULT_SUFFIXES PATHS ${QT_LIB_PATH})
	FIND_LIBRARY(DXGUID_LIBRARY dxguid)
	FIND_LIBRARY(D3DCOMPILER_LIBRARY d3dcompiler)
	message("plugin win: ${QWINDOWS_LIBRARIES}")
	message("dx: ${DXGUID_LIBRARY} ${D3DCOMPILER_LIBRARY}")

	if(QTRANSLATOR_LIBRARY)
		set(TRANSLATOR_LIBRARIES ${TRANSLATOR_LIBRARIES} ${QTRANSLATOR_LIBRARY})
	endif()
	if(QTRANS_COMMON_LIBRARY)
		set(TRANSLATOR_LIBRARIES ${TRANSLATOR_LIBRARIES} ${QTRANS_COMMON_LIBRARY})
	endif()
	if(QTRANS_HLSL_LIBRARY)
		set(TRANSLATOR_LIBRARIES ${TRANSLATOR_LIBRARIES} ${QTRANS_HLSL_LIBRARY})
	endif()

	target_link_libraries(trace_server
		wsock32 ws2_32
		imm32 winmm d3d9 ${DXGUID_LIBRARY} ${D3DCOMPILER_LIBRARY}
		${QPLATFORM_LIBRARY}
		${TRANSLATOR_LIBRARIES}
		${QPREPROC_LIBRARY}
		${QWINDOWS_LIBRARY})
elseif (UNIX)
	message("not implemented yet, but should be trivial to do so")
elseif (APPLE)
	message("not implemented yet, but should be trivial to do so")
endif()
######################## Plugin section ########################

qt5_use_modules(trace_server Widgets Core Gui Network OpenGL Script)

target_link_libraries(trace_server ${Boost_LIBRARIES}
	qwt
	qtsln
	${Qt5Gui_EGL_LIBRARIES}
	${Qt5Gui_OPENGL_LIBRARIES}
	${Qt5Widgets_LIBRARIES}
	${QICO}
	Qt5::WinMain)

message("src_dir: ${CMAKE_CURRENT_SOURCE_DIR}")

if (WIN32 AND NOT MINGW)
	find_program(GIT_EXECUTABLE
		NAMES "git.exe"
		HINTS "C:/Program Files (x86)/git/bin/"
		DOC "git x86 command line client")

	if (NOT GIT_FOUND)
		find_program(GIT_EXECUTABLE
			NAMES "git.exe"
			HINTS "C:/Program Files/git/bin/"
			DOC "git x64 command line client")
	endif (NOT GIT_FOUND)

	message("git: ${GIT_EXECUTABLE}")
endif (WIN32 AND NOT MINGW)

if (WIN32 AND NOT MINGW)
	get_filename_component(MK_VERSION_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../build/mk_version.bat" ABSOLUTE)
	message("mk_version.bat: ${MK_VERSION_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/")
	add_custom_command(TARGET trace_server PRE_BUILD
		COMMAND cmd /c ${MK_VERSION_PATH} ${GIT_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/)
endif (WIN32 AND NOT MINGW)



############################# debug ############################
#message("Generated headers: ${trace_server_UI_HDRS}")
#message("Generated files ${CMAKE_BINARY_DIR}")
#message("Generated files trace_server_MOC ${trace_server_MOC}")
#message("Generated files trace_server_UI_HDRS ${trace_server_UI_HDRS}")
#message("Generated files trace_server_UI_SRCS ${trace_server_UI_SRCS}")
#message("Generated files trace_server_RESOURCES_RCC ${trace_server_RESOURCES_RCC}")
#message("Generated files trace_server_SOURCES ${trace_server_SOURCES}")

#message("Debug variable dump:")
#get_cmake_property(_variableNames VARIABLES)
#foreach (_variableName ${_variableNames})
#	message(STATUS "${_variableName}=${${_variableName}}")
#endforeach()
