#pragma once
#include <QString>
#include <QColor>
#include <QVector>
#include <dockedconfig.h>

namespace table {

	struct TableConfig : DockedConfigBase
	{
		QString m_tag;
		QString m_title;
		std::vector<QString> m_hhdr;
		std::vector<int> m_hsize;
		std::vector<QString> m_vhdr;
		std::vector<int> m_vsize;

		bool m_hide_empty;
		bool m_unused_b2;

		TableConfig ()
			: m_tag()
			, m_title()
			, m_hide_empty(false)
			, m_unused_b2(false)
		{
	/*		m_hhdr.reserve(32);
			m_hsize.reserve(32);
			m_vhdr.reserve(32);
			m_vsize.reserve(32);*/
		}

		TableConfig (QString const & tag)
			: m_tag(tag)
		{ }

		template <class ArchiveT>
		void serialize (ArchiveT & ar, unsigned const version)
		{
			ar & boost::serialization::make_nvp("tag", m_tag);
			ar & boost::serialization::make_nvp("hhdr", m_hhdr);
			ar & boost::serialization::make_nvp("hsz", m_hsize);
			ar & boost::serialization::make_nvp("vhdr", m_vhdr);
			ar & boost::serialization::make_nvp("vsz", m_vsize);
			ar & boost::serialization::make_nvp("sync_group", m_sync_group);
			// flags
			ar & boost::serialization::make_nvp("autoscroll", m_auto_scroll);
			//ar & boost::serialization::make_nvp("central_widget", m_central_widget);
			ar & boost::serialization::make_nvp("show", m_show);
			ar & boost::serialization::make_nvp("hide_empty", m_hide_empty);
			ar & boost::serialization::make_nvp("flag2", m_unused_b2);
		}

		void clear ()
		{
			*this = TableConfig();
		}
	};

	bool loadConfig (TableConfig & config, QString const & fname);
	bool saveConfig (TableConfig const & config, QString const & fname);
	void fillDefaultConfig (TableConfig & config);
}

