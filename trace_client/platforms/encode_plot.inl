#pragma once
#include "include_ntohs.h"
#include <tlv_parser/tlv_parser.h>
#include <tlv_parser/tlv_encoder.h>
#include "encode_common_fields.inl"

namespace trace {

	inline void encode_plot (msg_t & msg, level_t level, context_t context, float x, float y, char const * fmt, va_list args)
	{
		size_t const tlv_buff_sz = 256;
		char tlv_buff[tlv_buff_sz];
		using namespace tlv;
		tlv::Encoder_v1 e(tlv::cmd_plot_xy, msg.m_data, msg_t::e_data_sz);
		e.Encode(TLV(tag_ctime, sys::trc_vsnprintf(tlv_buff, tlv_buff_sz, "%llu", sys::queryTime_us()), tlv_buff));
		e.Encode(TLV(tag_lvl,  sys::trc_vsnprintf(tlv_buff, tlv_buff_sz, "%u", level), tlv_buff));
		e.Encode(TLV(tag_ctx,  sys::trc_vsnprintf(tlv_buff, tlv_buff_sz, "%x", context), tlv_buff));
		e.Encode(TLV(tag_x,    sys::trc_vsnprintf(tlv_buff, tlv_buff_sz, "%f", x), tlv_buff));
		e.Encode(TLV(tag_y,    sys::trc_vsnprintf(tlv_buff, tlv_buff_sz, "%f", y), tlv_buff));
		encode_va_fields(e, fmt, args);
		if (e.Commit())
		{
			msg.m_length = e.total_len;
		}
	}

	inline void encode_plot (msg_t & msg, level_t level, context_t context, float x, float y, float z, char const * fmt, va_list args)
	{
		size_t const tlv_buff_sz = 256;
		char tlv_buff[tlv_buff_sz];
		using namespace tlv;
		tlv::Encoder_v1 e(tlv::cmd_plot_xy, msg.m_data, msg_t::e_data_sz);
		e.Encode(TLV(tag_ctime, sys::trc_vsnprintf(tlv_buff, tlv_buff_sz, "%llu", sys::queryTime_us()), tlv_buff));
		e.Encode(TLV(tag_lvl,  sys::trc_vsnprintf(tlv_buff, tlv_buff_sz, "%u", level), tlv_buff));
		e.Encode(TLV(tag_ctx,  sys::trc_vsnprintf(tlv_buff, tlv_buff_sz, "%x", context), tlv_buff));
		e.Encode(TLV(tag_x,    sys::trc_vsnprintf(tlv_buff, tlv_buff_sz, "%f", x), tlv_buff));
		e.Encode(TLV(tag_y,    sys::trc_vsnprintf(tlv_buff, tlv_buff_sz, "%f", y), tlv_buff));
		e.Encode(TLV(tag_z,    sys::trc_vsnprintf(tlv_buff, tlv_buff_sz, "%f", z), tlv_buff));
		encode_va_fields(e, fmt, args);
		if (e.Commit())
		{
			msg.m_length = e.total_len;
		}
	}

	inline void encode_plot_clear (msg_t & msg, level_t level, context_t context, char const * fmt, va_list args)
	{
		size_t const tlv_buff_sz = 256;
		char tlv_buff[tlv_buff_sz];
		using namespace tlv;
		tlv::Encoder_v1 e(tlv::cmd_plot_clear, msg.m_data, msg_t::e_data_sz);
		e.Encode(TLV(tag_ctime, sys::trc_vsnprintf(tlv_buff, tlv_buff_sz, "%llu", sys::queryTime_us()), tlv_buff));
		e.Encode(TLV(tag_lvl,  sys::trc_vsnprintf(tlv_buff, tlv_buff_sz, "%u", level), tlv_buff));
		e.Encode(TLV(tag_ctx,  sys::trc_vsnprintf(tlv_buff, tlv_buff_sz, "%x", context), tlv_buff));
		encode_va_fields(e, fmt, args);
		if (e.Commit())
		{
			msg.m_length = e.total_len;
		}
	}

}

