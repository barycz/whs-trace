/**
 * Copyright (C) 2011 Mojmir Svoboda
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is furnished to do
 * so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 **/
#pragma once
#include <string>
#include "tlv.h"

namespace tlv {

	//////////////////// std::string oriented command /////////////////////
	struct TV
	{
		tlv::tag_t        m_tag;	// 1 Byte tag
		std::string       m_val;	// payload

		TV (tag_t t, char const * v) : m_tag(t) , m_val(v) { }
		TV () : m_tag(0) , m_val() { }
		len_t len () const { return static_cast<len_t>(m_val.length()); }
	};
	typedef std::vector<TV> tvs_t;

	struct StringCommand
	{
		tlv::Header hdr;
		tvs_t tvs;

		StringCommand () : hdr(0, 0) { }

		void Reset ()
		{
			hdr.Reset();
			tvs.clear();
		}
	};

	/**@class	TVDecoder
	 * @brief	interprets stream of bytes as TLV protocol and creates StringCommand
	 */
	struct TVDecoder_v1
	{
		TVDecoder_v1 () { }

		bool decode_header (char const * buff, size_t ln, StringCommand & cmd)
		{
			memstream input(buff, ln);
			return decode_hdr_v1(input, cmd.hdr);
		}

		bool decode_payload (char const * buff, size_t ln, StringCommand & cmd)
		{
			memstream input(buff, ln);
			return decode(input, cmd.tvs);
		}

		bool decode (memstream & stream, TV & tv)
		{
			tag_t tag = 0;
			if (!stream.read(reinterpret_cast<char * >(&tag), sizeof(tag_t)))
				return false;
			tv.m_tag = tag;

			len_t len = 0;
			if (!stream.read(reinterpret_cast<char * >(&len), sizeof(len_t)))
				return false;

			char local_buffer[512]; // @#%!@%@%@#$%!@$%@#$%  assert len < e_buff_sz
			if (!stream.read(local_buffer, len))
				return false;
#if defined USE_QT_STRING
			tv.m_val = QString::fromAscii(local_buffer, len);
#else
			tv.m_val = std::string(local_buffer, len);
#endif
			//qDebug("DEC: len=%u str.ln=%u strl.val=%s ", len, tv.val.length(), tv.val.c_str());
			return true;
		}

		bool decode (memstream & stream, tvs_t & tvs)
		{
			bool result = true;
			while (stream.read_peek())
			{
				tvs.push_back(TV());
				bool const decode_ok = decode(stream, tvs.back());
				if (!decode_ok)
					tvs.pop_back();	// remove invalid tag and value if decoding failed
				result &= decode_ok;
			}
			return result;
		}
	};


}

