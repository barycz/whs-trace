#include "connection.h"
#include "logs/logwidget.h"
#include "mainwindow.h"
#include "constants.h"
#include "utils.h"

datalogs_t::iterator Connection::findOrCreateLog (QString const & tag)
{
	datalogs_t::iterator it = m_data.get<e_data_log>().find(tag);
	if (it == m_data.get<e_data_log>().end())
	{
		it = dataWidgetFactory<e_data_log>(tag);
		(*it)->setupLogModel();
		(*it)->applyConfig(); // 0 means "create new model"
	}
	return it;
}

// @FIXME: dodelat!!!!!!!!
bool Connection::handleLogCommand (DecodedCommand const & cmd, E_ReceiveMode mode)
{
	if (getClosestFeatureState(e_data_log) == e_FtrDisabled) return true;

	//QString const tag(g_MainLogName); // @FIXME
	QString const tag("messages"); // @FIXME
	//int const slash_pos = tag.lastIndexOf(QChar('/'));
	//tag.chop(msg_tag.size() - slash_pos);
	//QString subtag = msg_tag;
	//subtag.remove(0, slash_pos + 1);

	datalogs_t::iterator it = findOrCreateLog(tag);
	(*it)->handleCommand(cmd, mode);
	return true;
}

bool Connection::handleLogClearCommand (DecodedCommand const & cmd, E_ReceiveMode mode)
{
	if (getClosestFeatureState(e_data_log) == e_FtrDisabled) return true;
	return true;
}

