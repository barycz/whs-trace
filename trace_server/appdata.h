#pragma once

#include <boost/serialization/version.hpp>

#include "filterstate.h"

struct AppData {

	void setDictLvl (QStringList const & names, QStringList const & strvalues)
	{
		m_dict_lvl.setContent(names, strvalues);
	}

	void setDictCtx (QStringList const & names, QStringList const & strvalues)
	{
		m_dict_ctx.setContent(names, strvalues);
	}

	Dict const & getDictLvl () const { return m_dict_lvl; }
	Dict const & getDictCtx () const { return m_dict_ctx; }

	template <class ArchiveT>
	void serialize (ArchiveT & ar, unsigned const version)
	{
		if(version > 0)
		{
			ar & boost::serialization::make_nvp("dict_lvl", m_dict_lvl);
		}
		ar & boost::serialization::make_nvp("dict_ctx", m_dict_ctx);
	}

private:
	Dict		m_dict_lvl;
	Dict		m_dict_ctx;
};

BOOST_CLASS_VERSION(AppData, 1)
