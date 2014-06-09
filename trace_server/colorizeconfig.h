#pragma once
#include "findconfig.h"

struct ColorizeConfig : FindConfig
{
	QColor m_fgcolor;
	QColor m_bgcolor;

	ColorizeConfig ()
		: FindConfig()
		, m_fgcolor(Qt::blue)
		, m_bgcolor(Qt::white)
	{
	}

	template <class ArchiveT>
	void serialize (ArchiveT & ar, unsigned const version)
	{
		FindConfig::serialize(ar, version);
		ar & boost::serialization::make_nvp("fgcolor", m_fgcolor);
		ar & boost::serialization::make_nvp("bgcolor", m_bgcolor);
	}

	void clear ();
};

inline bool matchToColorizeConfig (QString const & str, ColorizeConfig const & fc)
{
	if (fc.m_regexp)
	{
		QRegExp const & r = fc.m_regexp_val;
		if (fc.m_regexp_val.isValid())
		{
			return r.exactMatch(str);
		}
	}
	else
	{
		Qt::CaseSensitivity const cs = fc.m_case_sensitive ? Qt::CaseSensitive : Qt::CaseInsensitive;
		if (str.contains(fc.m_str, cs))
		{
			return true;
		}
	}
	return false;
}

Q_DECLARE_METATYPE(ColorizeConfig)

bool loadConfig (ColorizeConfig & config, QString const & fname);
bool saveConfig (ColorizeConfig const & config, QString const & fname);
void fillDefaultConfig (ColorizeConfig & config);

