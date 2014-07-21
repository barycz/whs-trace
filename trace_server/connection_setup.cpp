#include "connection.h"
#include <QStandardItemModel>
#include <QListView>
#include <QHeaderView>
#include <tlv_parser/tlv_encoder.h>
#include "logs/logtablemodel.h"
#include "utils.h"
#include "utils_qstandarditem.h"
#include "utils_boost.h"
#include "constants.h"
#include "delegates.h"
#include "mainwindow.h"

bool Connection::dumpModeEnabled () const { return m_main_window->dumpModeEnabled(); }

namespace {
	struct RegisterCommand {
		MainWindow & m_main_window;
		QString const & m_app_name;
		RegisterCommand (MainWindow & mw, QString const & app_name) : m_main_window(mw), m_app_name(app_name) { }
		
		template <typename T>
		void operator() (T & t)
		{
			m_main_window.dockManager().removeActionAble(t);
			QStringList p = m_main_window.dockManager().path();
			p << m_app_name;
			p << g_fileTags[t.e_type];
			t.m_path = p;
			t.m_joined_path = p.join("/");
			m_main_window.dockManager().addActionAble(t, true);
		}
	};
}
void Connection::registerDataMaps ()
{
	recurse(m_data, RegisterCommand(*m_main_window, getAppName()));
}

bool Connection::handleSetupCommand (DecodedCommand const & cmd)
{
	qDebug("Connection::handleSetupCommand() this=0x%08x", this);

	QString pid;
	if (dumpModeEnabled())
	{
		for (size_t i=0, ie=cmd.m_tvs.size(); i < ie; ++i)
			if (cmd.m_tvs[i].m_tag == tlv::tag_pid)
				pid = cmd.m_tvs[i].m_val;
	}

	for (size_t i=0, ie=cmd.m_tvs.size(); i < ie; ++i)
	{
		if (cmd.m_tvs[i].m_tag == tlv::tag_lvl)
		{
			int const client_level = cmd.m_tvs[i].m_val.toInt();
			int const server_level = m_config.m_level;
			if (client_level != server_level)
			{
				qDebug("notifying client about new level");
				onLevelValueChanged(server_level);
			}
		}

		if (cmd.m_tvs[i].m_tag == tlv::tag_app)
		{
			QString const app_name = cmd.m_tvs[i].m_val;
			Connection * conn = m_main_window->findConnectionByName(app_name);
			if (conn)
			{
				qDebug("cmd setup: looking for app=%s: found", app_name.toStdString().c_str());
				qDebug("deleting old instance of %s at @ 0x%08x", conn->getAppName().toStdString().c_str(), conn);
				QString const curr_preset = conn->getCurrentPresetName();
				//m_main_window->markConnectionForClose(this);
				m_main_window->onCloseConnection(conn); // deletes it immeadiately

				// @TODO: delete persistent storage for the tab
				m_curr_preset = curr_preset;
			}
			else
			{
				qDebug("cmd setup: looking for app=%s: not found", app_name.toStdString().c_str());
				m_curr_preset = getClosestPresetName();
			}

			m_app_name = app_name;
			m_pid = pid;

			QString storage_name = createStorageName();
			setupStorage(storage_name);

			m_main_window->dockManager().removeActionAble(*this);
			m_path.last() = m_app_name;
			m_joined_path = m_path.join("/");
			m_main_window->dockManager().addActionAble(*this, true); // TODO: m_config.m_show

			loadConfig(m_curr_preset);
			onPresetApply(m_curr_preset);

			registerDataMaps();
			//m_main_window->onSetup(e_Proto_TLV, sessionState().m_app_idx, true, true);
		}
	}

	qDebug("Server::incomingConnection buffering not enabled, notifying client");
	onBufferingStateChanged(m_config.m_buffered);

	sendStateChangeCommand(tlv::cmd_set_logs_state, m_config.m_logs_recv_level);
	sendStateChangeCommand(tlv::cmd_set_plots_state, m_config.m_plots_recv_level);
	sendStateChangeCommand(tlv::cmd_set_tables_state, m_config.m_tables_recv_level);
	sendStateChangeCommand(tlv::cmd_set_gantts_state, m_config.m_gantts_recv_level);
	return true;
}

void Connection::handleCSVSetup (QString const & fname)
{
	qDebug("Connection::handleCSVSetup() this=0x%08x", this);

	QString const app_name = fname;

	// @TODO: dedup with tcp
	Connection * conn = m_main_window->findConnectionByName(app_name);
	if (conn)
	{
		qDebug("cmd setup: looking for app=%s: found", app_name.toStdString().c_str());
		qDebug("deleting old instance of %s at @ 0x%08x", conn->getAppName().toStdString().c_str(), conn);
		QString const curr_preset = conn->getCurrentPresetName();
		//m_main_window->markConnectionForClose(this);
		m_main_window->onCloseConnection(conn); // deletes it immeadiately

		// @TODO: delete persistent storage for the tab
		m_curr_preset = curr_preset;
	}
	else
	{
		qDebug("cmd setup: looking for app=%s: not found", app_name.toStdString().c_str());
		m_curr_preset = getClosestPresetName();
	}

	m_app_name = app_name;
	//m_pid = pid;

	m_main_window->dockManager().removeActionAble(*this);
	m_path.last() = m_app_name;
	m_joined_path = m_path.join("/");
	m_main_window->dockManager().addActionAble(*this, true); // TODO: m_config.m_show

	loadConfig(m_curr_preset);
	onPresetApply(m_curr_preset);

	registerDataMaps();

}

