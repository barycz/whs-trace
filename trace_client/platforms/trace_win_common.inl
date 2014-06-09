#pragma once
#include <cstdio>
#include <tlv_parser/tlv_parser.h>
#include <sysfn/atomic_win.h>
#include <sysfn/select_atomic.h>
#include <sysfn/time_query.h>
#include <sysfn/os.h>
#include "msg.h"

namespace trace {

	void create_log_filename (char * filename, size_t buff_sz)
	{
		char const * app_name = GetAppName() ? GetAppName() : "unknown";
		_snprintf_s(filename, buff_sz, buff_sz - 1, "%s_%u.trace", app_name, ::GetCurrentProcessId());
	}

	typedef sys::Message<1536> msg_t;

	/**@brief	simple pool of messages to be logged **/
	template <class T, unsigned N>
	struct MessagePool
	{
		enum { e_size = N };
		CACHE_ALIGN msg_t m_msgs[e_size];

		MessagePool () { memset(this, 0, sizeof(*this)); }
		msg_t & operator[] (size_t i) { return m_msgs[i]; }
		msg_t const & operator[] (size_t i) const { return m_msgs[i]; }
	};
}

