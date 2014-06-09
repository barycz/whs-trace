#pragma once

namespace trace {

#define ENCODE_BODY(xxx)                                        \
		if (GetRuntimeBuffering())                              \
		{                                                       \
			msg_t & msg = socks::acquire_msg_buffer();          \
			msg.WriteLock();                                    \
			{                                                   \
				xxx;                                            \
			}                                                   \
			msg.WriteUnlockAndDirty();                          \
		}                                                       \
		else                                                    \
		{                                                       \
			msg_t msg;                                          \
			xxx;                                                \
			socks::WriteToSocket(msg.m_data, msg.m_length);     \
		}


	// setup and utils
	inline void SetCustomUserDictionnary (CtxDictPair const * ptr, size_t n)
	{
		ENCODE_BODY(encode_dict(msg, tlv::cmd_dict_ctx, ptr, n));
	}
	inline void SetCustomUserDictionnary (LvlDictPair const * ptr, size_t n)
	{
		ENCODE_BODY(encode_dict(msg, tlv::cmd_dict_lvl, ptr, n));
	}
	inline void ExportToCSV (char const * file)
	{
		ENCODE_BODY(encode_exportCSV(msg, file));
	}


	// message logging
	inline void WriteLog (level_t level, context_t context, char const * file, int line, char const * fn, char const * fmt, va_list args)
	{
		ENCODE_BODY(encode_log(msg, level, context, file, line, fn, fmt, args));
	}
	inline void WriteStr (level_t level, context_t context, char const * file, int line, char const * fn, char const * str)
	{
		ENCODE_BODY(encode_str(msg, level, context, file, line, fn, str));
	}
	inline void WriteScopeVA (ScopedLog::E_Type type, level_t level, context_t context, char const * file, int line, char const * fn, char const * fmt, va_list args)
	{
		ENCODE_BODY(encode_scope(msg, (type == ScopedLog::e_Entry ? tlv::cmd_scope_entry : tlv::cmd_scope_exit) , level, context, file, line, fn, fmt, args));
	}


	// Plotting
	inline void WritePlot_impl (level_t level, context_t context, float x, float y, char const * fmt, va_list args)
	{
		ENCODE_BODY(encode_plot(msg, level, context, x, y, fmt, args));
	}
	inline void WritePlotXYZ (level_t level, context_t context, float x, float y, float z, char const * fmt, va_list args)
	{
		ENCODE_BODY(encode_plot(msg, level, context, x, y, z, fmt, args));
	}
	inline void WritePlotClear_impl (level_t level, context_t context, char const * fmt, va_list args)
	{
		ENCODE_BODY(encode_plot_clear(msg, level, context, fmt, args));
	}


	// Table data logging
	inline void WriteTable_impl (level_t level, context_t context, int x, int y, char const * fmt, va_list args)
	{
		ENCODE_BODY(encode_table(msg, level, context, x, y, fmt, args));
	}
	inline void WriteTable_impl (level_t level, context_t context, int x, int y, Color c, char const * fmt, va_list args)
	{
		ENCODE_BODY(encode_table(msg, level, context, x, y, c, fmt, args));
	}
	inline void WriteTable_impl (level_t level, context_t context, int x, int y, Color fg, Color bg, char const * fmt, va_list args)
	{
		ENCODE_BODY(encode_table(msg, level, context, x, y, fg, bg, fmt, args));
	}
	inline void WriteTableSetColor_impl (level_t level, context_t context, int x, int y, Color fg, char const * fmt, va_list args)
	{
		ENCODE_BODY(encode_table_setup_color(msg, level, context, x, y, fg, fmt, args));
	}
	inline void WriteTableSetColor_impl (level_t level, context_t context, int x, int y, Color fg, Color bg, char const * fmt, va_list args)
	{
		ENCODE_BODY(encode_table_setup_color(msg, level, context, x, y, fg, bg, fmt, args));
	}
	inline void WriteTableSetHHeader_impl (level_t level, context_t context, int x, char const * name, char const * fmt, va_list args)
	{
		ENCODE_BODY(encode_table_setup_hhdr(msg, level, context, x, name, fmt, args));
	}
	inline void WriteTableClear_impl (level_t level, context_t context, char const * fmt, va_list args)
	{
		ENCODE_BODY(encode_table_clear(msg, level, context, fmt, args));
	}


	// gantt write functions
	inline void WriteGanttBgnVA_Impl (level_t level, context_t context, char const * fmt, va_list args)
	{
		ENCODE_BODY(encode_gantt_bgn(msg, level, context, fmt, args));
	}
	inline void WriteGanttScopeBgnVA_Impl (level_t level, context_t context, char * tag_buff, size_t tag_max_sz, char const * fmt, va_list args)
	{
		ENCODE_BODY(encode_gantt_scope_bgn(msg, level, context, tag_buff, tag_max_sz, fmt, args));
	}
	void WriteGanttBgn_Impl (level_t level, context_t context)
	{
		ENCODE_BODY(encode_gantt_bgn(msg, level, context));
	}
	inline void WriteGanttEndVA_Impl (level_t level, context_t context, char const * fmt, va_list args)
	{
		ENCODE_BODY(encode_gantt_end(msg, level, context, fmt, args));
	}
	inline void WriteGanttEnd_Impl (level_t level, context_t context)
	{
		ENCODE_BODY(encode_gantt_end(msg, level, context));
	}
	inline void WriteGanttFrameBgnVA_Impl (level_t level, context_t context, char const * fmt, va_list args)
	{
		ENCODE_BODY(encode_gantt_frame_bgn(msg, level, context, fmt, args));
	}
	inline void WriteGanttFrameBgn_Impl(level_t level, context_t context)
	{
		ENCODE_BODY(encode_gantt_frame_bgn(msg, level, context));
	}
	inline void WriteGanttFrameEndVA_Impl (level_t level, context_t context, char const * fmt, va_list args)
	{
		ENCODE_BODY(encode_gantt_frame_end(msg, level, context, fmt, args));
	}
	inline void WriteGanttFrameEnd_Impl (level_t level, context_t context)
	{
		ENCODE_BODY(encode_gantt_frame_end(msg, level, context));
	}
	inline void WriteGanttClearVA_Impl (level_t level, context_t context, char const * fmt, va_list args)
	{
		ENCODE_BODY(encode_gantt_clear(msg, level, context, fmt, args));
	}
}

#undef ENCODE_BODY

