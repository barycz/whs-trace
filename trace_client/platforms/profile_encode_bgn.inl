#pragma once
#include "include_ntohs.h"
#include <tlv_parser/tlv_parser.h>
#include <tlv_parser/tlv_encoder.h>
#include "profile_encode_common_fields.inl"

namespace profile {

	inline void encode_bgn (msg_t & msg)
	{
		using namespace tlv;
		tlv::Encoder_v1 e(tlv::cmd_profile_bgn, msg.m_data, msg_t::e_data_sz);
		encode_common_fields(e);
		if (e.Commit())
		{
			msg.m_length = e.total_len;
		}
	}

	inline void encode_frame_bgn (msg_t & msg)
	{
		using namespace tlv;
		tlv::Encoder_v1 e(tlv::cmd_profile_frame_bgn, msg.m_data, msg_t::e_data_sz);
		encode_common_fields(e);
		if (e.Commit())
		{
			msg.m_length = e.total_len;
		}
	}

	inline void encode_bgn (msg_t & msg, char const * fmt, va_list args)
	{
		using namespace tlv;
		tlv::Encoder_v1 e(tlv::cmd_profile_bgn, msg.m_data, msg_t::e_data_sz);
		encode_common_fields(e);
		encode_va_fields(e, fmt, args);
		if (e.Commit())
		{
			msg.m_length = e.total_len;
		}
	}

	inline void encode_frame_bgn (msg_t & msg, char const * fmt, va_list args)
	{
		using namespace tlv;
		tlv::Encoder_v1 e(tlv::cmd_profile_frame_bgn, msg.m_data, msg_t::e_data_sz);
		encode_common_fields(e);
		encode_va_fields(e, fmt, args);
		if (e.Commit())
		{
			msg.m_length = e.total_len;
		}
	}
}
