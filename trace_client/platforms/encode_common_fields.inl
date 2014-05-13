#pragma once
#include "include_ntohs.h"
#include <tlv_parser/tlv_parser.h>
#include <tlv_parser/tlv_encoder.h>

namespace trace {

	template <class Encoder>
	void encode_va_fields (Encoder & e, char const * fmt, va_list args)
	{
		size_t const tlv_buff_sz = 1024;
		char tlv_buff[tlv_buff_sz];
		using namespace tlv;
		e.Encode(TLV(tag_msg, sys::va_trc_vsnprintf(tlv_buff, tlv_buff_sz, fmt, args), tlv_buff));
	}

	template <class Encoder>
	void encode_va_fields (Encoder & e, tlv::tag_t tag, char const * fmt, va_list args)
	{
		size_t const tlv_buff_sz = 1024;
		char tlv_buff[tlv_buff_sz];
		using namespace tlv;
		e.Encode(TLV(tag, sys::va_trc_vsnprintf(tlv_buff, tlv_buff_sz, fmt, args), tlv_buff));
	}

	template <class Encoder>
	void encode_str (Encoder & e, tlv::tag_t tag, char const * str)
	{
		size_t const tlv_buff_sz = 1024;
		char tlv_buff[tlv_buff_sz];
		using namespace tlv;
		e.Encode(TLV(tag, sys::trc_vsnprintf(tlv_buff, tlv_buff_sz, "%s", str), tlv_buff)); // @TODO: use snprintf
	}

	template <class Encoder>
	void encode_common_fields (Encoder & e, level_t level, context_t context, char const * file, int line, char const * fn)
	{
		size_t const tlv_buff_sz = 256;
		char tlv_buff[tlv_buff_sz];
		using namespace tlv;
		e.Encode(TLV(tag_ctime, sys::trc_vsnprintf(tlv_buff, tlv_buff_sz, "%llu", sys::queryTime_us()), tlv_buff));
		e.Encode(TLV(tag_lvl,  sys::trc_vsnprintf(tlv_buff, tlv_buff_sz, "%u", level), tlv_buff));
		e.Encode(TLV(tag_ctx,  sys::trc_vsnprintf(tlv_buff, tlv_buff_sz, "%x", context), tlv_buff));
		e.Encode(TLV(tag_tid,  sys::trc_vsnprintf(tlv_buff, tlv_buff_sz, "%u", sys::get_tid()), tlv_buff));
		e.Encode(TLV(tag_file, sys::trc_vsnprintf(tlv_buff, tlv_buff_sz, "%s", file), tlv_buff));
		e.Encode(TLV(tag_line, sys::trc_vsnprintf(tlv_buff, tlv_buff_sz, "%u", line), tlv_buff));
		e.Encode(TLV(tag_func, sys::trc_vsnprintf(tlv_buff, tlv_buff_sz, "%s", fn), tlv_buff));
	}
}
