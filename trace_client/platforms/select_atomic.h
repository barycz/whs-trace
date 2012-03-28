#pragma once

#if defined WH_TARGET_WINDOWS
#	define USE_CRY_ENGINE	1
#	include <FrameWork/include/Threading/platforms/threadpool_cryengine.h>
#else
#	if defined WIN32 || defined WIN64
#		include "platforms/atomic_win.h"
#	else
#		include "platforms/atomic_gcc.h"
#	endif
#endif


