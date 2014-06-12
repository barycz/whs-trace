#include "connection.h"
#include <QtNetwork>
#include <QMessageBox>
#include <QHeaderView>
#include <tlv_parser/tlv.h>
#include <tlv_parser/tlv_cmd_qstring.h>
#include <tlv_parser/tlv_encoder.h>
#include "logs/logtablemodel.h"
#include "cmd.h"
#include "utils.h"
#include "utils_boost.h"
#include "dock.h"
#include "mainwindow.h"
#include <cstdlib>

inline void Dump (DecodedCommand const & c)
{
	qDebug("command.hdr.cmd = 0x%x command.hdr.len = 0x%x", c.m_hdr.cmd, c.m_hdr.len);
	for (size_t i = 0; i < c.m_tvs.size(); ++i)
		qDebug("tlv[%u] t=%02x val=%s", i, c.m_tvs[i].m_tag, c.m_tvs[i].m_val.toStdString().c_str());
}

inline size_t read_min (boost::circular_buffer<char> & ring, char * dst, size_t min)
{
	if (ring.size() < min)
		return 0;

	for (size_t i = 0; i < min; ++i)
	{
		dst[i] = ring.front();
		ring.pop_front();
	}
	return min;
}

bool isCommandQueuable (tlv::tag_t cmd)
{
	switch (cmd)
	{
		case tlv::cmd_setup:			return false;
		case tlv::cmd_ping:				return false;
		case tlv::cmd_shutdown:			return false;
		case tlv::cmd_dict_ctx:			return false;
		case tlv::cmd_save_tlv:			return false;
		case tlv::cmd_export_csv:		return false;
		case tlv::cmd_plot_clear:		return false;
		case tlv::cmd_table_setup:		return false;
		case tlv::cmd_table_clear:		return false;
		case tlv::cmd_log_clear:		return false;
		case tlv::cmd_log:
		case tlv::cmd_scope_entry:
		case tlv::cmd_scope_exit:
		case tlv::cmd_plot_xy:
		case tlv::cmd_plot_xyz:	
		case tlv::cmd_table_xy:	
		case tlv::cmd_gantt_bgn:
		case tlv::cmd_gantt_end:
		case tlv::cmd_gantt_frame_bgn:
		case tlv::cmd_gantt_frame_end:
		case tlv::cmd_gantt_clear:		return true;

		default:
			qWarning("unknown command!\n"); 
			return true;
	}
}

E_DataWidgetType queueForCommand (tlv::tag_t cmd)
{
	switch (cmd)
	{
		case tlv::cmd_save_tlv:			return e_data_log;
		case tlv::cmd_export_csv:		return e_data_log; 
		case tlv::cmd_log:				return e_data_log;
		case tlv::cmd_log_clear:		return e_data_log;
		case tlv::cmd_scope_entry:		return e_data_log;
		case tlv::cmd_scope_exit:		return e_data_log;
		case tlv::cmd_plot_xy:			return e_data_plot;
		case tlv::cmd_plot_xyz:			return e_data_plot;
		case tlv::cmd_plot_clear:		return e_data_plot;
		case tlv::cmd_table_xy:			return e_data_table;
		case tlv::cmd_table_setup:		return e_data_table;
		case tlv::cmd_table_clear:		return e_data_table;
		case tlv::cmd_gantt_bgn:		return e_data_gantt;
		case tlv::cmd_gantt_end:		return e_data_gantt;
		case tlv::cmd_gantt_frame_bgn:	return e_data_gantt;
		case tlv::cmd_gantt_frame_end:	return e_data_gantt;
		case tlv::cmd_gantt_clear:		return e_data_gantt;

		default: qWarning("unknown command!\n"); return e_data_widget_max_value;
	}
}

namespace {
	struct EnqueueCommand {
		E_DataWidgetType m_type;
		DecodedCommand const & m_cmd;

		EnqueueCommand (E_DataWidgetType t, DecodedCommand const & cmd) : m_type(t), m_cmd(cmd) { }
		
		template <typename T>
		void operator() (T & t)
		{
			if (t.e_type == m_type)
				t.queue().push_back(m_cmd);
		}
	};
}

bool Connection::enqueueCommand (DecodedCommand const & cmd)
{
	E_DataWidgetType const queue_type = queueForCommand(cmd.m_hdr.cmd);
	if (queue_type == e_data_widget_max_value)
		return false;

	recurse(m_data, EnqueueCommand(queue_type, cmd));
	return true;
}


bool Connection::tryHandleCommand (DecodedCommand const & cmd, E_ReceiveMode mode)
{
	switch (cmd.m_hdr.cmd)
	{
		case tlv::cmd_setup:			handleSetupCommand(cmd); break;
		case tlv::cmd_shutdown:			handleShutdownCommand(cmd); break;
		case tlv::cmd_ping:				handlePingCommand(cmd); break;
		case tlv::cmd_save_tlv:			handleSaveTLVCommand(cmd); break;
		case tlv::cmd_export_csv:		handleExportCSVCommand(cmd); break;
		case tlv::cmd_dict_ctx:			handleDictionnaryCtx(cmd); break;

		case tlv::cmd_log:				handleLogCommand(cmd, mode); break;
		case tlv::cmd_log_clear:		handleLogClearCommand(cmd, mode); break;
		case tlv::cmd_scope_entry:		handleLogCommand(cmd, mode); break;
		case tlv::cmd_scope_exit:		handleLogCommand(cmd, mode); break;
		case tlv::cmd_plot_xy:			handlePlotCommand(cmd, mode); break;
		case tlv::cmd_plot_xyz:			handlePlotCommand(cmd, mode); break;
		case tlv::cmd_plot_clear:		handlePlotCommand(cmd, mode); break;
		case tlv::cmd_table_xy:			handleTableCommand(cmd, mode); break;
		case tlv::cmd_table_setup:		handleTableCommand(cmd, mode); break;
		case tlv::cmd_table_clear:		handleTableCommand(cmd, mode); break;
		case tlv::cmd_gantt_bgn:		handleGanttBgnCommand(cmd, mode); break;
		case tlv::cmd_gantt_end:		handleGanttEndCommand(cmd, mode); break;
		case tlv::cmd_gantt_frame_bgn:	handleGanttFrameBgnCommand(cmd, mode); break;
		case tlv::cmd_gantt_frame_end:	handleGanttFrameEndCommand(cmd, mode); break;
		case tlv::cmd_gantt_clear:		handleGanttClearCommand(cmd, mode); break;

		default: qDebug("unknown command, ignoring\n"); break;
	}
	return true;
}

void Connection::onHandleCommands ()
{
	size_t const rows = m_decoded_cmds.size();
	for (size_t i = 0; i < rows; ++i)
	{
		DecodedCommand & cmd = m_decoded_cmds.front();
		if (isCommandQueuable(cmd.m_hdr.cmd))
		{
			enqueueCommand(cmd);
		}
		else
		{
			tryHandleCommand(cmd, e_RecvSync);
		}

		if (m_src_stream == e_Stream_TCP && m_tcp_dump_stream)
			m_tcp_dump_stream->writeRawData(&cmd.m_orig_message[0], cmd.m_hdr.len + tlv::Header::e_Size);

		m_decoded_cmds.pop_front();
	}
}

void Connection::onHandleCommandsStart ()
{
}

namespace {
	struct DequeueCommand {
		Connection & m_conn;

		DequeueCommand ( Connection & c) : m_conn(c) { }
		
		template <typename T>
		void operator() (T & t)
		{
			while (!t.queue().isEmpty())
			{
				DecodedCommand const & cmd = t.queue().front();
				m_conn.tryHandleCommand(cmd, e_RecvBatched);
				t.queue().pop_front();
			}
			for (typename T::iterator it = t.begin(); it != t.end(); ++it)
				(*it)->commitCommands(e_RecvBatched);
		}
	};
}

void Connection::onHandleCommandsCommit ()
{
	recurse(m_data, DequeueCommand(*this));
}


template <class T, typename T_Ret, typename T_Arg0, typename T_Arg1>
int Connection::processStream (T * t, T_Ret (T::*read_member_fn)(T_Arg0, T_Arg1))
{
	enum { local_buff_sz = 128 };
	char local_buff[local_buff_sz];
	try {

		emit onHandleCommandsStart();
		bool data_in_stream = true;
		while (data_in_stream)
		{
			// read data into ring buffer
			while (!m_buffer.full())
			{
				size_t const free_space = m_buffer.reserve();
				size_t const to_read = free_space < local_buff_sz ? free_space : local_buff_sz;

				qint64 const count = (t->*read_member_fn)(local_buff, static_cast<int>(to_read));
				m_recv_bytes += count;			

				if (count <= 0)
				{
					data_in_stream = false;
					break;	// no more data in stream
				}

				for (int i = 0; i < count; ++i)
					m_buffer.push_back(local_buff[i]);
			}

			// try process data in ring buffer
			while (!m_decoded_cmds.full())
			{
				if (!m_current_cmd.m_written_hdr)
				{
					size_t const count_hdr = read_min(m_buffer, &m_current_cmd.m_orig_message[0], tlv::Header::e_Size);
					if (count_hdr == tlv::Header::e_Size)
					{
						tlv::HeaderDecoder hdr_decoder;
						hdr_decoder.decode_header(&m_current_cmd.m_orig_message[0], tlv::Header::e_Size, m_current_cmd.m_hdr);
						if (m_current_cmd.m_hdr.cmd == 0 || m_current_cmd.m_hdr.len == 0)
						{
							Q_ASSERT(false);
							//@TODO: parsing error, discard everything and re-request stop sequence
							break;
						}
						m_current_cmd.m_written_hdr = true;
					}
					else
						break; // not enough data
				}

				if (m_current_cmd.m_written_hdr && !m_current_cmd.m_written_payload)
				{
					size_t const count_payload = read_min(m_buffer, &m_current_cmd.m_orig_message[0] + tlv::Header::e_Size, m_current_cmd.m_hdr.len);
					if (count_payload == m_current_cmd.m_hdr.len)
						m_current_cmd.m_written_payload = true;
					else
						break; // not enough data
				}

				if (m_current_cmd.m_written_hdr && m_current_cmd.m_written_payload)
				{
					if (m_current_cmd.m_hdr.version == 1)
					{
						tlv::TVDecoder_v1 decoder;
						if (decoder.decode_payload(&m_current_cmd.m_orig_message[0] + tlv::Header::e_Size, m_current_cmd.m_hdr.len, m_current_cmd))
						{
							//qDebug("CONN: hdr_sz=%u payload_sz=%u buff_sz=%u ",  tlv::Header::e_Size, m_current_cmd.hdr.len, m_buffer.size());
							m_decoded_cmds.push_back(m_current_cmd);
							m_decoded_cmds.back().decode_postprocess();
						}
						else
						{
							Q_ASSERT(false);
							//@TODO: parsing error, discard everything and re-request stop sequence
							break;
						}

						m_current_cmd.reset(); // reset current command for another decoding pass
					}
				}
			}

			if (m_decoded_cmds.size() > 0)
				emit handleCommands();
		}
		
		emit onHandleCommandsCommit();
		return e_data_ok;
	}
	catch (std::out_of_range const & e)
	{
		QMessageBox::critical(0, tr("trace server"),
								tr("OOR exception during decoding: %1").arg(e.what()),
								QMessageBox::Ok, QMessageBox::Ok);	
		return e_data_decode_oor;
	}
	catch (std::length_error const & e)
	{
		QMessageBox::critical(0, tr("trace server"),
								tr("LE exception during decoding: %1").arg(e.what()),
								QMessageBox::Ok, QMessageBox::Ok);
		return e_data_decode_lnerr;
	}
	catch (std::exception const & e)
	{
		QMessageBox::critical(0, tr("trace server"),
								tr("generic exception during decoding: %1").arg(e.what()),
								QMessageBox::Ok, QMessageBox::Ok);
		return e_data_decode_captain_failure;
	}
	catch (...)
	{
		QMessageBox::critical(0, tr("trace server"),
								tr("... exception during decoding"),
								QMessageBox::Ok, QMessageBox::Ok);
		return e_data_decode_general_failure;
	}
	return e_data_decode_error;
}

void Connection::processReadyRead ()
{
	processStream(static_cast<QIODevice *>(m_tcpstream), &QTcpSocket::read);
}

void Connection::setSocketDescriptor (int sd)
{
	m_src_stream = e_Stream_TCP;
	m_src_protocol = e_Proto_TLV;
	m_src_name = QString("tcp ").arg(sd);

	m_tcpstream = new QTcpSocket(this);
	m_tcpstream->setSocketDescriptor(sd);
	connect(this, SIGNAL(handleCommands()), this, SLOT(onHandleCommands()));
}

void Connection::setImportFile (QString const & fname)
{
	m_src_stream = e_Stream_File;
	m_src_protocol = e_Proto_TLV;
	m_src_name = fname;
}

void Connection::setTailFile (QString const & fname)
{
	m_src_stream = e_Stream_File;
	m_src_protocol = e_Proto_CSV;
	m_src_name = fname;

	QFile * f = new QFile(fname);
	if (!f->open(QIODevice::ReadOnly))
	{
		QMessageBox::critical(0, tr("Error"), tr("Could not open file"));
		delete f;
		return;
	}

	m_file_csv_stream = new QTextStream(f);
	connect(this, SIGNAL(handleCommands()), this, SLOT(onHandleCommands()));
}

void Connection::run ()
{
	while (1)
	{
		const int Timeout = 5 * 1000;
		while (m_tcpstream->bytesAvailable() < tlv::Header::e_Size)
			m_tcpstream->waitForReadyRead(Timeout);

		processReadyRead();
	}
}

void Connection::processDataStream (QDataStream & stream)
{
	connect(this, SIGNAL(handleCommands()), this, SLOT(onHandleCommands()));

	int const res = processStream(&stream, &QDataStream::readRawData);
	switch (res)
	{
		case e_data_ok:
		{
			// update column sizes
			/*columns_sizes_t const & sizes = *sessionState().m_columns_sizes;
			bool const old = m_table_view_widget->blockSignals(true);
			for (int c = 0, ce = sizes.size(); c < ce; ++c)
				m_table_view_widget->horizontalHeader()->resizeSection(c, sizes.at(c));
			m_table_view_widget->blockSignals(old);*/
			return;
		}
		case e_data_decode_oor:
		case e_data_decode_lnerr:
		case e_data_decode_captain_failure:
		case e_data_decode_general_failure:
		case e_data_decode_error:
			qDebug("!!! exception duging decoding!");
	}
}

#ifdef WIN32
#	include <io.h>
	inline qint64 getFileSize (QFile * f)
	{
		qint64 sz = -1;
		if (f)
		{
			LARGE_INTEGER arg;
			HANDLE h = (HANDLE)_get_osfhandle(f->handle());
			if (::GetFileSizeEx(h, &arg) != 0)
				sz = arg.QuadPart;
		}
		return sz;
	}
#endif

void Connection::processTailCSVStream ()
{
	qint64 const sz = getFileSize(qobject_cast<QFile *>(m_file_csv_stream->device()));
	if (sz > m_file_size)
		m_file_size = sz;
	
	if (sz < m_file_size)
	{
		m_file_size = 0;
		m_main_window->requestReloadFile(m_src_name);
		QTimer::singleShot(32, m_main_window, SLOT(onReloadFile()));
		m_main_window->onCloseConnection(this); // deletes it immeadiately
		return;
	}
	while (!m_file_csv_stream->atEnd())
	{
		while (!m_decoded_cmds.full() && !m_file_csv_stream->atEnd())
		{
			QString const data = m_file_csv_stream->readLine(2048);
			QByteArray ba = data.toLatin1();
			memcpy(&m_current_cmd.m_orig_message[0], ba.data(), ba.size() < 2048? ba.size() : 2048 - 1);

			m_decoded_cmds.push_back(m_current_cmd);
			m_current_cmd.reset(); // reset current command for another decoding pass
		}

		if (m_decoded_cmds.size() > 0)
		{
			emit onHandleCommandsStart();

			for (size_t i = 0, ie = m_decoded_cmds.size(); i < ie; ++i)
			{
				DecodedCommand & cmd = m_decoded_cmds.front();
				handleCSVStreamCommand(cmd);
				m_decoded_cmds.pop_front();
			}

			emit onHandleCommandsCommit();
		}
	}

	QTimer::singleShot(250, this, SLOT(processTailCSVStream()));
}

bool Connection::handlePingCommand (DecodedCommand const & cmd)
{
	qDebug("ping from client!");
	disconnect(m_tcpstream, SIGNAL(disconnected()), this, SLOT(onDisconnected()));
	m_marked_for_close = true;
	QTimer::singleShot(0, m_main_window, SLOT(onCloseMarkedConnections()));
	return true;
}

bool Connection::handleShutdownCommand (DecodedCommand const & cmd)
{
	qDebug("shutdown from client requested (update?)");
	QTimer::singleShot(0, m_main_window, SLOT(onQuit()));	
	return true;
}

bool Connection::handleDictionnaryCtx (DecodedCommand const & cmd)
{
	qDebug("received custom context dictionnary");
	QList<QString> name;
	QList<QString> value;
	for (size_t i=0, ie=cmd.m_tvs.size(); i < ie; i+=2)
	{
		if (cmd.m_tvs[i].m_tag == tlv::tag_string)
		{
			name.append(cmd.m_tvs[i].m_val);
		}

		if (cmd.m_tvs[i+1].m_tag == tlv::tag_int)
		{
			value.append(cmd.m_tvs[i+1].m_val);
		}
	}
	m_app_data.addCtxDict(name, value);
	return true;
}


//////////////////// storage stuff //////////////////////////////
QString Connection::createStorageName () const
{
	return m_app_name + QString::number(m_storage_idx);
}

bool Connection::setupStorage (QString const & name)
{
	if (m_src_stream == e_Stream_TCP && !m_storage)
	{
		m_storage = new QFile(name + "." + g_traceFileExtTLV);
		m_storage->open(QIODevice::WriteOnly);
		m_tcp_dump_stream = new QDataStream(m_storage);

		if (!m_tcp_dump_stream)
		{
			return false;
		}
	}
	return true;
}

void Connection::copyStorageTo (QString const & filename)
{
	if (m_storage)
	{
		m_storage->flush();
		m_storage->copy(filename);
	}
	else
	{
		QString name = createStorageName();
		QFile trc(name + "." + g_traceFileExtTLV);
		trc.open(QIODevice::ReadOnly);
		trc.copy(filename);
		trc.close();
	}
}

bool Connection::handleSaveTLVCommand (DecodedCommand const & cmd)
{
	for (size_t i=0, ie=cmd.m_tvs.size(); i < ie; ++i)
	{
		if (cmd.m_tvs[i].m_tag == tlv::tag_file)
		{
			copyStorageTo(cmd.m_tvs[i].m_val);
			return true;
		}
	}
	return true;
}

bool Connection::handleExportCSVCommand (DecodedCommand const & cmd)
{
	for (size_t i=0, ie=cmd.m_tvs.size(); i < ie; ++i)
	{
		if (cmd.m_tvs[i].m_tag == tlv::tag_file) // ehm, actually it's a dir
		{
			onExportDataToCSV(cmd.m_tvs[i].m_val);
			return true;
		}
	}
	return false;
}

void Connection::closeStorage ()
{
	if (m_storage)
	{
		delete m_tcp_dump_stream;
		m_storage->close();
		m_storage->remove();
		delete m_storage;
		m_storage = 0;
	}
}


