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
 * Thanks to WarHorse Studios who funded initial development.
 **/
#pragma once

#if defined TRACE_ENABLED

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

#	include <stdarg.h>	// for va_args

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

/**	@macro		TRACE
 *	@brief		logging of the form TRACE(lvl, ctx, fmt, ...)
 **/
#	define TRACE(level, context, fmt, ... )	\
		trace::Write(static_cast<trace::level_t>(level), context, __FILE__, __LINE__, __FUNCTION__, fmt, __VA_ARGS__);
/**	@macro		TRACE_VA
 *	@brief		logging of the form TRACE_VA(lvl, ctx, fmt, va_list)
 **/
#	define TRACE_VA(level, context, fmt, vaargs)	\
		trace::Write(static_cast<trace::level_t>(level), context, __FILE__, __LINE__, __FUNCTION__, fmt, vaargs);

/**	@macro		TRACE_ENTRY
 *	@brief		logs entry to scope and logs exit on scope end
 **/
#	define TRACE_ENTRY(level, context)	\
		trace::ScopedLog entry_guard___(static_cast<trace::level_t>(level), context, __FILE__, __LINE__, __FUNCTION__);

	namespace trace {

		void SetAppName (char const *);
		char const * GetAppName ();

		void Connect ();
		void Disconnect ();

		/**@fn		SetRuntimeLevel
		 * @brief	adjusts run-time level of log message filtering
		 **/
		void SetRuntimeLevel (level_t level);
		level_t GetRuntimeLevel ();

		/**@fn		SetRuntimeContextMask
		 * @brief	adjusts run-time context of log message filtering
		 **/
		void SetRuntimeContextMask (context_t mask);
		context_t GetRuntimeContextMask ();

		/**@fn		RuntimeFilterPredicate
		 * @brief	decides if message will be logged or not
		 */
		inline bool RuntimeFilterPredicate (level_t level, context_t context)
		{
			return (level <= GetRuntimeLevel() && ((context & GetRuntimeContextMask()) != 0));
		}
		
		/**@fn		Write to log
		 * @brief	write to log of the form (fmt, va_list)
		 **/
		void Write (level_t level, context_t context, char const * file, int line, char const * fn, char const * fmt, va_list);

		/**@fn		Write to log
		 * @brief	write to log of the form (fmt, ...)
		 **/
		inline void Write (level_t level, context_t context, char const * file, int line, char const * fn, char const * fmt, ...)
		{
			va_list args;
			va_start(args, fmt);
			Write(level, context, file, line, fn, fmt, args);
			va_end(args);
		}

		/**@class	ScopedLog
		 * @brief	RAII class for logging entry on construction and exit on destruction
		 **/
		struct ScopedLog
		{
			enum E_Type { e_Entry = 0, e_Exit = 1 };
			level_t m_level;
			context_t m_context;
			char const * m_file;
			int m_line;
			char const * m_fn;

			ScopedLog (level_t level, context_t context, char const * file, int line, char const * fn);
			~ScopedLog ();
		};
	}

#else // no tracing at all
#	define TRACE_APPNAME(name)                      ((void)0)
#	define TRACE_SETLEVEL(n)                        ((void)0)
#	define TRACE_CONNECT()                          ((void)0)
#	define TRACE_DISCONNECT()                       ((void)0)
#	define TRACE(level, context, fmt, ... )         ((void)0)
#	define TRACE_VA(level, context, fmt, vaargs)    ((void)0)
#	define TRACE_ENTRY(level, context)              ((void)0)
#endif // TRACE_ENABLED