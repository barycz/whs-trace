#pragma once
#include <QString>
#include <QColor>
#include <QVector>
#include "history.h"

struct FindConfig
{
	bool m_whole_word;
	bool m_case_sensitive;
	bool m_regexp;
	bool m_prev;
	bool m_next;
	bool m_select;
	bool m_refs;
	bool m_clone;
	unsigned m_history_ln;
	History<QString> m_history;
	QStringList m_to_widgets;
	QString m_str;
	QRegExp m_regexp_val;
	bool m_selectedColumn;
	bool m_wrapSearch;
	FindConfig()
		: m_whole_word(false)
		, m_case_sensitive(false)
		, m_regexp(false)
		, m_prev(false)
		, m_next(false)
		, m_select(false)
		, m_refs(true)
		, m_clone(false)
		, m_history_ln(32)
		, m_history(m_history_ln)
		, m_wrapSearch(false)
		, m_selectedColumn(false)
	{
	}

	template <class ArchiveT>
	void serialize (ArchiveT & ar, unsigned const version)
	{
		ar & boost::serialization::make_nvp("whole_word", m_whole_word);
		ar & boost::serialization::make_nvp("case_sensitive", m_case_sensitive);
		ar & boost::serialization::make_nvp("regexp", m_regexp);
		ar & boost::serialization::make_nvp("prev", m_prev);
		ar & boost::serialization::make_nvp("next", m_next);
		ar & boost::serialization::make_nvp("select", m_select);
		ar & boost::serialization::make_nvp("refs", m_refs);
		ar & boost::serialization::make_nvp("clone", m_clone);
		ar & boost::serialization::make_nvp("history_ln", m_history_ln);
		ar & boost::serialization::make_nvp("history", m_history);
		ar & boost::serialization::make_nvp("to_widget", m_to_widgets);
		ar & boost::serialization::make_nvp("str", m_str);
		ar & boost::serialization::make_nvp("wrap_search", m_wrapSearch);
		ar & boost::serialization::make_nvp("selected_column", m_selectedColumn);
	}

	void clear ();
};

inline bool matchToFindConfig (QString const & str, FindConfig const & fc)
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

Q_DECLARE_METATYPE(FindConfig)

bool loadConfig (FindConfig & config, QString const & fname);
bool saveConfig (FindConfig const & config, QString const & fname);
void fillDefaultConfig (FindConfig & config);

