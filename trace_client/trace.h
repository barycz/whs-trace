/**
 * Copyright (C) 2011 Mojmir Svoboda
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is furnished to do
 * so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Thanks to WarHorse Studios http://warhorsestudios.cz/ who funded initial ver
 **/
#pragma once

#if defined TRACE_ENABLED

#	if defined (__GNUC__) && defined(__unix__)
#		define TRACE_API __attribute__ ((__visibility__("default")))
#	elif defined (WIN32)
#		if defined TRACE_STATIC
#			define TRACE_API
#		elif defined TRACE_DLL
#			define TRACE_API __declspec(dllexport)
#		else
#			define TRACE_API __declspec(dllimport)
#		endif
#	elif defined (_XBOX)
#		if defined TRACE_STATIC
#			define TRACE_API:
#		elif defined TRACE_DLL
#			define TRACE_API __declspec(dllexport)
#		else
#			define TRACE_API __declspec(dllimport)
#		endif
#	endif

#	if  defined _MSC_VER
#		include <vadefs.h>
#	else
#		include <stdarg.h>	// for va_args
#	endif

#include "common.h"
#include "../sysfn/enum_factory.h"

/**	@macro		TRACE_CONFIG_INCLUDE
 *	@brief		overrides default config with user-specified one
 **/
#	if !defined TRACE_CONFIG_INCLUDE
#		include	"default_config.h"
#	else
#		include TRACE_CONFIG_INCLUDE
#	endif

/**	@macro		TRACE_LEVELS_INCLUDE
 *	@brief		overrides default levels with user-specified one
 **/
#	if !defined TRACE_LEVELS_INCLUDE
#		include	"default_levels.h"
#	else
#		include TRACE_LEVELS_INCLUDE
#	endif

/**	@macro		TRACE_CONTEXTS_INCLUDE
 *	@brief		overrides default contexts with user-specified one
 **/
#	if !defined TRACE_CONTEXTS_INCLUDE
#		include	"default_contexts.h"
#	else
#	include TRACE_CONTEXTS_INCLUDE
#	endif

/*****************************************************************************/
/* Text logging macros                                                       */
/*****************************************************************************/

/**	@macro		TRACE_MSG
 *	@brief		logging of the form TRACE_MSG(lvl, ctx, fmt, ...)
 **/
#	define TRACE_MSG(level, context, fmt, ... )	\
		trace::Write(static_cast<trace::level_t>(level), context, __FILE__, __LINE__, __FUNCTION__, fmt, __VA_ARGS__)
/**	@macro		TRACE_MSG_VA
 *	@brief		logging of the form TRACE_MSG_VA(lvl, ctx, fmt, va_list)
 **/
#	define TRACE_MSG_VA(level, context, fmt, vaargs)	\
		trace::WriteVA(static_cast<trace::level_t>(level), context, __FILE__, __LINE__, __FUNCTION__, fmt, vaargs)
/**	@macro		TRACE_MSG_CUSTOM
 *	@brief		TRACE_MSG variant that accepts custom file, line and function arguments
 */
#	define TRACE_MSG_CUSTOM(level, context, file, line, function, fmt, ...)	\
		trace::Write(static_cast<trace::level_t>(level), context, file, line, function, fmt, __VA_ARGS__)

/**	@macro		TRACE_SCOPE_MSG
 *	@brief		logs "entry to" and "exit from" scope
 *	@param[in]	fmt			formatted message appended to the scope
 **/
#	define TRACE_SCOPE_MSG(level, context, fmt, ...)	\
		trace::ScopedLog TRACE_UNIQUE(entry_guard_)(static_cast<trace::level_t>(level), context, __FILE__, __LINE__, __FUNCTION__, fmt, __VA_ARGS__)
/**	@macro		TRACE_SCOPE
 *	@brief		logs "entry to" and "exit from" scope
 **/
#	define TRACE_SCOPE(level, context)	TRACE_SCOPE_MSG(level, context, "%s", __FUNCTION__)

/**	@macro		TRACE_CODE
 *	@brief		code that is executed only when trace is enabled
 **/
#	define TRACE_CODE(code) code


/*****************************************************************************/
/* Plot logging macros                                                       */
/*****************************************************************************/

/**	@macro		TRACE_PLOT_XY
 *	@brief		logging of 2d xy data in the form of 2d plot
 **/
#	define TRACE_PLOT_XY	trace::WritePlot
/**	@macro		TRACE_PLOT_XYZ
 *	@brief		logging of 3d xyz data
 **/
#	define TRACE_PLOT_XYZ	trace::WritePlot
/**	@macro		TRACE_PLOT_CLEAR
 *	@brief		clear curve data identified by "tag/curve" or clear all plot using "tag" only
 **/
#	define TRACE_PLOT_CLEAR	trace::WritePlotClear

/*****************************************************************************/
/* Table logging macros                                                      */
/*****************************************************************************/

/**	@macro		TRACE_TABLE
 *	@brief		logging of tabular data
 *	@see		trace::WriteTable
 **/
#	define TRACE_TABLE          trace::WriteTable
#	define TRACE_TABLE_HHEADER	trace::WriteTableSetHHeader
#	define TRACE_TABLE_COLOR	trace::WriteTableSetColor
#	define TRACE_TABLE_CLEAR	trace::WriteTableClear


/*****************************************************************************/
/* Gantt logging macros                                                      */
/*****************************************************************************/

/**	@macro		TRACE_GANTT_.*
 *	@brief		logging of the form TRACE_GANTT_MSG(lvl, ctx, fmt, ...)
 **/
#	define TRACE_GANTT_BGN                  trace::WriteGanttBgn
#	define TRACE_GANTT_END                  trace::WriteGanttEnd

#	define TRACE_GANTT_FRAME_BGN            trace::WriteGanttFrameBgn
#	define TRACE_GANTT_FRAME_END            trace::WriteGanttFrameEnd

/**	@macro		TRACE_GANTT_MSG_VA
 *	@brief		traces event into gantt chart  of the form TRACE_GANTT_MSG_VA(lvl, ctx, fmt, va_list)
 **/
#	define TRACE_GANTT_MSG_VA(fmt, vaargs)	trace::WriteGanttVA(fmt, vaargs);

/**	@macro		TRACE_GANTT_SCOPE
 *	@brief		traces event into gantt chart
 **/
#	define TRACE_GANTT_SCOPE                trace::ScopedGantt TRACE_UNIQUE(profile_entry_guard_)

/**	@macro		TRACE_GANTT_FRAME_SCOPE
 *	@brief		traces frame into gantt chart
 **/
#	define TRACE_GANTT_FRAME_SCOPE          trace::ScopedGanttFrame TRACE_UNIQUE(profile_entry_guard_)

#	define TRACE_GANTT_CLEAR                trace::WriteGanttClear


/*****************************************************************************/
/* Basic setup and utilitary macros                                          */
/*****************************************************************************/

/**	@macro		TRACE_APPNAME
 *	@brief		sets application name that will be sent to server to identify
 **/
#	define TRACE_APPNAME(name) trace::SetAppName(name)

/**	@macro		TRACE_CONNECT
 *	@brief		connects to server and sends application name to server
 **/
#	define TRACE_CONNECT() trace::Connect()

/**	@macro		TRACE_DISCONNECT
 *	@brief		disconnects from server
 **/
#	define TRACE_DISCONNECT() trace::Disconnect()

/**	@macro		TRACE_SETLEVEL
 *	@brief		switch level to another value
 **/
#	define TRACE_SETLEVEL(n) trace::SetRuntimeLevel(n)

/**	@macro		TRACE_SETBUFFERED
 *	@brief		switch between buffered/unbuffered
 **/
#	define TRACE_SETBUFFERED(n) trace::SetRuntimeBuffering(n)

/** @macro		TRACE_EXPORT_CSV
 *  @brief      causes export of current server content as csv format
 */
#	define TRACE_EXPORT_CSV(file)	trace::ExportToCSV(file)

/** @macro		TRACE_SET_CTX_DICT
 *  @brief      causes export of current server content as csv format
 */
#	define TRACE_SET_CTX_DICT()		trace::SetCustomUserDictionnary()

/** @macro		TRACE_FLUSH
 *  @brief      forces flush of all buffers into socket
 *				comes handy when you expect crash and want the data to be sent
 *				out immediately.
 *				typical usage would be logging of a text from assert for example
 */
#	define TRACE_FLUSH()		trace::Flush()

/** @macro		TRACE_ADD_CALLBACK
 *  @brief      adds callback. To this callback are sent all messages.
 */
#	define TRACE_ADD_CALLBACK(callback)	trace::AddCallback(callback)

/** @macro		TRACE_ADD_CALLBACK
 *  @brief      removes callback. To this callback are sent all messages.
 */
#	define TRACE_REMOVE_CALLBACK(callback)	trace::RemoveCallback(callback)


	namespace trace {

		/// type of the traced data
		/// TODO: merge with E_DataWidgetType declared in the server types.h
#		define TRACE_DATA_TYPE_ENUM(FN) \
			FN(DT_Logs,) \
			FN(DT_Plots,) \
			FN(DT_Tables,) \
			FN(DT_Gantts,) \
			FN(DT_Last,)

		FACT_DECLARE_ENUM(E_DataType, TRACE_DATA_TYPE_ENUM);
		FACT_DECLARE_ENUM_TO_STRING(E_DataType, TRACE_DATA_TYPE_ENUM);

		/// how to deal with the traced data
		/// TODO: merge with E_FeatureStates declared in the server types.h
#		define TRACE_DATA_STATE_ENUM(FN) \
			FN(DS_DontSend, = 0) \
			FN(DS_Send, = 1) \

		FACT_DECLARE_ENUM(E_DataState, TRACE_DATA_STATE_ENUM);
		FACT_DECLARE_ENUM_TO_STRING(E_DataState, TRACE_DATA_STATE_ENUM);

		TRACE_API void SetAppName (char const *);
		TRACE_API char const * GetAppName ();

		TRACE_API void Connect ();
		TRACE_API void Disconnect ();

		/**@fn		SetRuntimeLevel
		 * @brief	adjusts run-time level of log message filtering
		 **/
		TRACE_API void SetRuntimeLevel (level_t level);
		TRACE_API level_t GetRuntimeLevel ();

		/// per-context log level adjustment (SetRuntimeLevel overrides all)
		TRACE_API void SetRuntimeContextLevel(context_t ctxId, level_t level);
		/// returns context mask for a given level (masked with the context mask)
		/// this is what is used to discards contexts
		TRACE_API context_t GetMakedRuntimeContextLevelMask(level_t level);

		/**@fn		SetRuntimeBuffering
		 * @brief	adjusts run-time buffering of log message filtering
		 **/
		TRACE_API void SetRuntimeBuffering (bool level);
		TRACE_API bool GetRuntimeBuffering ();

		/**@fn		SetDataTypeState
		 * @brief	you can disable sending of certain data types using this function
		 **/
		TRACE_API void SetDataTypeState(E_DataType dataType, E_DataState state);
		TRACE_API E_DataState GetDataTypeState(E_DataType dataType);

		/**@fn		ExportToCSV
		 * @brief	client sends command to force server dump content to csv
		 **/
		TRACE_API void ExportToCSV (char const *);

		/**@fn		SetRuntimeContextMask
		 * @brief	adjusts run-time context of log message filtering
		 **/
		TRACE_API void SetRuntimeContextMask (context_t mask);
		TRACE_API context_t GetRuntimeContextMask ();

		TRACE_API void SetCustomUserDictionnary ();

		TRACE_API void Flush ();

		/**@fn		RuntimeFilterPredicate
		 * @brief	decides if message will be logged or not
		 */
		inline bool RuntimeFilterPredicate (E_DataType dataType, level_t level, context_t context)
		{
			return GetDataTypeState(dataType) != e_DS_DontSend &&
				((context & GetMakedRuntimeContextLevelMask(level)) != 0);
		}
		
		class I_TraceCallback
		{
		public:
			virtual ~I_TraceCallback() {};
			virtual void Write (level_t level, context_t context, char const * file, int line, char const * fn, char const * fmt, va_list args) =0;
		};

		TRACE_API void AddCallback (I_TraceCallback*);
		TRACE_API void RemoveCallback (I_TraceCallback*);

		/**@fn		Write to log
		 * @brief	write to log of the form (fmt, va_list)
		 **/
		TRACE_API void WriteVA (level_t level, context_t context, char const * file, int line, char const * fn, char const * fmt, va_list);

		/**@fn		Write to log
		 * @brief	write to log of the form (fmt, ...)
		 **/
#if defined __GCC__ || defined __MINGW32__ || defined __linux__
		TRACE_API void Write (level_t level, context_t context, char const * file, int line, char const * fn, char const * fmt, ...) __attribute__ ((format(printf, 6, 7) ));
#elif defined _MSC_VER
		TRACE_API void Write (level_t level, context_t context, char const * file, int line, char const * fn, char const * fmt, ...);
#endif

		/**@fn		WritePlot
		 * @brief	writes data to be plotted in server part
		 *
		 * @param[in]	x	float x-coordinate
		 * @param[in]	y	float y-coordinate
		 * @param[in]	fmt		message to server
		 * @Note:
		 *		the format determines how and where your data is plotted in the server
		 *	counterpart. the message consists of two mandatory parts: plot_name and curve_name
		 *	in the form
		 *			plot_name/curve_name
		 *
		 * @Example:
		 *		WritePlot(lvl, ctx, 1.0f, 2.0f, "my_plot/curve1");
		 *		WritePlot(lvl, ctx, 1.0f, 6.0f, "my_plot/curve2");
		 *		WritePlot(lvl, ctx, 1.0f,-1.0f, "my_plot2/c");
		 *		will add value 2 in curve1 and value in curve2, but they will be in the same
		 *		plot "my_plot"
		 *		third value of -1 will take place into another plot widget.
		 */
		TRACE_API void WritePlot (level_t level, context_t context, float x, float y, char const * fmt, ...);
		TRACE_API void WritePlot (level_t level, context_t context, float x, float y, float z, char const * fmt, ...);
		TRACE_API void WritePlotClear (level_t level, context_t context, char const * fmt, ...);

		/**@class	ScopedLog
		 * @brief	RAII class for logging entry on construction and exit on destruction **/
		struct TRACE_API ScopedLog
		{
			enum E_Type { e_Entry = 0, e_Exit = 1 };
			level_t m_level;
			context_t m_context;
			char const * m_file;
			int m_line;
			char const * m_fn;
			unsigned long long m_start;

			ScopedLog (level_t level, context_t context, char const * file, int line, char const * fn, char const * fmt, ...);
			~ScopedLog ();
		};


		/**@fn		Write table to log
		 * @brief	writes tavle to log of the form (fmt, ...)
		 *
		 * @param[in]	x	x-coordinate of the table. -1, 0, ...X
		 * @param[in]	y	y-coordinate of the table  -1, 0, ...Y
		 * Note:
		 *		-1 is special value of the x,y coordinates.
		 *		-1 means append
		 *	if x or y >= 0 appropriate cell is found (created if not found) and value
		 *  is set onto that cell
		 *
		 * @param[in]	fmt format for the value to be written 
		 * Note:
		 *	Message can set values more cells at once separating them by the
		 *	colum. For example:
		 *		WriteTable(lvl, ctx, 0, -1, "%i|%i|%i|0", a, b, c);
		 *	sets a,b,c to columns of new row and 0 to 4th column
		 *
		 **/
		TRACE_API void WriteTable (level_t level, context_t context, int x, int y, char const * fmt, ...);
		struct Color {
			unsigned char r,g,b,a;
			Color (unsigned char red, unsigned char green, unsigned char blue, unsigned char alpha = 0xff) : r(red), g(green), b(blue), a(alpha) {  }
		};
		TRACE_API void WriteTable (level_t level, context_t context, int x, int y, Color c, char const * fmt, ...);
		TRACE_API void WriteTable (level_t level, context_t context, int x, int y, Color fg, Color bg, char const * fmt, ...);
		TRACE_API void WriteTableSetColor (level_t level, context_t context, int x, int y, Color fg, char const * fmt, ...);
		TRACE_API void WriteTableSetColor (level_t level, context_t context, int x, int y, Color fg, Color bg, char const * fmt, ...);
		TRACE_API void WriteTableSetHHeader (level_t level, context_t context, int x, char const * name, char const * fmt, ...);
		TRACE_API void WriteTableClear (level_t level, context_t context, char const * fmt, ...);


		/**@fn		WriteBgnGanttVA
		 * @brief	write begin to gantt event in form (fmt, va_list) **/
		TRACE_API void WriteGanttBgnVA (level_t level, context_t context, char const * fmt, va_list);

		/**@fn		WriteGanttBgn
		 * @brief	write begin to gantt evet in form (fmt, ...) **/
#if defined __GCC__ || defined __MINGW32__ || defined __linux__
		TRACE_API void WriteGanttBgn (level_t level, context_t context, char const * fmt, ...) __attribute__ ((format(printf, 3, 4) ));
#elif defined _MSC_VER
		TRACE_API void WriteGanttBgn (level_t level, context_t context, char const * fmt, ...);
#endif
		//TRACE_API void WriteGanttBgn (level_t level, context_t context);
		//TRACE_API void WriteGanttEnd (level_t level, context_t context);
		TRACE_API void WriteGanttEnd (level_t level, context_t context, char const * fmt, ...);

#if defined __GCC__ || defined __MINGW32__ || defined __linux__
		TRACE_API void WriteGanttFrameBgn (level_t level, context_t context, char const * fmt, ...) __attribute__ ((format(printf, 3, 4) ));
#elif defined _MSC_VER
		TRACE_API void WriteGanttFrameBgn (level_t level, context_t context, char const * fmt, ...);
#endif
		TRACE_API void WriteGanttFrameBgn (level_t level, context_t context);
		TRACE_API void WriteGanttFrameEnd (level_t level, context_t context);
		TRACE_API void WriteGanttFrameEnd (level_t level, context_t context, char const * fmt, ...);
		TRACE_API void WriteGanttClear (level_t level, context_t context, char const * fmt, ...);

		/**@class	ScopedGantt
		 * @brief	RAII class for gantt begin on construction and gantt end on destruction **/
		struct ScopedGantt
		{
			level_t m_level;
			context_t m_context;
			char m_tag[256];

			TRACE_API ScopedGantt (level_t level, context_t context, char const * fmt, ...);
			TRACE_API ~ScopedGantt ();
		};
		/**@class	ScopedGanttFrame
		 * @brief	RAII class for gantt begin on construction and gantt end on destruction **/
		struct ScopedGanttFrame
		{
			level_t m_level;
			context_t m_context;
			char m_tag[256];

			TRACE_API ScopedGanttFrame (level_t level, context_t context, char const * fmt, ...);
			TRACE_API ~ScopedGanttFrame ();
		};
	}

#else // no tracing at all
# include "trace_dummy.h"
#	define TRACE_ADD_CALLBACK(callback)	((void)0)
#	define TRACE_REMOVE_CALLBACK(callback)	((void)0)
#endif // !TRACE_ENABLED

