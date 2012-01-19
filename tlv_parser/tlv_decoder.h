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
#include <cstring>
#include "memstream.h"
#ifdef __CYGWIN__
#	include <netinet/in.h>	// for ntohs
#else
#	include <winsock2.h> // for ntohs
#endif

namespace tlv {

	inline bool decode_hdr (memstream & stream, Header & h)
	{
		cmd_t command = 0;
		if (!stream.read(reinterpret_cast<char * >(&command), sizeof(cmd_t)))
			return false;
		h.cmd = command;		//qDebug("DEC: hdr.cmd=%02x\n", h.cmd);
		
		len_t ln = 0;
		if (!stream.read(reinterpret_cast<char * >(&ln), sizeof(len_t)))
			return false;
        h.len = ntohs(ln);		//qDebug("DEC: hdr.len=%02x hd.len_ntohs=%02x\n", h.len, ntohs(h.len));
		return true;
	}

	/**@class	TVDecoder
	 * @brief	interprets stream of bytes as TLV protocol and creates StringCommand
	 */
	struct TVDecoder
	{
		TVDecoder () { }

		bool decode_header (char const * buff, size_t ln, StringCommand & cmd)
		{
			memstream input(buff, ln);
			return decode_hdr(input, cmd.hdr);
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
			tv.m_val = std::string(local_buffer, local_buffer + len);
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

	/**@class	Decoder
	 * @brief	main encoder class
	 */
	struct TLVDecoder
	{
		TLVDecoder () { }

		template<unsigned N, unsigned M>
		bool decode_header (char const * buff, size_t ln, Command<N, M> & cmd)
		{
			memstream input(buff, ln);
			return decode_hdr(input, cmd.hdr);
		}

		template<unsigned N, unsigned M>
		bool decode_payload (char const * buff, size_t ln, Command<N, M> & cmd)
		{
			memstream input(buff, ln);
			return decode(input, cmd, cmd.concat_values);
		}

		bool decode (memstream & s, TLV & tlv, char * value_buffer)
		{
			tag_t tag = 0;
			if (!s.read(reinterpret_cast<char * >(&tag), sizeof(tag_t)))
				return false;
			tlv.m_tag = tag;		//qDebug("DEC: tlv.tag=%02x", tag);

			len_t len = 0;
			if (!s.read(reinterpret_cast<char * >(&len), sizeof(len_t)))
				return false;
			tlv.m_len = len;		//qDebug("DEC: tlv.len=%04x", len);

			tlv.m_val = &value_buffer[0];
			if (!s.read(value_buffer, len))
				return false;
			value_buffer[len + 1] = '\0';		//qDebug("DEC: val=%s ", value_buffer);
			return true;
		}

		template<unsigned N, unsigned M>
		bool decode (memstream & s, Command<N,M> & cmd, char * value_buffer)
		{
			bool result = true;
			while (s.read_peek())
			{
				if (cmd.tlvs_count < M)
				{
					TLV & tlv = cmd.tlvs[cmd.tlvs_count];
					result &= decode(s, tlv, value_buffer);
					value_buffer += tlv.m_len + 1; // +1 for the terminating zero
					if (result)
						++cmd.tlvs_count;
				}
				else
					break;
			}
			return result;
		}
	};
}
