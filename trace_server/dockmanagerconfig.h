#pragma once
#include <QString>
#include <QVector>
#include <QColor>
#include "constants.h"
#include <filters/file_filter.hpp>
#include "types.h"
#include "dockedinfo.h"

struct DockManagerConfig
{
	QString 		m_tag;
	QString 		m_font;
	int 			m_fontsize;
	int 			m_row_width;
	std::vector<int> 	m_columns_sizes;		/// column sizes for each registered application
	bool 			m_show;
	typedef tree_filter<DockedInfo> data_filters_t;
	data_filters_t	m_data;

	DockManagerConfig (QString const & tag)
		: m_tag(tag)
		, m_font("Verdana")
		, m_fontsize(12)
		, m_row_width(24)
		, m_show(true)
		, m_data()
	{ }

	template <class ArchiveT>
	void serialize (ArchiveT & ar, unsigned const version)
	{
		ar & boost::serialization::make_nvp("tag", m_tag);
		ar & boost::serialization::make_nvp("font", m_font);
		ar & boost::serialization::make_nvp("fontsize", m_fontsize);
		ar & boost::serialization::make_nvp("row_width", m_row_width);
		ar & boost::serialization::make_nvp("columns_sizes", m_columns_sizes);
		ar & boost::serialization::make_nvp("show", m_show);
		ar & boost::serialization::make_nvp("docked_widgets_data", m_data);
	}

	void clear ()
	{
		m_data.clear();
		DockManagerConfig rhs(g_traceServerName);
		*this = rhs;
		rhs.m_data.root = 0;
	}

	void defaultConfig ()
	{
		m_font = "Verdana";
		m_fontsize = 12;
		m_row_width = 24;
		m_show = true;
		m_columns_sizes.clear();
		m_columns_sizes.push_back(128);
		m_columns_sizes.push_back(32);
		m_columns_sizes.push_back(512);
	}
};

bool loadConfig (DockManagerConfig & config, QString const & fname);
bool saveConfig (DockManagerConfig const & config, QString const & fname);

