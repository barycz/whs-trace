#pragma once
#include "include_ntohs.h"
#include <tlv_parser/tlv_parser.h>
#include <tlv_parser/tlv_encoder.h>

namespace trace {

	inline void encode_ctx_dict (msg_t & msg, CtxDictPair const * ptr, size_t n)
	{
		tlv::Encoder_v1 e(tlv::cmd_dict_ctx, msg.m_data, msg_t::e_data_sz);
		size_t const tlv_buff_sz = 256;
		char tlv_buff[tlv_buff_sz];
		using namespace tlv;
		for (size_t i = 0; i < n; ++i)
		{
			e.Encode(TLV(tag_string,  sys::trc_vsnprintf(tlv_buff, tlv_buff_sz, "%s", ptr[i].first ), tlv_buff));
			e.Encode(TLV(tag_int,     sys::trc_vsnprintf(tlv_buff, tlv_buff_sz, "%x", ptr[i].second), tlv_buff));
		}
		if (e.Commit())
		{
			msg.m_length = e.total_len;
		}
	}
}

