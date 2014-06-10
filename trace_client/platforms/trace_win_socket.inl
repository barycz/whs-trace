#include <tlv_parser/tlv_parser.h>
#include <tlv_parser/tlv_decoder.h>
#include <sysfn/socket_win.h>
#include "trace_win_common.inl"
#include "encode_log.inl"
#include "encode_plot.inl"
#include "encode_table.inl"
#include "encode_gantt.inl"
#include "encode_scope.inl"
#include "encode_setup.inl"
#include "encode_exportcsv.inl"
#include "encode_ctx_dict.inl"

//#define DBG_OUT printf
#define DBG_OUT(fmt, ...) ((void)0)

namespace trace {

	namespace socks {

		using namespace sys::socks;

		typedef MessagePool<msg_t, 16384> message_pool_t;

		sys::atomic32_t volatile g_Quit = 0;			/// request to quit
		sys::atomic32_t volatile g_Flushed = 0;			/// request to quit
		CACHE_ALIGN sys::atomic32_t volatile m_wr_idx = 0;		// write index
		message_pool_t g_MessagePool;					// pool of messages
		CACHE_ALIGN sys::atomic32_t volatile m_rd_idx = 0;		// read index

		sys::Thread g_ThreadSend(THREAD_PRIORITY_HIGHEST);		/// consumer-sender thread (high priority)
		sys::Thread g_ThreadRecv(THREAD_PRIORITY_LOWEST);		/// receiving thread (low priority)
		socket_t g_Socket = INVALID_SOCKET;
		HANDLE g_LogFile = INVALID_HANDLE_VALUE;
		sys::Timer g_ReconnectTimer;
		sys::Timer g_ClottedTimer;

		inline msg_t & msg_buffer_at (size_t i)
		{
			return g_MessagePool[i];
		}

		inline msg_t & acquire_msg_buffer ()
		{
			sys::atomic32_t wr_idx = InterlockedIncrement(&m_wr_idx);
			return msg_buffer_at((wr_idx - 1) % message_pool_t::e_size);
		}

		inline bool is_file_connected () { return g_LogFile != INVALID_HANDLE_VALUE; }

		inline bool WriteToFile (char const * buff, size_t ln)
		{
			if (is_file_connected())
			{
				DWORD written;
				WriteFile(g_LogFile, buff, static_cast<DWORD>(ln), &written, 0);
				return true;
			}
			return false;
		}

		inline bool WriteToSocket (char const * buff, size_t ln)
		{
			if (sys::socks::is_connected(socks::g_Socket))
			{
				int result = SOCKET_ERROR;
				if (!g_ClottedTimer.enabled())
					result = send(g_Socket, buff, (int)ln, 0);
				else
				{
					if (g_ClottedTimer.expired())
					{
						result = send(g_Socket, buff, (int)ln, 0);
						if (result != SOCKET_ERROR && result > 0)
						{
							DBG_OUT("declotted\n");
							g_ClottedTimer.reset();
						}
					}
					else
					{
#ifdef TRACE_WINDOWS_SOCKET_FAILOVER_TO_FILE
						WriteToFile(buff, ln);
#	ifdef TRACE_WINDOWS_SOCKET_FAILOVER_NOTIFY_MSVC
						OutputDebugStringA("dropped some log messages!");
#	endif
#endif
						return true;
					}
				}

				bool const timeouted = ( result == SOCKET_ERROR && is_timeouted());
				if (result == SOCKET_ERROR && !timeouted)
				{
					DBG_OUT("send failed with error: %d\n", get_errno());
					closesocket(g_Socket);
					WSACleanup();
					g_Socket = INVALID_SOCKET;

#ifdef TRACE_WINDOWS_SOCKET_FAILOVER_TO_FILE
					WriteToFile(buff, ln);
#	ifdef TRACE_WINDOWS_SOCKET_FAILOVER_NOTIFY_MSVC
					OutputDebugStringA("dropping log messages!");
#	endif
#endif
					return false;
				}
				else if (timeouted || result == 0)
				{
					DBG_OUT("socked clotted\n");
					g_ClottedTimer.set_delay_ms(128);
					return true;
				}
			}
#ifdef TRACE_WINDOWS_SOCKET_FAILOVER_TO_FILE
			else
			{
				WriteToFile(buff, ln);
#	ifdef TRACE_WINDOWS_SOCKET_FAILOVER_NOTIFY_MSVC
				OutputDebugStringA("dropping log messages!");
#	endif
			}
#endif
			DBG_OUT(".");
			return true;
		}

		/**@brief	function receiving commands from server **/
		DWORD WINAPI receive_thread ( LPVOID )
		{
			enum { e_buff_sz = 256 };
			char buff[e_buff_sz];
			tlv::Command<256, 8> cmd; // reserve 4 tlvs with maximum of 128 bytes of concatenated data
			while (!g_Quit)
			{
				int const result = recv(g_Socket, buff, e_buff_sz, 0); //@TODO: better recv logic (this is sufficent for now)
				if (result > 0)
				{
					cmd.Reset();
					tlv::TLVDecoder d;
					if (d.decode_header(buff, result, cmd))
					{
						if (d.decode_payload(buff + tlv::Header::e_Size, cmd.hdr.len, cmd))
						{
							if (cmd.hdr.cmd == tlv::cmd_set_level && cmd.tlvs_count > 0)
							{
								printf("level changed!\n");
								SetRuntimeLevel(static_cast<trace::level_t>(atoi(cmd.tlvs[0].m_val)));
							}

							if (cmd.hdr.cmd == tlv::cmd_set_buffering && cmd.tlvs_count > 0)
							{
								unsigned const buff_state = atoi(cmd.tlvs[0].m_val);
								char grr[256];
								_snprintf_s(grr, 256, "buffering changed! val=%u\n", buff_state);
								OutputDebugString(grr);
								SetRuntimeBuffering(buff_state == 1);
							}
						}
					}
				}
				else if (result == 0)
				{
					DBG_OUT("Connection closed\n");
					break;
				}
				else
				{
					DBG_OUT("recv failed: %d\n", get_errno());
					break;
				}
			}
			return 0;
		}

		bool try_connect ();


		/**@brief	function consuming and sending items from MessagePool **/
		DWORD WINAPI consumer_thread ( LPVOID )
		{
			unsigned counter = 0;
			while (!g_Quit)
			{
				if (g_ReconnectTimer.enabled() && g_ReconnectTimer.expired())
				{
					if (try_connect())
						g_ReconnectTimer.reset();
					else
						g_ReconnectTimer.set_delay_ms(250);
				}
				sys::atomic32_t wr_idx = sys::atomic_get32(&m_wr_idx);
				sys::atomic32_t rd_idx = m_rd_idx;
				// @TODO: wraparound
				if (rd_idx < wr_idx)
				{

					//DBG_OUT("rd_idx=%10i, wr_idx=%10i, diff=%10i \n", rd_idx, wr_idx, wr_idx - rd_idx);
					msg_t & msg = socks::msg_buffer_at(rd_idx % message_pool_t::e_size);
					msg.ReadLock();

					bool const write_ok = socks::WriteToSocket(msg.m_data, msg.m_length);
					msg.m_length = 0;
					msg.ReadUnlockAndClean();
					sys::atomic_faa32(&m_rd_idx, 1);

					if (!write_ok)
					{
						g_ReconnectTimer.set_delay_ms(250);
						//break;
					}
					counter = 0;
				}

				sys::delay_execution(counter);
			}
			return 0;
		}

		void Flush ()
		{
			unsigned counter = 0;
			for (;;)
			{
				sys::atomic32_t const wr_idx = sys::atomic_get32(&m_wr_idx);
				sys::atomic32_t const rd_idx = sys::atomic_get32(&m_rd_idx);
				if (rd_idx < wr_idx)
					sys::delay_execution(counter);
				else
					break;
			}
		}
	}

	namespace socks {

		bool try_connect ()
		{
			g_ReconnectTimer.reset();
			g_ClottedTimer.reset();
			msg_t msg;
			// send cmd_setup message
			encode_setup(msg, GetRuntimeLevel(), GetRuntimeContextMask());

			socks::connect("localhost", "13127", g_Socket);

			if (socks::is_connected(socks::g_Socket))
			{
				socks::WriteToSocket(msg.m_data, msg.m_length);

				TRACE_SET_CTX_DICT();

				socks::g_ThreadRecv.Create(socks::receive_thread, 0);
				socks::g_ThreadRecv.Resume();
				return true;
			}
			return false;
		}
	}

	void Connect ()
	{
		sys::setTimeStart();

		socks::g_ThreadSend.Create(socks::consumer_thread, 0);

		bool const connected = socks::try_connect();
		if (!connected)
			socks::g_ReconnectTimer.set_delay_ms(1000);

#	if defined TRACE_WINDOWS_SOCKET_FAILOVER_TO_FILE
		char filename[128];
		create_log_filename(filename, sizeof(filename) / sizeof(*filename));
		socks::g_LogFile = CreateFileA(filename, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

		msg_t msg;
		// send cmd_setup message
		encode_setup(msg, GetRuntimeLevel(), GetRuntimeContextMask());
		socks::WriteToFile(msg.m_data, msg.m_length);
#	endif

		socks::g_ThreadSend.Resume();
	}

	void Disconnect ()
	{
		Flush();
		socks::g_Quit = 1; // @TODO: atomic_store?
		if (socks::is_connected(socks::g_Socket))
			sys::socks::disconnect(socks::g_Socket);
		socks::g_ThreadSend.WaitForTerminate();
		socks::g_ThreadSend.Close();
		socks::g_ThreadRecv.WaitForTerminate();
		socks::g_ThreadRecv.Close();

#	if defined TRACE_WINDOWS_SOCKET_FAILOVER_TO_FILE
		CloseHandle(socks::g_LogFile);
#	endif
	}

	void Flush () { socks::Flush(); }
}

#include "write_fns.inl"

