#include "connection.h"
#include <QtNetwork>
#include <tlv_parser/tlv_encoder.h>
#include <trace_client/trace.h>
#include "utils.h"
#include "utils_qstandarditem.h"
#include "utils_boost.h"
#include "types.h"
#include "server.h"
#include "delegates.h"
#include "tableview.h"

GlobalConfig const & Connection::getGlobalConfig () const { return m_main_window->getConfig(); }

Connection::Connection (QObject * parent)
	: QThread(parent)
	, m_main_window(qobject_cast<MainWindow *>(parent))
	, m_src_stream(e_Stream_TCP)
	, m_src_protocol(e_Proto_TLV)
	, m_app_data()
	, m_app_idx(-1)
	, m_storage_idx(-2)
	, m_app_name()
	, m_recv_bytes(0)
	, m_marked_for_close(false)
	, m_curr_preset()
	, m_file_tlv_stream(0)
	, m_file_csv_stream(0)
	, m_buffer(e_ringbuff_size)
	, m_current_cmd()
	, m_decoded_cmds(e_ringcmd_size)
	, m_decoder()
	, m_storage(0)
	, m_tcp_dump_stream(0)
	, m_tcpstream(0)
{
	qDebug("Connection::Connection() this=0x%08x", this);

	static int counter = 0;
	m_storage_idx = counter;
	++counter;
}

namespace {

	template <class ContainerT>
	void unregisterDockedWidgets (ContainerT & c, MainWindow & mainwin)
	{
		for (typename ContainerT::iterator it = c.begin(), ite = c.end(); it != ite; ++it)
		{
			typename ContainerT::iterator::value_type ptr = *it;
			mainwin.dockManager().removeDockable(ptr->path().join("/"));
			mainwin.dockManager().removeActionAble(ptr->path().join("/"));
		}
		//c.clear(); toto je spatne!
	}
}

struct UnregisterDockedWidgets {
	MainWindow & m_main_window;

	UnregisterDockedWidgets (MainWindow & mw) : m_main_window(mw) { }

	template <typename T>
	void operator() (T & t)
	{
		unregisterDockedWidgets(t, m_main_window);
	}
};

void Connection::destroyDockedWidget (DockedWidgetBase * dwb)
{
	switch (dwb->type())
	{
		case e_data_log:
			m_main_window->dockManager().removeDockable(dwb->path().join("/"));
			m_main_window->dockManager().removeActionAble(dwb->path().join("/"));
			// @TODO!
			//removeDockWidget<e_data_log>(static_cast<DataLog *>(dwb));
			//destroyDockedWidget<e_data_log>(static_cast<DataLog *>(dwb));
			break;
		case e_data_plot:
		case e_data_table:
		case e_data_gantt:
		case e_data_frame:
		default: break;
	}
}

namespace {

	template <class ContainerT>
	void destroyDockedWidgets (ContainerT & c, MainWindow & mw, Connection & conn)
	{
		for (typename ContainerT::iterator it = c.begin(), ite = c.end(); it != ite; ++it)
		{
			typename ContainerT::iterator::value_type ptr = *it;
			delete ptr;
		}
		c.clear();
	}
}

struct DestroyDockedWidgets {
	MainWindow & m_main_window;
	Connection & m_conn;

	DestroyDockedWidgets (MainWindow & mw, Connection & conn) : m_main_window(mw), m_conn(conn) { }

	template <typename T>
	void operator() (T & t)
	{
		destroyDockedWidgets(t, m_main_window, m_conn);
	}
};

Connection::~Connection ()
{
	qDebug("Connection::~Connection() this=0x%08x", this);

	recurse(m_data, UnregisterDockedWidgets(*m_main_window));

	if (m_tcpstream)
	{
		QObject::disconnect(m_tcpstream, SIGNAL(readyRead()), this, SLOT(processReadyRead()));
		QObject::disconnect(m_tcpstream, SIGNAL(disconnected()), this, SLOT(onDisconnected()));
	}
	if (m_tcpstream)
		m_tcpstream->close();
	closeStorage();

	qDebug("destroying docked widgets");
	recurse(m_data, DestroyDockedWidgets(*m_main_window, *this));

	/*if (m_file_tlv_stream)
	{
		QDevice * const f = m_file_tlv_stream->device();
		f->close();
		delete m_file_tlv_stream;
		m_file_tlv_stream = 0;
		delete f;
	}*/

/*	if (m_file_csv_stream)
	{
		QIODevice * const f = m_file_csv_stream->device();
		f->close();
		delete m_file_csv_stream;
		m_file_csv_stream = 0;
		delete f;
	}*/
}

void Connection::mkWidgetPath (E_DataWidgetType type, QString const tag, QStringList & path)
{
	char const * widget_prefix = g_fileTags[type];
	QString const & name0 = m_main_window->dockedName();
	QString const & name1 = getAppName();
	QString const & name2 = widget_prefix;
	QString const & name3 = tag;
	path.append(name0);
	path.append(name1);
	path.append(name2);
	path.append(name3);
}

void Connection::onDisconnected ()
{
	qDebug("onDisconnected()");
	//if (m_statswindow) m_statswindow->stopUpdate();

	if (m_main_window->dumpModeEnabled())
	{
		QString fname = tr("%1_%2.csv").arg(getAppName()).arg(m_pid);

		exportStorageToCSV(fname);

		Server * server = static_cast<Server *>(parent());
		m_marked_for_close = true;
		QTimer::singleShot(0, server, SLOT(onCloseMarkedTabs()));
	}

	for (dataplots_t::iterator it = m_data.get<e_data_plot>().begin(), ite = m_data.get<e_data_plot>().end(); it != ite; ++it)
		(*it)->stopUpdate();
}

void Connection::onLevelValueChanged (int val)
{
	char tlv_buff[16];
#ifdef __linux__
	int const result = snprintf(tlv_buff, 16, "%u", val);
#else
	int const result = _snprintf_s(tlv_buff, 16, "%u", val);
#endif

	if (result > 0)
	{
		char buff[256];
		using namespace tlv;
		Encoder e(cmd_set_level, buff, 256);
		e.Encode(TLV(tag_lvl, tlv_buff));
		if (m_tcpstream && e.Commit())
			m_tcpstream->write(e.buffer, e.total_len); /// @TODO: async write
	}
}

void Connection::onBufferingStateChanged (int val)
{
	bool const buffering_enabled = (val == Qt::Checked) ? true : false;

	char tlv_buff[16];
#ifdef __linux__
	int const result = snprintf(tlv_buff, 16, "%u", buffering_enabled);
#else
	int const result = _snprintf_s(tlv_buff, 16, "%u", buffering_enabled);
#endif

	if (result > 0)
	{
		qDebug("Connection::onBufferingStateChanged to_state=%i", buffering_enabled);
		char buff[256];
		using namespace tlv;
		Encoder e(cmd_set_buffering, buff, 256);
		e.Encode(TLV(tag_bool, tlv_buff));
		if (m_tcpstream && e.Commit())
			m_tcpstream->write(e.buffer, e.total_len); /// @TODO: async write
	}
}

struct Save {

	QString const & m_path;
	Save (QString const & p) : m_path(p) { }

	template <class T>
	void operator() (T const & t)
	{
		typedef typename T::const_iterator it_t;
		for (it_t it = t.begin(), ite = t.end(); it != ite; ++it)
			(*it)->widget().saveConfig(m_path);
	}
};

struct Load {

	QString const & m_path;
	Load (QString const & p) : m_path(p) { }

	template <class T>
	void operator() (T const & t)
	{
		typedef typename T::const_iterator it_t;
		for (it_t it = t.begin(), ite = t.end(); it != ite; ++it)
			(*it)->widget().loadConfig(m_path);
	}
};

struct Apply {

	template <class T>
	void operator() (T const & t)
	{
		typedef typename T::const_iterator it_t;
		for (it_t it = t.begin(), ite = t.end(); it != ite; ++it)
			(*it)->widget().applyConfig();
	}
};

struct ExportAsCSV {

	QString const & m_path;
	ExportAsCSV (QString const & dir) : m_path(dir) { }

	template <class T>
	void operator() (T const & t)
	{
		typedef typename T::const_iterator it_t;
		for (it_t it = t.begin(), ite = t.end(); it != ite; ++it)
			(*it)->widget().exportStorageToCSV(m_path);
	}
};

/*void Connection::onSaveAll ()
{
	qDebug("%s", __FUNCTION__);
	// @TODO: v hhdr bude 0 !
	
	QString const preset_name = m_curr_preset.isEmpty() ? m_main_window->getValidCurrentPresetName() : m_curr_preset;
	//QString const fname = getDataTagFileName(getConfig().m_appdir, preset_name, g_presetTableTag, tag);
	QString const preset_path = getPresetPath(getConfig().m_appdir, preset_name);
	saveConfigs(preset_path);
}*/


void Connection::saveConfigs (QString const & path)
{
	qDebug("%s", __FUNCTION__);
	recurse(m_data, Save(path));
}

void Connection::loadConfigs (QString const & path)
{
	qDebug("%s", __FUNCTION__);
	recurse(m_data, Load(path));
}

void Connection::applyConfigs ()
{
	qDebug("%s", __FUNCTION__);
	recurse(m_data, Apply());
}

void Connection::exportStorageToCSV (QString const & dir)
{
	qDebug("%s", __FUNCTION__);
	recurse(m_data, ExportAsCSV(dir));
}

