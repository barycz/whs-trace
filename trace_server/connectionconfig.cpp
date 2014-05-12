#include "connectionconfig.h"
#include "serialize.h"

void ConnectionConfig::loadHistory (QString const & path)
{
	QString const pfilename = path + "/app_preset_history.xml";
	::loadConfigTemplate(m_preset_history, pfilename.toLatin1());
}

void ConnectionConfig::saveHistory (QString const & path) const
{
	QString const pfilename = path + "/app_preset_history.xml";
	::saveConfigTemplate(m_preset_history, pfilename.toLatin1());
}


