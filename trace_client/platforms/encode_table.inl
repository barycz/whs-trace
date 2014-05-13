#pragma once
#include "include_ntohs.h"
#include <tlv_parser/tlv_parser.h>
#include <tlv_parser/tlv_encoder.h>
#include "encode_common_fields.inl"

namespace trace {

	inline void encode_table (msg_t & msg, level_t level, context_t context, int x, int y, char const * fmt, va_list args)
	{
		size_t const tlv_buff_sz = 256;
		char tlv_buff[tlv_buff_sz];
		using namespace tlv;
		tlv::Encoder_v1 e(tlv::cmd_table_xy, msg.m_data, msg_t::e_data_sz);
		e.Encode(TLV(tag_ctime, sys::trc_vsnprintf(tlv_buff, tlv_buff_sz, "%llu", sys::queryTime_us()), tlv_buff));
		e.Encode(TLV(tag_lvl,  sys::trc_vsnprintf(tlv_buff, tlv_buff_sz,   "%u", level), tlv_buff));
		e.Encode(TLV(tag_ctx,  sys::trc_vsnprintf(tlv_buff, tlv_buff_sz,   "%x", context), tlv_buff));
		e.Encode(TLV(tag_ix,   sys::trc_vsnprintf(tlv_buff, tlv_buff_sz,   "%i", x), tlv_buff));
		e.Encode(TLV(tag_iy,   sys::trc_vsnprintf(tlv_buff, tlv_buff_sz,   "%i", y), tlv_buff));
		encode_va_fields(e, fmt, args);
		if (e.Commit())
		{
			msg.m_length = e.total_len;
		}
	}

	inline void encode_table_clear (msg_t & msg, level_t level, context_t context, char const * fmt, va_list args)
	{
		size_t const tlv_buff_sz = 256;
		char tlv_buff[tlv_buff_sz];
		using namespace tlv;
		tlv::Encoder_v1 e(tlv::cmd_table_clear, msg.m_data, msg_t::e_data_sz);
		e.Encode(TLV(tag_ctime, sys::trc_vsnprintf(tlv_buff, tlv_buff_sz, "%llu", sys::queryTime_us()), tlv_buff));
		e.Encode(TLV(tag_lvl,  sys::trc_vsnprintf(tlv_buff, tlv_buff_sz,   "%u", level), tlv_buff));
		e.Encode(TLV(tag_ctx,  sys::trc_vsnprintf(tlv_buff, tlv_buff_sz,   "%x", context), tlv_buff));
		encode_va_fields(e, fmt, args);
		if (e.Commit())
		{
			msg.m_length = e.total_len;
		}
	}

	inline void encode_table (msg_t & msg, level_t level, context_t context, int x, int y, Color c, char const * fmt, va_list args)
	{
		size_t const tlv_buff_sz = 256;
		char tlv_buff[tlv_buff_sz];
		using namespace tlv;
		tlv::Encoder_v1 e(tlv::cmd_table_xy, msg.m_data, msg_t::e_data_sz);
		e.Encode(TLV(tag_ctime, sys::trc_vsnprintf(tlv_buff, tlv_buff_sz, "%llu", sys::queryTime_us()), tlv_buff));
		e.Encode(TLV(tag_lvl,  sys::trc_vsnprintf(tlv_buff, tlv_buff_sz,   "%u", level), tlv_buff));
		e.Encode(TLV(tag_ctx,  sys::trc_vsnprintf(tlv_buff, tlv_buff_sz,   "%x", context), tlv_buff));
		e.Encode(TLV(tag_ix,   sys::trc_vsnprintf(tlv_buff, tlv_buff_sz,   "%i", x), tlv_buff));
		e.Encode(TLV(tag_iy,   sys::trc_vsnprintf(tlv_buff, tlv_buff_sz,   "%i", y), tlv_buff));
		e.Encode(TLV(tag_fgc,  sys::trc_vsnprintf(tlv_buff, tlv_buff_sz,   "#%02x%02x%02x", c.r, c.g, c.b), tlv_buff));
		encode_va_fields(e, fmt, args);
		if (e.Commit())
		{
			msg.m_length = e.total_len;
		}
	}
	inline void encode_table (msg_t & msg, level_t level, context_t context, int x, int y, Color fg, Color bg, char const * fmt, va_list args)
	{
		size_t const tlv_buff_sz = 256;
		char tlv_buff[tlv_buff_sz];
		using namespace tlv;
		tlv::Encoder_v1 e(tlv::cmd_table_xy, msg.m_data, msg_t::e_data_sz);
		e.Encode(TLV(tag_ctime, sys::trc_vsnprintf(tlv_buff, tlv_buff_sz, "%llu", sys::queryTime_us()), tlv_buff));
		e.Encode(TLV(tag_lvl,  sys::trc_vsnprintf(tlv_buff, tlv_buff_sz,   "%u", level), tlv_buff));
		e.Encode(TLV(tag_ctx,  sys::trc_vsnprintf(tlv_buff, tlv_buff_sz,   "%x", context), tlv_buff));
		e.Encode(TLV(tag_ix,   sys::trc_vsnprintf(tlv_buff, tlv_buff_sz,   "%i", x), tlv_buff));
		e.Encode(TLV(tag_iy,   sys::trc_vsnprintf(tlv_buff, tlv_buff_sz,   "%i", y), tlv_buff));
		e.Encode(TLV(tag_fgc,  sys::trc_vsnprintf(tlv_buff, tlv_buff_sz,   "#%02x%02x%02x", fg.r, fg.g, fg.b), tlv_buff));
		e.Encode(TLV(tag_bgc,  sys::trc_vsnprintf(tlv_buff, tlv_buff_sz,   "#%02x%02x%02x", bg.r, bg.g, bg.b), tlv_buff));
		encode_va_fields(e, fmt, args);
		if (e.Commit())
		{
			msg.m_length = e.total_len;
		}
	}


	inline void encode_table_setup_hhdr (msg_t & msg, level_t level, context_t context, int x, char const * name, char const * fmt, va_list args)
	{
		size_t const tlv_buff_sz = 256;
		char tlv_buff[tlv_buff_sz];
		using namespace tlv;
		tlv::Encoder_v1 e(tlv::cmd_table_setup, msg.m_data, msg_t::e_data_sz);
		e.Encode(TLV(tag_ctime, sys::trc_vsnprintf(tlv_buff, tlv_buff_sz, "%llu", sys::queryTime_us()), tlv_buff));
		e.Encode(TLV(tag_lvl,  sys::trc_vsnprintf(tlv_buff, tlv_buff_sz,   "%u", level), tlv_buff));
		e.Encode(TLV(tag_ctx,  sys::trc_vsnprintf(tlv_buff, tlv_buff_sz,   "%x", context), tlv_buff));
		e.Encode(TLV(tag_ix,   sys::trc_vsnprintf(tlv_buff, tlv_buff_sz,   "%i", x), tlv_buff));
		encode_str(e, tag_hhdr, name);
		encode_va_fields(e, tag_msg, fmt, args);
		if (e.Commit())
		{
			msg.m_length = e.total_len;
		}
	}

	inline void encode_table_setup_color (msg_t & msg, level_t level, context_t context, int x, int y, Color c, char const * fmt, va_list args)
	{
		size_t const tlv_buff_sz = 256;
		char tlv_buff[tlv_buff_sz];
		using namespace tlv;
		tlv::Encoder_v1 e(tlv::cmd_table_setup, msg.m_data, msg_t::e_data_sz);
		e.Encode(TLV(tag_ctime, sys::trc_vsnprintf(tlv_buff, tlv_buff_sz, "%llu", sys::queryTime_us()), tlv_buff));
		e.Encode(TLV(tag_lvl,  sys::trc_vsnprintf(tlv_buff, tlv_buff_sz,   "%u", level), tlv_buff));
		e.Encode(TLV(tag_ctx,  sys::trc_vsnprintf(tlv_buff, tlv_buff_sz,   "%x", context), tlv_buff));
		e.Encode(TLV(tag_ix,   sys::trc_vsnprintf(tlv_buff, tlv_buff_sz,   "%i", x), tlv_buff));
		e.Encode(TLV(tag_y,    sys::trc_vsnprintf(tlv_buff, tlv_buff_sz,   "%i", y), tlv_buff));
		e.Encode(TLV(tag_fgc,  sys::trc_vsnprintf(tlv_buff, tlv_buff_sz,   "#%02x%02x%02x", c.r, c.g, c.b), tlv_buff));
		encode_va_fields(e, tag_msg, fmt, args);
		if (e.Commit())
		{
			msg.m_length = e.total_len;
		}
	}
	inline void encode_table_setup_color (msg_t & msg, level_t level, context_t context, int x, int y, Color fg, Color bg, char const * fmt, va_list args)
	{
		size_t const tlv_buff_sz = 256;
		char tlv_buff[tlv_buff_sz];
		using namespace tlv;
		tlv::Encoder_v1 e(tlv::cmd_table_setup, msg.m_data, msg_t::e_data_sz);
		e.Encode(TLV(tag_ctime, sys::trc_vsnprintf(tlv_buff, tlv_buff_sz, "%llu", sys::queryTime_us()), tlv_buff));
		e.Encode(TLV(tag_lvl,  sys::trc_vsnprintf(tlv_buff, tlv_buff_sz,   "%u", level), tlv_buff));
		e.Encode(TLV(tag_ctx,  sys::trc_vsnprintf(tlv_buff, tlv_buff_sz,   "%x", context), tlv_buff));
		e.Encode(TLV(tag_ix,   sys::trc_vsnprintf(tlv_buff, tlv_buff_sz,   "%i", x), tlv_buff));
		e.Encode(TLV(tag_y,    sys::trc_vsnprintf(tlv_buff, tlv_buff_sz,   "%i", y), tlv_buff));
		e.Encode(TLV(tag_fgc,  sys::trc_vsnprintf(tlv_buff, tlv_buff_sz,   "#%02x%02x%02x", fg.r, fg.g, fg.b), tlv_buff));
		e.Encode(TLV(tag_bgc,  sys::trc_vsnprintf(tlv_buff, tlv_buff_sz,   "#%02x%02x%02x", bg.r, bg.g, bg.b), tlv_buff));
		encode_va_fields(e, tag_msg, fmt, args);
		if (e.Commit())
		{
			msg.m_length = e.total_len;
		}
	}
}

