#include "config.h"
#include "serialize.h"

void GlobalConfig::loadHistory (QString const & path)
{
	QString const pfilename = path + "/preset_history.xml";
	::loadConfigTemplate(m_preset_history, pfilename.toLatin1());
	QString const rfilename = path + "/recent_history.xml";
	::loadConfigTemplate(m_recent_history, rfilename.toLatin1());
}

void GlobalConfig::saveHistory (QString const & path) const
{
	QString const pfilename = path + "/preset_history.xml";
	::saveConfigTemplate(m_preset_history, pfilename.toLatin1());
	QString const rfilename = path + "/recent_history.xml";
	::saveConfigTemplate(m_recent_history, rfilename.toLatin1());
}


