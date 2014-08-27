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
#include <vector>
#include "tlv.h"

namespace tlv {

	//////////////////// char * oriented command /////////////////////
	struct TLV
	{
		tag_t        m_tag;	// 1 Byte tag
		len_t        m_len;	// 2 Byte length of payload
		char const * m_val;	// payload

		TLV () : m_tag(tag_invalid), m_len(0), m_val(0) { }
		TLV (tag_t t, char const * v) : m_tag(t) , m_len(static_cast<len_t>(strlen(v))) , m_val(v) { } // constructor requires 0 terminated string
		TLV (tag_t t, size_t l, char const * v) : m_tag(t) , m_len(static_cast<len_t>(l)) , m_val(v) { }

		len_t len () const { return m_len; }
	};

	template<size_t N, size_t M>
	struct Command
	{
		static unsigned const max_buff_size = N;
		static unsigned const max_tlvs = M;

		typedef TLV tlvs_t[max_tlvs];		/// array of TLVs
		Header hdr;							/// header
		unsigned tlvs_count;				/// count of TLVs
		tlvs_t tlvs;						/// array of TLV
		char concat_values[max_buff_size];	/// internal concantenated values from list of TLV
	
		Command() : hdr(1, 0, 0), tlvs_count(0) { Reset(); }
		void Reset ()
		{
			memset(tlvs, 0, sizeof(tlvs));
			memset(concat_values, 0, sizeof(concat_values));
			hdr.Reset();
			tlvs_count = 0;
		}
	};

	/**@class	Encoder
	 * @brief	main encoder class
	 */
	struct Encoder_v1
	{
		len_t total_len;		/// total message length including header
		len_t data_len;			/// payload (data) length (i.e. without header)
		memstream output;		/// encapsulation of raw buffer
		char * buffer;			///	raw buffer


		Encoder_v1 (cmd_t cmd, char * buff, size_t ln) 
			: total_len(0) , data_len(0) , output(buff, ln) , buffer(buff) 
		{
			Header h(1, cmd, 0);
			Encode(h);
		}

		bool Commit ()
		{
			bool const is_ok = !output.write_eof();
			if (is_ok)
			{
				total_len = static_cast<len_t>(output.total_write_len());
				data_len = total_len - Header::e_Size;
                len_t const encoded_ln = htons(data_len);
                buffer[sizeof(unsigned char) + sizeof(cmd_t)    ] = reinterpret_cast<char const *>(&encoded_ln)[0];
                buffer[sizeof(unsigned char) + sizeof(cmd_t) + 1] = reinterpret_cast<char const *>(&encoded_ln)[1]; // write length of data
			}
			return is_ok;
		}

		void Encode (Header h)
		{
			output.write(reinterpret_cast<char const * >(&h.version), sizeof(unsigned char));
			output.write(reinterpret_cast<char const * >(&h.cmd), sizeof(cmd_t));
			output.write(reinterpret_cast<char const * >(&h.len), sizeof(len_t));
		}

		void Encode (TLV const & tlv)
		{
			// Woody: Replace unwanted 0x0d and 0x0a anywhere in the string. It would later destroy exported .CSV
			// TODO: Remove this code once implemeted to TraceServer CSV export. We can't compile it yet
			for(size_t i = tlv.m_len; i--;)
			{
				if((tlv.m_val[i] == 0x0d) ||
					 (tlv.m_val[i] == 0x0a))
				{
					((char*)tlv.m_val)[i] = ' ';
				}
			}

			output.write(reinterpret_cast<char const * >(&tlv.m_tag), sizeof(tag_t));
			output.write(reinterpret_cast<char const * >(&tlv.m_len), sizeof(len_t));
			output.write(reinterpret_cast<char const * >(&tlv.m_val[0]), tlv.len());
		}
	};


	/**@class	Decoder
	 * @brief	main encoder class
	 */
	struct TLVDecoder
	{
		TLVDecoder () { }

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
		bool decode_header (char const * buff, size_t ln, Command<N, M> & cmd)
		{
			memstream input(buff, ln);
			return decode_hdr(input, cmd.hdr);
		}

		template<unsigned N, unsigned M>
		bool decode_payload (char const * buff, size_t ln, Command<N, M> & cmd)
		{
			memstream input(buff, ln);
			return decode<N, M>(input, cmd, cmd.concat_values);
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

