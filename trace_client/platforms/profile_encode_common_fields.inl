#pragma once
#include "include_ntohs.h"
#include <tlv_parser/tlv_parser.h>
#include <tlv_parser/tlv_encoder.h>

namespace profile {

	template<class Encoder>
	void encode_va_fields (Encoder & e, char const * fmt, va_list args)
	{
		size_t const tlv_buff_sz = 256;
		char tlv_buff[tlv_buff_sz];
		using namespace tlv;
		e.Encode(TLV(tag_msg, sys::va_trc_vsnprintf(tlv_buff, tlv_buff_sz, fmt, args), tlv_buff));
	}

	template<class Encoder>
	void encode_common_fields (Encoder & e)
	{
		size_t const tlv_buff_sz = 256;
		char tlv_buff[tlv_buff_sz];
		using namespace tlv;
		e.Encode(TLV(tag_ctime, sys::trc_vsnprintf(tlv_buff, tlv_buff_sz, "%llu", sys::queryTime_us()), tlv_buff));
		e.Encode(TLV(tag_tid,  sys::trc_vsnprintf(tlv_buff, tlv_buff_sz, "%u", sys::get_tid()), tlv_buff));
	}
}
