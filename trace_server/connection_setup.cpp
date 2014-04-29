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
				delete conn;
				// @TODO: delete persistent storage for the tab
				m_curr_preset = curr_preset;
			}
			else
			{
				qDebug("cmd setup: looking for app=%s: not found", app_name.toStdString().c_str());
				m_curr_preset = getCurrentPresetName();
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
	return true;
}

void Connection::handleCSVSetup (QString const & fname)
{
/*	qDebug("Connection::handleCSVSetup() this=0x%08x", this);

	this->setupModelColorRegex();
	this->setupModelRegex();
	this->setupModelString();

	QString app_name = fname;
	if (m_main_window->reuseTabEnabled())
	{
		Connection * conn = m_main_window->findConnectionByName(app_name);
		if (conn)
		{
			if (!m_main_window->clrFltEnabled())
			{
				loadSessionState(conn->sessionState(), m_session_state);
			}

			QWidget * w = conn->m_tab_widget;
			m_main_window->onCloseTab(w);	// close old one
			// @TODO: delete persistent storage for the tab
		}
		else
		{
			QString const pname = m_main_window->matchClosestPresetName(app_name);
			m_main_window->onPresetActivate(this, pname);
		}
	}

	m_app_name = app_name;
	//sessionState().m_pid = pid;

	m_table_view_widget->setVisible(false);
	int const tab_idx = m_main_window->getTabTrace()->indexOf(m_tab_widget);
	m_main_window->getTabTrace()->setTabText(tab_idx, app_name);

	m_app_idx = m_main_window->findAppName(app_name);
	if (m_app_idx == e_InvalidItem)
	{
		qDebug("Unknown application: requesting user input");
		m_app_idx = m_main_window->createAppName(app_name, e_Proto_CSV);
	}

	columns_setup_t & cs_setup = m_main_window->getColumnSetup(sessionState().m_app_idx);
	columns_sizes_t & cs_sizes = m_main_window->getColumnSizes(sessionState().m_app_idx);
	columns_align_t & cs_align = m_main_window->getColumnAlign(sessionState().m_app_idx);
	columns_elide_t & cs_elide = m_main_window->getColumnElide(sessionState().m_app_idx);

	if (cs_setup.empty() || cs_sizes.empty() || cs_align.empty() || cs_elide.empty())
	{
		//m_main_window->onSetup(sessionState().m_app_idx, true, true);
	}

	sessionState().setupColumnsCSV(&cs_setup, &cs_sizes, &cs_align, &cs_elide); 

	m_current_cmd.tvs.reserve(sessionState().getColumnsSetupCurrent()->size());
	for (size_t i = 0, ie = sessionState().getColumnsSetupCurrent()->size(); i < ie; ++i)
	{
		m_table_view_widget->model()->insertColumn(i);
	}

	m_table_view_widget->horizontalHeader()->resizeSections(QHeaderView::Fixed);

	columns_sizes_t const & sizes = *sessionState().m_columns_sizes;
	for (int c = 0, ce = sizes.size(); c < ce; ++c)
	{
		m_table_view_widget->horizontalHeader()->resizeSection(c, sizes.at(c));
		//qDebug("sizes: %u %u %u", sizes.at(0), sizes.at(1), sizes.at(2));
	}

	m_table_view_widget->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
	m_table_view_widget->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);

	//connect(m_table_view_widget, SIGNAL(clicked(QModelIndex const &)), this, SLOT(onTableClicked(QModelIndex const &)));
	//connect(m_table_view_widget, SIGNAL(doubleClicked(QModelIndex const &)), this, SLOT(onTableDoubleClicked(QModelIndex const &)));
	//m_table_view_widget->setContextMenuPolicy(Qt::CustomContextMenu);
	//connect(m_table_view_widget, SIGNAL(customContextMenuRequested(QPoint const &)), this, SLOT(onShowContextMenu(QPoint const &)));

	m_table_view_widget->setVisible(true);
	//m_table_view_widget->setItemDelegate(new TableItemDelegate(sessionState(), this));

	m_main_window->getTabTrace()->setCurrentIndex(tab_idx);
	static_cast<LogTableModel *>(m_table_view_widget->model())->emitLayoutChanged();

	qDebug("Server::incomingConnection buffering not enabled, notifying client");
	onBufferingStateChanged(m_main_window->buffState());*/
}

/*void Connection::tryLoadMatchingPreset (QString const & app_name)
{
	// bit clumsy, but no time to loose
	QString preset_name;
	QString const conn_name = m_control_bar->ui->presetComboBox->currentText();
	QString const parent_name = m_main_window->getCurrentPresetName();
	if (!conn_name.isEmpty() && validatePresetName(conn_name))
		preset_name = conn_name;
	else if (!parent_name.isEmpty() && validatePresetName(parent_name))
		preset_name = parent_name;
	else
		preset_name = g_defaultPresetName;

	onPresetApply(preset_name);
}*/



