#pragma once
#include "include_ntohs.h"
#include <tlv_parser/tlv_parser.h>
#include <tlv_parser/tlv_encoder.h>

namespace trace {

	inline void encode_exportCSV (msg_t & msg, char const * filename)
	{
		tlv::Encoder_v1 e(tlv::cmd_export_csv, msg.m_data, msg_t::e_data_sz);
		size_t const tlv_buff_sz = 256;
		char tlv_buff[tlv_buff_sz];
		using namespace tlv;
		e.Encode(TLV(tag_file,  sys::trc_vsnprintf(tlv_buff, tlv_buff_sz, "%s", filename), tlv_buff));
		if (e.Commit())
		{
			msg.m_length = e.total_len;
		}
	}
}

