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
#include <QString>
#include <vector>
#include "tlv.h"
#include "memstream.h"
#include "tlv_decoder.h"

namespace tlv {

	//////////////////// QString oriented command /////////////////////
	struct TV
	{
		tlv::tag_t        m_tag;	// 1 Byte tag
		QString			  m_val;	// payload

		TV (tag_t t, char const * v) : m_tag(t) , m_val(v) { }
		TV () : m_tag(0) , m_val() { }
		len_t len () const { return static_cast<len_t>(m_val.length()); }
	};
	typedef std::vector<TV> tvs_t;

	struct StringCommand_v1
	{
		tlv::Header m_hdr;
		tvs_t m_tvs;

		StringCommand_v1 () : m_hdr(1, 0, 0) { }

		void reset ()
		{
			m_hdr.Reset();
			m_tvs.clear();
		}
	};

	/**@class	HeaderDecoder
	 * @brief	decodes header
	 */
	struct HeaderDecoder
	{
		HeaderDecoder () { }

		bool decode_header (char const * buff, size_t ln, tlv::Header & hdr)
		{
			memstream input(buff, ln);
			return decode_hdr(input, hdr);
		}
	};

	/**@class	TVDecoder_v1
	 * @brief	interprets stream of bytes as TLV protocol and creates StringCommand_v1
	 */
	struct TVDecoder_v1
	{
		TVDecoder_v1 () { }

		bool decode_payload (char const * buff, size_t ln, StringCommand_v1 & cmd)
		{
			memstream input(buff, ln);
			return decode(input, cmd.m_tvs);
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

			size_t const buffsize = 512;
			QString val;
			val.reserve(2 * buffsize);
			size_t rd = 0;
			while (rd < len)
			{
				char local_buffer[buffsize];
				size_t const remaining = len - rd;
				size_t const to_read = std::min(remaining, buffsize);
				size_t const consumed = stream.read(local_buffer, to_read);
				if (consumed == 0)
					return false;
				val += QString::fromLatin1(local_buffer, static_cast<int>(consumed));
				rd += to_read;
			}
			val.squeeze();
			tv.m_val = val;
			//qDebug("DEC: len=%u str.ln=%u strl.val=%s ", len, tv.m_val.toStdString().length(), tv.m_val.toStdString().c_str());
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

