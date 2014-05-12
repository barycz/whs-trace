#pragma once
#include "types.h"
#include "constants.h"
#include "history.h"
#include <QDir>

struct GlobalConfig {
	unsigned  m_hotkey;
	bool	  m_hidden;
	bool	  m_was_maximized;
	bool	  m_dump_mode;
	bool	  m_auto_scroll;
	bool	  m_buffered;
	bool	  m_on_top;
	int		  m_level;
	int		  m_logs_recv_level;
	int		  m_plots_recv_level;
	int		  m_tables_recv_level;
	int		  m_gantts_recv_level;
	QString	  m_time_units_str;
	float	  m_time_units;
	QString	  m_font;
	int		  m_fontsize;
	QString	  m_trace_addr;
	unsigned short m_trace_port;
	QString	  m_profiler_addr;
	unsigned short m_profiler_port;
	QString	  m_appdir;

	History<QString> m_preset_history;

	GlobalConfig ()
		: m_hotkey(0x91 /*VK_SCROLL*/)
		, m_hidden(false)
		, m_was_maximized(false)
		, m_dump_mode(false)
		, m_auto_scroll(false)
		, m_buffered(true)
		, m_on_top(false)
		, m_level(3)
		, m_logs_recv_level(2)
		, m_plots_recv_level(0)
		, m_tables_recv_level(0)
		, m_gantts_recv_level(0)
		, m_time_units_str("ms")
		, m_time_units(stringToUnitsValue(m_time_units_str))
		, m_font("Verdana")
		, m_fontsize(10)
		, m_trace_addr("127.0.0.1")
		, m_trace_port(g_defaultPort)
		, m_profiler_addr("127.0.0.1")
		, m_profiler_port(13147)
		, m_appdir(QDir::homePath() + "/" + g_traceServerDirName)
		, m_preset_history(16)
	{ }

	void loadHistory (QString const & path);
	void saveHistory (QString const & path) const;

	void fillDefaultConfig ()
	{
		*this = GlobalConfig();
	}

	template <class ArchiveT>
	void serialize (ArchiveT & ar, unsigned const version)
	{
		ar & boost::serialization::make_nvp("hotkey", m_hotkey);
		ar & boost::serialization::make_nvp("hidden", m_hidden);
		ar & boost::serialization::make_nvp("was_maximized", m_was_maximized);
		ar & boost::serialization::make_nvp("dump_mode", m_dump_mode);
		ar & boost::serialization::make_nvp("autoscroll", m_auto_scroll);
		ar & boost::serialization::make_nvp("buffered", m_buffered);
		ar & boost::serialization::make_nvp("on_top", m_on_top);
		ar & boost::serialization::make_nvp("level", m_level);
		ar & boost::serialization::make_nvp("m_logs_recv_level", m_logs_recv_level);
		ar & boost::serialization::make_nvp("m_plots_recv_level", m_plots_recv_level);
		ar & boost::serialization::make_nvp("m_tables_recv_level", m_tables_recv_level);
		ar & boost::serialization::make_nvp("m_gantts_recv_level", m_gantts_recv_level);
		ar & boost::serialization::make_nvp("time_units_str", m_time_units_str);
		ar & boost::serialization::make_nvp("time_units", m_time_units);
		ar & boost::serialization::make_nvp("font", m_font);
		ar & boost::serialization::make_nvp("fontsize", m_fontsize);

		ar & boost::serialization::make_nvp("trace_addr", m_trace_addr);
		ar & boost::serialization::make_nvp("trace_port", m_trace_port);
		ar & boost::serialization::make_nvp("profiler_addr", m_profiler_addr);
		ar & boost::serialization::make_nvp("profiler_port", m_profiler_port);
		//ar & boost::serialization::make_nvp("appdir", m_appdir); // do not want this probably
	}
};

struct TreeModelItem {
	/*@member	state
	 * duplicates qt enum
	 *	Qt::Unchecked	0	The item is unchecked.
	 *	Qt::PartiallyChecked	1	The item is partially checked. Items in hierarchical models may be partially checked if some, but not all, of their children are checked.
	 *	Qt::Checked	2	The item is checked.
	 */
	int m_state;
	int m_collapsed;

	TreeModelItem () : m_state(e_Unchecked), m_collapsed(true) { }
	TreeModelItem (int s) : m_state(s), m_collapsed(true) { }
	TreeModelItem (int s, bool c) : m_state(s), m_collapsed(c) { }

	template <class ArchiveT>
	void serialize (ArchiveT & ar, unsigned const version)
	{
		ar & boost::serialization::make_nvp("state", m_state);
		ar & boost::serialization::make_nvp("collapsed", m_collapsed);
	}
};


