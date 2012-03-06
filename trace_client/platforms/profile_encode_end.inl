#pragma once
#include "include_ntohs.h"
#include "../../tlv_parser/tlv_parser.h"
#include "../../tlv_parser/tlv_encoder.h"
#include "profile_encode_common_fields.inl"

namespace profile {

	inline void encode_end (sys::msg_t & msg)
	{
		using namespace tlv;
		tlv::Encoder e(tlv::cmd_profile_end, msg.m_data, sys::msg_t::e_data_sz);
		encode_common_fields(e);
		if (e.Commit())
		{
			msg.m_length = e.total_len;
		}
	}

	inline void encode_frame_end (sys::msg_t & msg)
	{
		using namespace tlv;
		tlv::Encoder e(tlv::cmd_profile_frame_end, msg.m_data, sys::msg_t::e_data_sz);
		encode_common_fields(e);
		if (e.Commit())
		{
			msg.m_length = e.total_len;
		}
	}
}
