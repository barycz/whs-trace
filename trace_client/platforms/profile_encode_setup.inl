#pragma once
#include "include_ntohs.h"
#include <tlv_parser/tlv_parser.h>
#include <tlv_parser/tlv_encoder.h>

namespace profile {

	inline void encode_setup (msg_t & msg)
	{
		tlv::Encoder_v1 e(tlv::cmd_setup, msg.m_data, msg_t::e_data_sz);
		size_t const tlv_buff_sz = 128;
		char tlv_buff[tlv_buff_sz];
		using namespace tlv;
		e.Encode(TLV(tag_app,  sys::trc_vsnprintf(tlv_buff, tlv_buff_sz, "%s", GetAppName()), tlv_buff));
		e.Encode(TLV(tag_pid,  sys::trc_vsnprintf(tlv_buff, tlv_buff_sz, "%u", sys::get_pid()), tlv_buff));
		if (e.Commit())
		{
			msg.m_length = e.total_len;
		}
	}
}
