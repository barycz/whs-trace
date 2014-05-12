#pragma once
#include <QString>
#include <QColor>
#include <vector>
#include "frameviewconfig.h"
#include "dockedconfig.h"

//namespace gantt {

	struct FrameViewConfig : DockedConfigBase
	{
		QString m_tag;
		QString m_title;
		std::vector<FrameViewConfig> m_gvcfg;
		float m_begin;
		float m_end;
		float m_val1;
		float m_val2;
		float m_val3;
		float m_val4;
		QColor m_color1;
		QColor m_color2;
		QColor m_color3;
		QColor m_color4;
		bool m_on1;
		bool m_on2;
		bool m_on3;
		bool m_on4;

		FrameViewConfig ()
			: m_tag()
			, m_begin(0)
			, m_end(30)
			, m_val1(0.0f)
			, m_val2(0.5f)
			, m_val3(0.75f)
			, m_val4(0.95f)
			, m_color1(Qt::black)
			, m_color2(Qt::green)
			, m_color3(Qt::yellow)
			, m_color4(Qt::red)
			, m_on1(true)
			, m_on2(true)
			, m_on3(true)
			, m_on4(true)
		{ }

		FrameViewConfig (QString const & tag)
			: m_tag(tag)
			, m_begin(0)
			, m_end(30)
			, m_val1(0.0f)
			, m_val2(0.5f)
			, m_val3(0.75f)
			, m_val4(0.95f)
			, m_color1(Qt::black)
			, m_color2(Qt::green)
			, m_color3(Qt::yellow)
			, m_color4(Qt::red)
			, m_on1(true)
			, m_on2(true)
			, m_on3(true)
			, m_on4(true)
		{ }

		template <class ArchiveT>
		void serialize (ArchiveT & ar, unsigned const version)
		{
			DockedConfigBase::serialize(ar, version);
			ar & boost::serialization::make_nvp("tag", m_tag);
			ar & boost::serialization::make_nvp("title", m_title);
			ar & boost::serialization::make_nvp("val1", m_val1);
			ar & boost::serialization::make_nvp("val2", m_val2);
			ar & boost::serialization::make_nvp("val3", m_val3);
			ar & boost::serialization::make_nvp("val4", m_val4);
			ar & boost::serialization::make_nvp("color1", m_color1);
			ar & boost::serialization::make_nvp("color2", m_color2);
			ar & boost::serialization::make_nvp("color3", m_color3);
			ar & boost::serialization::make_nvp("color4", m_color4);
			ar & boost::serialization::make_nvp("on1", m_on1);
			ar & boost::serialization::make_nvp("on2", m_on2);
			ar & boost::serialization::make_nvp("on3", m_on3);
			ar & boost::serialization::make_nvp("on4", m_on4);
		}
	};

	bool loadConfig (FrameViewConfig & config, QString const & fname);
	void saveConfig (FrameViewConfig const & config, QString const & fname);
	void fillDefaultConfig (FrameViewConfig & config);
//}

