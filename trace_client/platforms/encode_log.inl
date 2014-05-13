#pragma once
#include "include_ntohs.h"
#include <tlv_parser/tlv_parser.h>
#include <tlv_parser/tlv_encoder.h>
#include "encode_common_fields.inl"

namespace trace {

	inline void encode_log (msg_t & msg, level_t level, context_t context, char const * file, int line, char const * fn, char const * fmt, va_list args)
	{
		using namespace tlv;
		tlv::Encoder_v1 e(tlv::cmd_log, msg.m_data, msg_t::e_data_sz);
		encode_common_fields(e, level, context, file, line, fn);
		encode_va_fields(e, fmt, args);
		if (e.Commit())
		{
			msg.m_length = e.total_len;
		}
	}

	inline void encode_str (msg_t & msg, level_t level, context_t context, char const * file, int line, char const * fn, char const * str)
	{
		using namespace tlv;
		tlv::Encoder_v1 e(tlv::cmd_log, msg.m_data, msg_t::e_data_sz);
		encode_common_fields(e, level, context, file, line, fn);
		encode_str(e, tag_msg, str);
		if (e.Commit())
		{
			msg.m_length = e.total_len;
		}
	}

	inline void encode_log_clear (msg_t & msg, level_t level, context_t context)
	{
		size_t const tlv_buff_sz = 256;
		char tlv_buff[tlv_buff_sz];
		using namespace tlv;
		tlv::Encoder_v1 e(tlv::cmd_log_clear, msg.m_data, msg_t::e_data_sz);
		e.Encode(TLV(tag_ctime, sys::trc_vsnprintf(tlv_buff, tlv_buff_sz, "%llu", sys::queryTime_us()), tlv_buff));
		e.Encode(TLV(tag_lvl,  sys::trc_vsnprintf(tlv_buff, tlv_buff_sz, "%u", level), tlv_buff));
		e.Encode(TLV(tag_ctx,  sys::trc_vsnprintf(tlv_buff, tlv_buff_sz, "%x", context), tlv_buff));
		if (e.Commit())
		{
			msg.m_length = e.total_len;
		}
	}

}

