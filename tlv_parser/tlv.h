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
 **/
#pragma once
#include <vector>

namespace tlv {

	typedef unsigned char cmd_t;					/// command type (1 byte for now)
	typedef unsigned char tag_t;					/// tag type (1 byte)
	typedef unsigned short len_t;					/// length type (2 bytes)

	/** available commands */
	static cmd_t const cmd_setup             = 0xFF;	/// trace setup message
	static cmd_t const cmd_log               = 0xFE;	/// trace log message
	static cmd_t const cmd_scope_entry       = 0xFD;	/// trace scope
	static cmd_t const cmd_scope_exit        = 0xFC;	/// trace scope
	static cmd_t const cmd_restart_req       = 0xFB;	/// tcp stream restart request
	static cmd_t const cmd_restart_ack       = 0xFA;	/// tcp stream restart ack
	static cmd_t const cmd_set_level         = 0xF9;	/// set debug level
	static cmd_t const cmd_set_level_ack     = 0xF8;	/// set debug level ack
	static cmd_t const cmd_set_ctx           = 0xF7;	/// adjust runtime context filtering
	static cmd_t const cmd_set_ctx_ack       = 0xF6;	/// request context from other party
	static cmd_t const cmd_set_buffering     = 0xF5;	/// set buffering on/off
	static cmd_t const cmd_export_csv        = 0xF4;	/// export as csv
	static cmd_t const cmd_save_tlv          = 0xF3;	/// save as tlv (native format)
	static cmd_t const cmd_setup_ack         = 0xF2;	/// acknowledge setup command
	static cmd_t const cmd_plot_xy           = 0xF1;	/// draw data xy
	static cmd_t const cmd_plot_xyz          = 0xF0;	/// draw data xyz
	static cmd_t const cmd_profile_bgn       = 0xEF;	/// send profiling begin
	static cmd_t const cmd_profile_end       = 0xEE;	/// send profiling end
	static cmd_t const cmd_profile_frame_bgn = 0xED;	/// send profiling frame begin
	static cmd_t const cmd_profile_frame_end = 0xEC;	/// send profiling frame end
	static cmd_t const cmd_table_xy          = 0xEB;	/// trace table xy
	static cmd_t const cmd_table_setup       = 0xEA;	/// trace table setup (color, headers)
	static cmd_t const cmd_gantt_bgn         = 0xE9;	/// log event begin
	static cmd_t const cmd_gantt_end         = 0xE8;	/// log event end
	static cmd_t const cmd_gantt_frame_bgn   = 0xE7;	/// log frame begin
	static cmd_t const cmd_gantt_frame_end   = 0xE6;	/// log frame end
	static cmd_t const cmd_gantt_clear       = 0xE5;	/// clear gantt chart
	static cmd_t const cmd_plot_clear        = 0xE4;	/// clear plot
	static cmd_t const cmd_table_clear       = 0xE3;	/// clear table
	static cmd_t const cmd_log_clear         = 0xE2;	/// clear log

	///////////////////////////////////////////////////////
	static cmd_t const cmd_shutdown          = 0xCF;	/// request to shut down server
	static cmd_t const cmd_ping              = 0xCE;	/// ping
	static cmd_t const cmd_pong              = 0xCD;	/// ping response
	static cmd_t const cmd_dict_ctx          = 0xCC;	/// context dictionnary (string, int)
	static cmd_t const cmd_version           = 0xCB;	/// get version
	static cmd_t const cmd_version_ack       = 0xCA;	/// version ack
	static cmd_t const cmd_set_proto_version = 0xC9;	/// switch protocol version
	static cmd_t const cmd_set_proto_version_ack = 0xC8;	/// switch protocol version ack
	static cmd_t const cmd_dict_lvl          = 0xC7;	/// level dictionnary (string, int)

	static cmd_t const cmd_set_logs_state    = 0xC6;	/// change state of logs (turn off sending)
	static cmd_t const cmd_set_plots_state   = 0xC5;	/// change state of plots (turn off sending)
	static cmd_t const cmd_set_tables_state  = 0xC4;	/// change state of tables (turn off sending)
	static cmd_t const cmd_set_gantts_state  = 0xC3;	/// change state of gantts (turn off sending)

	static cmd_t const cmd_set_ctx_level     = 0xC2;	/// change logging level per context (default: all contexts use cmd_set_level)

	enum e_Tags
	{
		tag_invalid = 0,  /// invalid tag value
		tag_app,          /// application name
		tag_pid,          /// process id
		tag_ctime,        /// time (client)
		tag_stime,        /// time (server)
		tag_tid,          /// thread id
		tag_file,         /// file name
		tag_line,         /// line
		tag_func,         /// function
		tag_msg,          /// logged message
		tag_lvl,          /// logging level
		tag_ctx,          /// logging context
		tag_dt,           /// vertical header

		/// now following tags are considered as "system"
		tag_bool,         /// bool
		tag_int,          /// int
		tag_string,       /// string
		tag_float,        /// float
		tag_ix,           /// x (int)
		tag_iy,           /// y (int)
		tag_x,            /// x (float)
		tag_y,            /// y (float)
		tag_z,            /// z (float)
		tag_fgc,          /// fg color
		tag_bgc,          /// bg color
		tag_hhdr,         /// horizontal header
		tag_vhdr,         /// vertical header

		tag_max_value     /** this should be last line of enum **/
	};

	/** printable form of tags */
	static char const * tag_names[tag_max_value] =
	{
		"---",
		"App",
		"PID",
		"CTime",
		"STime",
		"TID",
		"File",
		"Line",
		"Func",
		"Msg",
		"Lvl",
		"Ctx",
		"dt",

		/*sys tags*/
		"Bool",
		"Int",
		"Str",
		"Flt",
		"ix",
		"iy",
		"x",
		"y",
		"z",
		"FgCol",
		"BgCol",
		"HHdr",
		"VHdr"
	};

	inline size_t get_tag_count () { return tag_max_value; }

	inline char const * get_tag_name (size_t i)
	{
		if (i < tag_max_value)
			return tag_names[i];
		return 0;
	}

	inline size_t tag_for_name (char const * tag_name)
	{
		for (size_t i = 0; i < tag_max_value; ++i)
			if (strcmp(tag_names[i], tag_name) == 0)
				return i;
		return tag_invalid;
	}

	/********************************************
	 * protocol structure is very simple:
	 * Header | Payload
	 * where Payload is a sequence of TLV's
	 * i.e.:
	 * Header | TLVTLVTLV.... TLV
	 ********************************************/
	struct Header
	{
		unsigned char version;
		cmd_t cmd;  // 1 Byte command ( setup | log | ... )
		len_t len;  // 2 Byte length of payload

		enum { e_Size = sizeof(unsigned char) + sizeof(cmd_t) + sizeof(len_t) } ;

		Header (unsigned char v, cmd_t c, len_t l) : version(v), cmd(c), len(l) { }
		void Reset () { version = 1; cmd = 0; len = 0; }
	};
}

