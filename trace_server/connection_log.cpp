#include "connection.h"
#include <QClipboard>
#include <QObject>
#include "logs/logtablemodel.h"
#include "logs/logwidget.h"
#include "tableview.h"
#include "constants.h"
#include "utils.h"

DataLog::DataLog (Connection * connection, QString const & confname, QStringList const & path)
	: DockedData<e_data_log>(connection, confname, path)
{
	qDebug("%s this=0x%08x name=%s", __FUNCTION__, this, confname.toStdString().c_str());

	QWidget * tab = connection->m_tab_widget;
	QHBoxLayout * horizontalLayout = new QHBoxLayout(tab);
	horizontalLayout->setSpacing(1);
	horizontalLayout->setContentsMargins(0, 0, 0, 0);
	horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
	logs::LogWidget * tableView = new logs::LogWidget(connection, tab, this->config(), confname, path);
	horizontalLayout->addWidget(tableView);
	m_widget = tableView;
}

DataLog::DataLog (Connection * connection, QString const & confname, QStringList const & path, logs::LogConfig const & config)
	: DockedData<e_data_log>(connection, confname, path, config)
{
	qDebug("%s this=0x%08x with cfg name=%s", __FUNCTION__, this, confname.toStdString().c_str());

	QWidget * tab = connection->m_tab_widget;
	QHBoxLayout * horizontalLayout = new QHBoxLayout(tab);
	horizontalLayout->setSpacing(1);
	horizontalLayout->setContentsMargins(0, 0, 0, 0);
	horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
	logs::LogWidget * tableView = new logs::LogWidget(connection, tab, this->config(), confname, path);
	horizontalLayout->addWidget(tableView);
	m_widget = tableView;
}

DataLog::~DataLog ()
{
	//QObject::disconnect(connection->m_table_view_widget->horizontalHeader(), SIGNAL(sectionResized(int, int, int)), this, SLOT(onSectionResized(int, int, int)));
}

// @FIXME: dodelat!!!!!!!!
void Connection::onTabTraceFocus ()
{
	if (!m_curr_preset.isEmpty())
		m_main_window->setPresetAsCurrent(m_curr_preset);

	// @TODO: set app level to spinbox
	// @TODO: set app buffering to spinbox

	//m_main_window->setLastSearchIntoCombobox(m_last_search);
}

bool Connection::handleLogCommand (DecodedCommand const & cmd, E_ReceiveMode mode)
{
	QString const tag(g_MainLogName); // @FIXME
	//int const slash_pos = tag.lastIndexOf(QChar('/'));
	//tag.chop(msg_tag.size() - slash_pos);
	//QString subtag = msg_tag;
	//subtag.remove(0, slash_pos + 1);

	datalogs_t::iterator it = findOrCreateLog(tag);
	DataLog & dp = **it;

	dp.widget().handleCommand(cmd, mode);
	return true;
}

bool Connection::handleLogClearCommand (DecodedCommand const & cmd, E_ReceiveMode mode)
{
	return true;
}

void Connection::onShowLogs ()
{
	qDebug("%s", __FUNCTION__);
	for (datalogs_t::iterator it = m_data.get<e_data_log>().begin(), ite = m_data.get<e_data_log>().end(); it != ite; ++it)
	{
		//(*it)->onShow();
		m_main_window->restoreDockWidget((*it)->m_wd);
	}
}

void Connection::onHideLogs ()
{
	qDebug("%s", __FUNCTION__);
	/*for (datalogs_t::iterator it = m_data.get<e_data_log>().begin(), ite = m_data.get<e_data_log>().end(); it != ite; ++it)
	{
		(*it)->onHide();
	}*/
}

void Connection::onShowLogContextMenu (QPoint const &)
{
	qDebug("%s", __FUNCTION__);
	for (datalogs_t::iterator it = m_data.get<e_data_log>().begin(), ite = m_data.get<e_data_log>().end(); it != ite; ++it)
	{
		(*it)->widget().onHideContextMenu();
	}
}

datalogs_t::iterator Connection::findOrCreateLog (QString const & tag)
{
	typedef SelectIterator<e_data_log>::type iterator;
	iterator it = m_data.get<e_data_log>().find(tag);
	if (it == m_data.get<e_data_log>().end())
	{
		it = dataWidgetFactory<e_data_log>(tag);
		(*it)->widget().setupNewLogModel();
		(*it)->widget().applyConfig(); // 0 means "create new model"
	}
	return it;
}


