#pragma once
#include <QString>
#include <QColor>
#include <dockedconfig.h>

namespace gantt {

	struct GanttViewConfig
	{
		QString m_tag;
		QString m_label;
		QString m_strtimeunits;
		int m_sync_group;
		QColor m_fgcolor;
		QColor m_bgcolor;
		QColor m_symbolcolor;
		int m_symbolsize;
		QString m_font;
		int m_fontsize;
		float m_timeunits;
		float m_scale;
		bool m_show;
		bool m_auto_scroll;
		bool m_auto_color;
		bool m_y_scaling;

		GanttViewConfig ()
			: m_strtimeunits(QString("ms"))
			, m_sync_group(0)
			, m_fgcolor(Qt::yellow)
			, m_bgcolor(Qt::black)
			, m_symbolcolor(Qt::red)
			, m_symbolsize(6)
			, m_font("Verdana")
			, m_fontsize(10)
			, m_timeunits(0.001f)
			, m_scale(1.0f)
			, m_show(true)
			, m_auto_scroll(true)
			, m_auto_color(true)
			, m_y_scaling(false)
		{ }

		template <class ArchiveT>
		void serialize (ArchiveT & ar, unsigned const version)
		{
			ar & boost::serialization::make_nvp("tag", m_tag);
			ar & boost::serialization::make_nvp("label", m_label);
			ar & boost::serialization::make_nvp("strtimeunits", m_strtimeunits);
			ar & boost::serialization::make_nvp("sync_group", m_sync_group);
			ar & boost::serialization::make_nvp("fgcol", m_fgcolor);
			ar & boost::serialization::make_nvp("bgcol", m_bgcolor);
			ar & boost::serialization::make_nvp("symcol", m_symbolcolor);
			ar & boost::serialization::make_nvp("symsize", m_symbolsize);
			ar & boost::serialization::make_nvp("font", m_font);
			ar & boost::serialization::make_nvp("fontsize", m_fontsize);
			ar & boost::serialization::make_nvp("timeunits", m_timeunits);
			ar & boost::serialization::make_nvp("scale", m_scale);
			ar & boost::serialization::make_nvp("show", m_show);
			ar & boost::serialization::make_nvp("autoscroll", m_auto_scroll);
			ar & boost::serialization::make_nvp("autocolor", m_auto_color);
			ar & boost::serialization::make_nvp("y_scaling", m_y_scaling);
		}

		void setTimeUnits (QString const & unit)
		{
			if (unit == "ms")
				m_timeunits = 0.001f;
			if (unit == "us")
				m_timeunits = 0.000001f;
			if (unit == "s")
				m_timeunits = 1.0f;
			if (unit == "m")
				m_timeunits = 60.0f;
			else
				m_timeunits = 0.001f;
		}

	};

	struct GanttConfig : DockedConfigBase
	{
		QString m_tag;
		QString m_title;
		QList<GanttViewConfig> m_gvcfg;
		int m_timer_delay_ms;
		int m_history_ln;

		GanttConfig ()
			: m_tag()
			, m_timer_delay_ms(50)
			, m_history_ln(2048)
		{ }

		GanttConfig (QString const & tag)
			: m_tag(tag)
			, m_timer_delay_ms(50)
			, m_history_ln(2048)
		{ }

		template <class ArchiveT>
		void serialize (ArchiveT & ar, unsigned const version)
		{
			DockedConfigBase::serialize(ar, version);
			ar & boost::serialization::make_nvp("tag", m_tag);
			ar & boost::serialization::make_nvp("title", m_title);
			ar & boost::serialization::make_nvp("gfcfg", m_gvcfg);
			ar & boost::serialization::make_nvp("timer", m_timer_delay_ms);
			ar & boost::serialization::make_nvp("length", m_history_ln);
		}

		bool findGanttViewConfig (QString const & tag, GanttViewConfig const * & ccfg)
		{
			for (int i = 0, ie = m_gvcfg.size(); i < ie; ++i)
				if (m_gvcfg.at(i).m_tag == tag)
				{
					ccfg = &m_gvcfg.at(i);
					return true;
				}
			return false;
		}

		void clear ()
		{
			*this = GanttConfig();
		}

		void fillDefaultConfig ()
		{
			*this = GanttConfig();
		}
	};

	bool loadConfig (GanttConfig & config, QString const & fname);
	bool saveConfig (GanttConfig const & config, QString const & fname);
	void fillDefaultConfig (GanttConfig & config);
}

