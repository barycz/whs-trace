#pragma once
#include "include_ntohs.h"
#include <tlv_parser/tlv_parser.h>
#include <tlv_parser/tlv_encoder.h>

namespace trace {

	template <class DictT>
	inline void encode_dict (msg_t & msg, tlv::cmd_t cmd, DictT const * ptr, size_t n)
	{
		tlv::Encoder_v1 e(cmd, msg.m_data, msg_t::e_data_sz);
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

