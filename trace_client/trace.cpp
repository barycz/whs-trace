#include "trace.h"
#include <stdarg.h>
#include <sysfn/time_query.h>
#include <list>
#include <algorithm>

	namespace sys {
		hptimer_t g_Start = 0, g_Freq = 1000000;
	}

#if defined TRACE_ENABLED
	namespace trace {

		level_t      g_RuntimeLevel       = static_cast<level_t>(e_max_trace_level);
		context_t    g_RuntimeContextMask = ~(0U);
		char const * g_AppName            = "trace_client";
		bool         g_RuntimeBuffering   = true;

		typedef std::list<I_TraceCallback*> T_Callbacks;
		T_Callbacks	g_Callbacks;


		// setup and utils
		void SetAppName (char const * name) { g_AppName = name; }
		char const * GetAppName () { return g_AppName; }

		void SetRuntimeLevel (level_t level) { g_RuntimeLevel = level; }
		level_t GetRuntimeLevel () { return g_RuntimeLevel; }

		void SetRuntimeBuffering (bool buffered) { g_RuntimeBuffering = buffered; }
		bool GetRuntimeBuffering () { return g_RuntimeBuffering; }

		void SetRuntimeContextMask (context_t mask) { g_RuntimeContextMask = mask; }
		context_t GetRuntimeContextMask () { return g_RuntimeContextMask; }

		inline void SetCustomUserDictionnary (CtxDictPair const * ptr, size_t n);
		inline void SetCustomUserDictionnary (LvlDictPair const * ptr, size_t n);
		void SetCustomUserDictionnary ()
		{
			{
				CtxDictPair const * ptr = 0;
				size_t const n = getContextDictionnary(ptr);
				SetCustomUserDictionnary(ptr, n);
			}

			{
				LvlDictPair const * ptr = 0;
				size_t const n = getLevelDictionnary(ptr);
				SetCustomUserDictionnary(ptr, n);
			}
		}

		void AddCallback (I_TraceCallback* pCallback)
		{
			if(std::find(g_Callbacks.begin(), g_Callbacks.end(), pCallback) == g_Callbacks.end())
			{
				g_Callbacks.push_back(pCallback);
			}
		}

		void RemoveCallback (I_TraceCallback* pCallback)
		{
			g_Callbacks.remove(pCallback);
		}

		// message logging
		void WriteLog (level_t level, context_t context, char const * file, int, char const *, char const *, va_list);
		void WriteVA (level_t level, context_t context, char const * file, int line, char const * fn, char const * fmt, va_list args)
		{
			if (RuntimeFilterPredicate(level, context))
				WriteLog(level, context, file, line, fn, fmt, args);

			for(T_Callbacks::iterator it = g_Callbacks.begin(); it != g_Callbacks.end(); ++it)
			{
				(*it)->Write(level, context, file, line, fn, fmt, args);
			}
		}

		void Write (level_t level, context_t context, char const * file, int line, char const * fn, char const * fmt, ...)
		{
			va_list args;
			va_start(args, fmt);
			WriteVA(level, context, file, line, fn, fmt, args);
			va_end(args);
		}

		inline void WriteScopeVA (ScopedLog::E_Type type, level_t level, context_t context, char const * file, int line, char const * fn, char const * fmt, va_list args);
		inline void WriteScope (ScopedLog::E_Type type, level_t level, context_t context, char const * file, int line, char const * fn, char const * fmt, ...)
		{
			va_list args;
			va_start(args, fmt);
			WriteScopeVA(type, level, context, file, line, fn, fmt, args);
			va_end(args);
		}
		ScopedLog::ScopedLog (level_t level, context_t context, char const * file, int line, char const * fn, char const * fmt, ...)
			: m_level(level), m_context(context), m_file(file), m_line(line), m_fn(fn), m_start(sys::queryTime_us())
		{
			if (RuntimeFilterPredicate(level, context))
			{
				va_list args;
				va_start(args, fmt);
				WriteScopeVA(e_Entry, level, context, file, line, fn, fmt, args);
				va_end(args);
			}
		}
		ScopedLog::~ScopedLog ()
		{
			if (RuntimeFilterPredicate(m_level, m_context))
				WriteScope(e_Exit, m_level, m_context, m_file, m_line, m_fn, "dt=%llu", sys::queryTime_us() - m_start);
		}


		// plot-data logging
		inline void WritePlot_impl (level_t level, context_t context, float x, float y, char const * fmt, va_list args);
		void WritePlotVA (level_t level, context_t context, float x, float y, char const * fmt, va_list args)
		{
			if (RuntimeFilterPredicate(level, context))
				WritePlot_impl(level, context, x, y, fmt, args);
		}
		void WritePlot (level_t level, context_t context, float x, float y, char const * fmt, ...)
		{
			va_list args;
			va_start(args, fmt);
			WritePlotVA(level, context, x, y, fmt, args);
			va_end(args);
		}

		inline void WritePlotClear_impl (level_t level, context_t context, char const * fmt, va_list args);
		void WritePlotClearVA (level_t level, context_t context, char const * fmt, va_list args)
		{
			if (RuntimeFilterPredicate(level, context))
				WritePlotClear_impl(level, context, fmt, args);
		}
		void WritePlotClear (level_t level, context_t context, char const * fmt, ...)
		{
			va_list args;
			va_start(args, fmt);
			WritePlotClearVA(level, context, fmt, args);
			va_end(args);
		}


		// table-data logging
		inline void WriteTable_impl (level_t level, context_t context, int x, int y, char const * fmt, va_list args);
		void WriteTableVA (level_t level, context_t context, int x, int y, char const * fmt, va_list args)
		{
			if (RuntimeFilterPredicate(level, context))
				WriteTable_impl(level, context, x, y, fmt, args);
		}
		void WriteTable (level_t level, context_t context, int x, int y, char const * fmt, ...)
		{
			va_list args;
			va_start(args, fmt);
			WriteTableVA(level, context, x, y, fmt, args);
			va_end(args);
		}

		inline void WriteTable_impl (level_t level, context_t context, int x, int y, Color c, char const * fmt, va_list args);
		void WriteTableVA (level_t level, context_t context, int x, int y, Color c, char const * fmt, va_list args)
		{
			if (RuntimeFilterPredicate(level, context))
				WriteTable_impl(level, context, x, y, c, fmt, args);
		}
		void WriteTable (level_t level, context_t context, int x, int y, Color c, char const * fmt, ...)
		{
			va_list args;
			va_start(args, fmt);
			WriteTableVA(level, context, x, y, c, fmt, args);
			va_end(args);
		}

		inline void WriteTable_impl (level_t level, context_t context, int x, int y, Color fg, Color bg, char const * fmt, va_list args);
		void WriteTableVA (level_t level, context_t context, int x, int y, Color fg, Color bg, char const * fmt, va_list args)
		{
			if (RuntimeFilterPredicate(level, context))
				WriteTable_impl(level, context, x, y, fg, bg, fmt, args);
		}
		void WriteTable (level_t level, context_t context, int x, int y, Color fg, Color bg, char const * fmt, ...)
		{
			va_list args;
			va_start(args, fmt);
			WriteTableVA(level, context, x, y, fg, bg, fmt, args);
			va_end(args);
		}

		inline void WriteTableSetColor_impl (level_t level, context_t context, int x, int y, Color fg, char const * fmt, va_list args);
		void WriteTableSetColorVA (level_t level, context_t context, int x, int y, Color fg, char const * fmt, va_list args)
		{
			if (RuntimeFilterPredicate(level, context))
				WriteTableSetColor_impl(level, context, x, y, fg, fmt, args);
		}
		void WriteTableSetColor (level_t level, context_t context, int x, int y, Color fg, char const * fmt, ...)
		{
			va_list args;
			va_start(args, fmt);
			WriteTableSetColorVA(level, context, x, y, fg, fmt, args);
			va_end(args);
		}

		inline void WriteTableSetColor_impl (level_t level, context_t context, int x, int y, Color fg, Color bg, char const * fmt, va_list args);
		void WriteTableSetColorVA (level_t level, context_t context, int x, int y, Color fg, Color bg, char const * fmt, va_list args)
		{
			if (RuntimeFilterPredicate(level, context))
				WriteTableSetColor_impl(level, context, x, y, fg, bg, fmt, args);
		}
		void WriteTableSetColor (level_t level, context_t context, int x, int y, Color fg, Color bg, char const * fmt, ...)
		{
			va_list args;
			va_start(args, fmt);
			WriteTableSetColorVA(level, context, x, y, fg, bg, fmt, args);
			va_end(args);
		}

		inline void WriteTableSetHHeader_impl (level_t level, context_t context, int x, char const * name, char const * fmt, va_list args);
		void WriteTableSetHHeaderVA (level_t level, context_t context, int x, char const * name, char const * fmt, va_list args)
		{
			if (RuntimeFilterPredicate(level, context))
				WriteTableSetHHeader_impl(level, context, x, name, fmt, args);
		}
		void WriteTableSetHHeader (level_t level, context_t context, int x,  char const * name, char const * fmt, ...)
		{
			va_list args;
			va_start(args, fmt);
			WriteTableSetHHeaderVA(level, context, x, name, fmt, args);
			va_end(args);
		}
		inline void WriteTableClear_impl (level_t level, context_t context, char const * fmt, va_list args);
		void WriteTableClearVA (level_t level, context_t context, char const * fmt, va_list args)
		{
			if (RuntimeFilterPredicate(level, context))
				WriteTableClear_impl(level, context, fmt, args);
		}
		void WriteTableClear (level_t level, context_t context, char const * fmt, ...)
		{
			va_list args;
			va_start(args, fmt);
			WriteTableClearVA(level, context, fmt, args);
			va_end(args);
		}




		// gantt chart event logging
		void WriteGanttBgnVA_Impl (level_t level, context_t context, char const * fmt, va_list args);
		void WriteGanttBgnVA (level_t level, context_t context, char const * fmt, va_list args)
		{
			if (RuntimeFilterPredicate(level, context))
				WriteGanttBgnVA_Impl(level, context, fmt, args);
		}
		void WriteGanttBgn (level_t level, context_t context, char const * fmt, ...)
		{
			va_list args;
			va_start(args, fmt);
			WriteGanttBgnVA(level, context, fmt, args);
			va_end(args);
		}

		void WriteGanttBgn_Impl (level_t level, context_t context);
		void WriteGanttBgn (level_t level, context_t context)
		{
			if (RuntimeFilterPredicate(level, context))
				WriteGanttBgn_Impl(level, context);
		}

		void WriteGanttEndVA_Impl (level_t level, context_t context, char const * fmt, va_list args);
		void WriteGanttEndVA (level_t level, context_t context, char const * fmt, va_list args)
		{
			if (RuntimeFilterPredicate(level, context))
				WriteGanttEndVA_Impl(level, context, fmt, args);
		}
		void WriteGanttEnd (level_t level, context_t context, char const * fmt, ...)
		{
			va_list args;
			va_start(args, fmt);
			WriteGanttEndVA(level, context, fmt, args);
			va_end(args);
		}

		void WriteGanttEnd_Impl (level_t level, context_t context);
		void WriteGanttEnd (level_t level, context_t context)
		{
			if (RuntimeFilterPredicate(level, context))
				WriteGanttEnd_Impl(level, context);
		}

		void WriteGanttFrameBgnVA_Impl (level_t level, context_t context, char const * fmt, va_list args);
		void WriteGanttFrameBgnVA (level_t level, context_t context, char const * fmt, va_list args)
		{
			if (RuntimeFilterPredicate(level, context))
				WriteGanttFrameBgnVA_Impl(level, context, fmt, args);
		}
		void WriteGanttFrameBgn (level_t level, context_t context, char const * fmt, ...)
		{
			va_list args;
			va_start(args, fmt);
			WriteGanttFrameBgnVA(level, context, fmt, args);
			va_end(args);
		}
		void WriteGanttFrameBgn_Impl(level_t level, context_t context);
		void WriteGanttFrameBgn (level_t level, context_t context)
		{
			if (RuntimeFilterPredicate(level, context))
				WriteGanttFrameBgn_Impl(level, context);
		}

		void WriteGanttFrameEndVA_Impl (level_t level, context_t context, char const * fmt, va_list args);
		void WriteGanttFrameEndVA (level_t level, context_t context, char const * fmt, va_list args)
		{
			if (RuntimeFilterPredicate(level, context))
				WriteGanttFrameEndVA_Impl(level, context, fmt, args);
		}
		void WriteGanttFrameEnd (level_t level, context_t context, char const * fmt, ...)
		{
			va_list args;
			va_start(args, fmt);
			WriteGanttFrameEndVA(level, context, fmt, args);
			va_end(args);
		}
		void WriteGanttFrameEnd_Impl (level_t level, context_t context);
		void WriteGanttFrameEnd (level_t level, context_t context)
		{
			if (RuntimeFilterPredicate(level, context))
				WriteGanttFrameEnd_Impl(level, context);
		}

		void WriteGanttClearVA_Impl (level_t level, context_t context, char const * fmt, va_list args);
		void WriteGanttClearVA (level_t level, context_t context, char const * fmt, va_list args)
		{
			if (RuntimeFilterPredicate(level, context))
				WriteGanttClearVA_Impl(level, context, fmt, args);
		}
		void WriteGanttClear (level_t level, context_t context, char const * fmt, ...)
		{
			va_list args;
			va_start(args, fmt);
			WriteGanttClearVA(level, context, fmt, args);
			va_end(args);
		}

		void WriteGanttScopeBgnVA_Impl (level_t level, context_t context, char * tag_buff, size_t max_size, char const * fmt, va_list args);
		void WriteGanttScopeBgnVA (level_t level, context_t context, char * tag_buff, size_t max_size, char const * fmt, va_list args)
		{
			if (RuntimeFilterPredicate(level, context))
				WriteGanttScopeBgnVA_Impl(level, context, tag_buff, max_size, fmt, args);
		}
		ScopedGantt::ScopedGantt (level_t level, context_t context, char const * fmt, ...)
		{
			va_list args;
			va_start(args, fmt);
			WriteGanttScopeBgnVA(level, context, m_tag, 256, fmt, args);
			va_end(args);
		}
		
		ScopedGantt::~ScopedGantt ()
		{
			WriteGanttEnd(m_level, m_context, "%s", m_tag);
		}

		ScopedGanttFrame::ScopedGanttFrame (level_t level, context_t context, char const * fmt, ...)
		{
			va_list args;
			va_start(args, fmt);
			WriteGanttFrameBgnVA(level, context, fmt, args);
			va_end(args);
		}

		ScopedGanttFrame::~ScopedGanttFrame()
		{
			WriteGanttFrameEnd(m_level, m_context);
		}

	}
#	include "platforms/select_platform.inl"

#else // tracing is NOT enabled
	; // the perfect line
#endif
