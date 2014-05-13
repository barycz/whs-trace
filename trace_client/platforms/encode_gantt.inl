#pragma once
#include "include_ntohs.h"
#include <tlv_parser/tlv_parser.h>
#include <tlv_parser/tlv_encoder.h>
#include "encode_common_fields.inl"
#include <algorithm>

namespace trace {

	char const * strnchr (char const * s, size_t len, int c)
	{
		for (size_t i = 0; i < len; ++i)
			if (s[i] == c)
				return s + i;
		return 0;
	}

	template <class Encoder>
	void encode_gantt_msg (Encoder & e, char * tag_buff, size_t max_size, char const * fmt, va_list args)
	{
		size_t const tlv_buff_sz = 1024;
		char tlv_buff[tlv_buff_sz];
		using namespace tlv;
		size_t const len = sys::va_trc_vsnprintf(tlv_buff, tlv_buff_sz, fmt, args);
		e.Encode(TLV(tag_msg, sys::va_trc_vsnprintf(tlv_buff, tlv_buff_sz, fmt, args), tlv_buff));
		char const * ptr = strnchr(tlv_buff, len, '[');
		if (ptr)
		{
			size_t const n = ptr - tlv_buff;
			size_t const to_cpy = n;
			size_t max_to_cpy = to_cpy > max_size ? max_size : to_cpy;
			strncpy_s(tag_buff, max_size, tlv_buff, max_to_cpy);
			//@FIXME append \0 on overflow
		}
		else
		{
			strncpy_s(tag_buff, max_size, tlv_buff, len > max_size ? max_size : len);
		}
	}


	inline void encode_gantt_scope_bgn (msg_t & msg, level_t level, context_t context, char * tag_buff, size_t max_tag_size, char const * fmt, va_list args)
	{
		size_t const tlv_buff_sz = 256;
		char tlv_buff[tlv_buff_sz];
		using namespace tlv;
		tlv::Encoder_v1 e(tlv::cmd_gantt_bgn, msg.m_data, msg_t::e_data_sz);
		e.Encode(TLV(tag_ctime, sys::trc_vsnprintf(tlv_buff, tlv_buff_sz, "%llu", sys::queryTime_us()), tlv_buff));
		e.Encode(TLV(tag_lvl,  sys::trc_vsnprintf(tlv_buff, tlv_buff_sz,   "%u", level), tlv_buff));
		e.Encode(TLV(tag_ctx,  sys::trc_vsnprintf(tlv_buff, tlv_buff_sz,   "%x", context), tlv_buff));
		e.Encode(TLV(tag_tid,  sys::trc_vsnprintf(tlv_buff, tlv_buff_sz,   "%u", sys::get_tid()), tlv_buff));
		encode_gantt_msg(e, tag_buff, max_tag_size, fmt, args);
		if (e.Commit())
		{
			msg.m_length = e.total_len;
		}

	}

	inline void encode_gantt_bgn (msg_t & msg, level_t level, context_t context, char const * fmt, va_list args)
	{
		size_t const tlv_buff_sz = 256;
		char tlv_buff[tlv_buff_sz];
		using namespace tlv;
		tlv::Encoder_v1 e(tlv::cmd_gantt_bgn, msg.m_data, msg_t::e_data_sz);
		e.Encode(TLV(tag_ctime, sys::trc_vsnprintf(tlv_buff, tlv_buff_sz, "%llu", sys::queryTime_us()), tlv_buff));
		e.Encode(TLV(tag_lvl,  sys::trc_vsnprintf(tlv_buff, tlv_buff_sz,   "%u", level), tlv_buff));
		e.Encode(TLV(tag_ctx,  sys::trc_vsnprintf(tlv_buff, tlv_buff_sz,   "%x", context), tlv_buff));
		e.Encode(TLV(tag_tid,  sys::trc_vsnprintf(tlv_buff, tlv_buff_sz,   "%u", sys::get_tid()), tlv_buff));
		encode_va_fields(e, fmt, args);
		if (e.Commit())
		{
			msg.m_length = e.total_len;
		}
	}

	inline void encode_gantt_bgn (msg_t & msg, level_t level, context_t context)
	{
		size_t const tlv_buff_sz = 256;
		char tlv_buff[tlv_buff_sz];
		using namespace tlv;
		tlv::Encoder_v1 e(tlv::cmd_gantt_bgn, msg.m_data, msg_t::e_data_sz);
		e.Encode(TLV(tag_ctime, sys::trc_vsnprintf(tlv_buff, tlv_buff_sz, "%llu", sys::queryTime_us()), tlv_buff));
		e.Encode(TLV(tag_lvl,  sys::trc_vsnprintf(tlv_buff, tlv_buff_sz,   "%u", level), tlv_buff));
		e.Encode(TLV(tag_ctx,  sys::trc_vsnprintf(tlv_buff, tlv_buff_sz,   "%x", context), tlv_buff));
		e.Encode(TLV(tag_tid,  sys::trc_vsnprintf(tlv_buff, tlv_buff_sz,   "%u", sys::get_tid()), tlv_buff));
		if (e.Commit())
		{
			msg.m_length = e.total_len;
		}
	}

	inline void encode_gantt_end (msg_t & msg, level_t level, context_t context, char const * fmt, va_list args)
	{
		size_t const tlv_buff_sz = 256;
		char tlv_buff[tlv_buff_sz];
		using namespace tlv;
		tlv::Encoder_v1 e(tlv::cmd_gantt_end, msg.m_data, msg_t::e_data_sz);
		e.Encode(TLV(tag_ctime, sys::trc_vsnprintf(tlv_buff, tlv_buff_sz, "%llu", sys::queryTime_us()), tlv_buff));
		e.Encode(TLV(tag_lvl,  sys::trc_vsnprintf(tlv_buff, tlv_buff_sz,   "%u", level), tlv_buff));
		e.Encode(TLV(tag_ctx,  sys::trc_vsnprintf(tlv_buff, tlv_buff_sz,   "%x", context), tlv_buff));
		e.Encode(TLV(tag_tid,  sys::trc_vsnprintf(tlv_buff, tlv_buff_sz,   "%u", sys::get_tid()), tlv_buff));
		encode_va_fields(e, fmt, args);
		if (e.Commit())
		{
			msg.m_length = e.total_len;
		}
	}

	inline void encode_gantt_end (msg_t & msg, level_t level, context_t context)
	{
		size_t const tlv_buff_sz = 256;
		char tlv_buff[tlv_buff_sz];
		using namespace tlv;
		tlv::Encoder_v1 e(tlv::cmd_gantt_end, msg.m_data, msg_t::e_data_sz);
		e.Encode(TLV(tag_ctime, sys::trc_vsnprintf(tlv_buff, tlv_buff_sz, "%llu", sys::queryTime_us()), tlv_buff));
		e.Encode(TLV(tag_lvl,  sys::trc_vsnprintf(tlv_buff, tlv_buff_sz,   "%u", level), tlv_buff));
		e.Encode(TLV(tag_ctx,  sys::trc_vsnprintf(tlv_buff, tlv_buff_sz,   "%x", context), tlv_buff));
		e.Encode(TLV(tag_tid,  sys::trc_vsnprintf(tlv_buff, tlv_buff_sz,   "%u", sys::get_tid()), tlv_buff));
		if (e.Commit())
		{
			msg.m_length = e.total_len;
		}
	}

	inline void encode_gantt_frame_bgn (msg_t & msg, level_t level, context_t context, char const * fmt, va_list args)
	{
		size_t const tlv_buff_sz = 256;
		char tlv_buff[tlv_buff_sz];
		using namespace tlv;
		tlv::Encoder_v1 e(tlv::cmd_gantt_frame_bgn, msg.m_data, msg_t::e_data_sz);
		e.Encode(TLV(tag_ctime, sys::trc_vsnprintf(tlv_buff, tlv_buff_sz, "%llu", sys::queryTime_us()), tlv_buff));
		e.Encode(TLV(tag_lvl,  sys::trc_vsnprintf(tlv_buff, tlv_buff_sz,   "%u", level), tlv_buff));
		e.Encode(TLV(tag_ctx,  sys::trc_vsnprintf(tlv_buff, tlv_buff_sz,   "%x", context), tlv_buff));
		e.Encode(TLV(tag_tid,  sys::trc_vsnprintf(tlv_buff, tlv_buff_sz,   "%u", sys::get_tid()), tlv_buff));
		encode_va_fields(e, fmt, args);
		if (e.Commit())
		{
			msg.m_length = e.total_len;
		}
	}

	inline void encode_gantt_frame_bgn (msg_t & msg, level_t level, context_t context)
	{
		size_t const tlv_buff_sz = 256;
		char tlv_buff[tlv_buff_sz];
		using namespace tlv;
		tlv::Encoder_v1 e(tlv::cmd_gantt_frame_bgn, msg.m_data, msg_t::e_data_sz);
		e.Encode(TLV(tag_ctime, sys::trc_vsnprintf(tlv_buff, tlv_buff_sz, "%llu", sys::queryTime_us()), tlv_buff));
		e.Encode(TLV(tag_lvl,  sys::trc_vsnprintf(tlv_buff, tlv_buff_sz,   "%u", level), tlv_buff));
		e.Encode(TLV(tag_ctx,  sys::trc_vsnprintf(tlv_buff, tlv_buff_sz,   "%x", context), tlv_buff));
		e.Encode(TLV(tag_tid,  sys::trc_vsnprintf(tlv_buff, tlv_buff_sz,   "%u", sys::get_tid()), tlv_buff));
		if (e.Commit())
		{
			msg.m_length = e.total_len;
		}
	}

	inline void encode_gantt_frame_end (msg_t & msg, level_t level, context_t context, char const * fmt, va_list args)
	{
		size_t const tlv_buff_sz = 256;
		char tlv_buff[tlv_buff_sz];
		using namespace tlv;
		tlv::Encoder_v1 e(tlv::cmd_gantt_frame_end, msg.m_data, msg_t::e_data_sz);
		e.Encode(TLV(tag_ctime, sys::trc_vsnprintf(tlv_buff, tlv_buff_sz, "%llu", sys::queryTime_us()), tlv_buff));
		e.Encode(TLV(tag_lvl,  sys::trc_vsnprintf(tlv_buff, tlv_buff_sz,   "%u", level), tlv_buff));
		e.Encode(TLV(tag_ctx,  sys::trc_vsnprintf(tlv_buff, tlv_buff_sz,   "%x", context), tlv_buff));
		e.Encode(TLV(tag_tid,  sys::trc_vsnprintf(tlv_buff, tlv_buff_sz,   "%u", sys::get_tid()), tlv_buff));
		encode_va_fields(e, fmt, args);
		if (e.Commit())
		{
			msg.m_length = e.total_len;
		}
	}

	inline void encode_gantt_frame_end (msg_t & msg, level_t level, context_t context)
	{
		size_t const tlv_buff_sz = 256;
		char tlv_buff[tlv_buff_sz];
		using namespace tlv;
		tlv::Encoder_v1 e(tlv::cmd_gantt_frame_end, msg.m_data, msg_t::e_data_sz);
		e.Encode(TLV(tag_ctime, sys::trc_vsnprintf(tlv_buff, tlv_buff_sz, "%llu", sys::queryTime_us()), tlv_buff));
		e.Encode(TLV(tag_lvl,  sys::trc_vsnprintf(tlv_buff, tlv_buff_sz,   "%u", level), tlv_buff));
		e.Encode(TLV(tag_ctx,  sys::trc_vsnprintf(tlv_buff, tlv_buff_sz,   "%x", context), tlv_buff));
		e.Encode(TLV(tag_tid,  sys::trc_vsnprintf(tlv_buff, tlv_buff_sz,   "%u", sys::get_tid()), tlv_buff));
		if (e.Commit())
		{
			msg.m_length = e.total_len;
		}
	}

	inline void encode_gantt_clear (msg_t & msg, level_t level, context_t context, char const * fmt, va_list args)
	{
		size_t const tlv_buff_sz = 256;
		char tlv_buff[tlv_buff_sz];
		using namespace tlv;
		tlv::Encoder_v1 e(tlv::cmd_gantt_clear, msg.m_data, msg_t::e_data_sz);
		e.Encode(TLV(tag_ctime, sys::trc_vsnprintf(tlv_buff, tlv_buff_sz, "%llu", sys::queryTime_us()), tlv_buff));
		e.Encode(TLV(tag_lvl,  sys::trc_vsnprintf(tlv_buff, tlv_buff_sz,   "%u", level), tlv_buff));
		e.Encode(TLV(tag_ctx,  sys::trc_vsnprintf(tlv_buff, tlv_buff_sz,   "%x", context), tlv_buff));
		e.Encode(TLV(tag_tid,  sys::trc_vsnprintf(tlv_buff, tlv_buff_sz,   "%u", sys::get_tid()), tlv_buff));
		encode_va_fields(e, fmt, args);
		if (e.Commit())
		{
			msg.m_length = e.total_len;
		}
	}

}

