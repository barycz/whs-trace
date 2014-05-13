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
#if defined __CYGWIN__
#	include <netinet/in.h>	// for ntohs
#elif defined __linux__
#	include <arpa/inet.h>
#elif defined WIN32 || defined WIN64
#	include <winsock2.h> // for ntohs
#endif

namespace tlv {

	inline bool decode_hdr (memstream & stream, Header & h)
	{
		unsigned char version = 0;
		if (!stream.read(reinterpret_cast<char * >(&version), sizeof(unsigned char)))
			return false;
		h.version = version;

		if (version != 1)
			return false;

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

}
