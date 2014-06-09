# -----------------------------------------------------------------------------
# Find DirectX SDK
# Define:
# DirectX_FOUND
# DirectX_INCLUDE_DIR
# DirectX_LIBRARY
# DirectX_ROOT_DIR
set(DirectX_FOUND false)
if(WIN32) # The only platform it makes sense to check for DirectX SDK
	# We start with Microsoft DirectX SDK (August 2008) 9.24.1400
	# Example of path is C:\apps_x86\Microsoft DirectX SDK (August 2008)\Include
	find_path(DirectX_INCLUDE_DIR d3d9.h
		"$ENV{DXSDK_DIR}/Include"
		"C:/Program Files (x86)/Windows Kits/8.1/Include/shared"
		"C:/apps_x86/Microsoft DirectX SDK*/Include"
		"C:/Program Files (x86)/Microsoft DirectX SDK*/Include"
		"C:/apps/Microsoft DirectX SDK*/Include"
		"C:/Program Files/Microsoft DirectX SDK*/Include"
	)
	mark_as_advanced(DirectX_INCLUDE_DIR)

	#string(REGEX MATCH ".*8\\.1.*" DETECT_WIN81 "${DirectX_INCLUDE_DIR}")
	#message("dx ${DirectX_INCLUDE_DIR}")
	set(WIN81 False)
	if(DirectX_INCLUDE_DIR MATCHES "(.*).*8\\.1.*/(.*)")
		set(WIN81 True)
		message("Win 8.1 detected")
	endif()

	if(WIN81)
		if(DirectX_INCLUDE_DIR)
			set(DirectX_ROOT_DIR "${DirectX_INCLUDE_DIR}/../..")
		endif(DirectX_INCLUDE_DIR)
	else(WIN81)
		if(DirectX_INCLUDE_DIR)
			set(DirectX_ROOT_DIR "${DirectX_INCLUDE_DIR}/..")
		endif(DirectX_INCLUDE_DIR)
	endif(WIN81)

	# dlls are in DirectX_ROOT_DIR/Developer Runtime/x64|x86
	# lib files are in DirectX_ROOT_DIR/Lib/x64|x86
	set(DirectX_LIBRARY_PATHS)

	if(WIN81)
		set(DirectX_LIBRARY_PATHS "${DirectX_ROOT_DIR}/Lib/winv6.3/um/x64")
	else(WIN81)
		if(CMAKE_CL_64)
			set(DirectX_LIBRARY_PATHS "${DirectX_ROOT_DIR}/Lib/x64")
		else(CMAKE_CL_64)
			set(DirectX_LIBRARY_PATHS "${DirectX_ROOT_DIR}/Lib/x86" "${DirectX_ROOT_DIR}/Lib")
		endif(CMAKE_CL_64)
	endif(WIN81)

	find_library(DirectX_LIBRARY d3d9 ${DirectX_LIBRARY_PATHS} NO_DEFAULT_PATH)

	if(DirectX_INCLUDE_DIR AND DirectX_LIBRARY)
		set(DirectX_FOUND true)
	endif(DirectX_INCLUDE_DIR AND DirectX_LIBRARY)
	mark_as_advanced(DirectX_LIBRARY)

	message("DirectX_FOUND=${DirectX_FOUND}")
	message("DirectX_INCLUDE_DIR=${DirectX_INCLUDE_DIR}")
	message("DirectX_LIBRARY=${DirectX_LIBRARY}")
	message("DirectX_ROOT_DIR=${DirectX_ROOT_DIR}")
endif(WIN32)
