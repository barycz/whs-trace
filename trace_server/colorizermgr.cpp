#include "colorizermgr.h"
#include <algorithm>
#include <QGridLayout>
#include <QComboBox>
#include <QModelIndexList>
#include "ui_combolist.h"
#include "serialize.h"
// serialization stuff

bool ColorizerMgr::accept (DecodedCommand const & cmd) const
{
    return true;
}

bool ColorizerMgr::action (DecodedCommand const & cmd)
{
	bool accepted = true;
	for (int i = 0, ie = m_filters.size(); i < ie; ++i)
	{
		FilterBase * b = m_filters[i];
		if (b->enabled())
    {
		  bool const filter_accepted = b->action(cmd); // @TODO: short circuit?
			accepted |= filter_accepted;
    }
	}
	return accepted;
}


// @TODO: to base l8r (tag to virtual fn)
void ColorizerMgr::loadConfig (QString const & path)
{
	QString const fname = path + "/" + g_colorizerTag + "/" + typeName();
	if (!::loadConfigTemplate(*this, fname))
		defaultConfig();

	recreateFilters();

	for (int i = 0, ie = m_filters.size(); i < ie; ++i)
		m_filters[i]->loadConfig(path);
}

void ColorizerMgr::saveConfig (QString const & path)
{
	m_currTab = m_tabFilters->currentIndex();
	QString const fname = path + "/" + g_colorizerTag + "/" + typeName();
	::saveConfigTemplate(*this, fname);

	for (int i = 0, ie = m_filters.size(); i < ie; ++i)
		m_filters[i]->saveConfig(path);
}

FilterBase * ColorizerMgr::filterFactory (E_FilterType t, QWidget * parent)
{
	switch (t)
	{
		case e_Colorizer_Mgr: return new ColorizerMgr (parent);
		//case e_Colorizer_Script: return new ColorizerScript (parent);
		case e_Colorizer_String: return new ColorizerString (parent);
		case e_Colorizer_Regex: return new ColorizerRegex (parent);
		//case e_Colorizer_Ctx: return new ColorizerCtx (parent);
		//case e_Colorizer_Lvl: return new ColorizerLvl (parent);
		//case e_Colorizer_Tid: return new ColorizerTid (parent);
		//case e_Colorizer_FileLine: return new ColorizerFileLine (parent);
		case e_Colorizer_Row: return new ColorizerRow (parent);
        // time, fn, dt
		default: return 0;
	}
}

/////////////////// FILTER MGR ///////////////////////////////

ColorizerMgr::ColorizerMgr (QWidget * parent)
	: FilterMgr(parent)
{
	m_cache.resize(e_filtertype_max_value);
}

ColorizerMgr::~ColorizerMgr ()
{
	qDebug("%s", __FUNCTION__);
}

void ColorizerMgr::defaultConfig ()
{
	m_filter_order.clear();
	// @TODO clear others? probably no
	m_filter_order.push_back(g_filterNames[e_Colorizer_String]);
	//m_filter_order.push_back(g_filterNames[e_Colorizer_Ctx]);
	//m_filter_order.push_back(g_filterNames[e_Colorizer_Lvl]);
	//m_filter_order.push_back(g_filterNames[e_Colorizer_FileLine]);
	m_filter_order.push_back(g_filterNames[e_Colorizer_Regex]);
	m_filter_order.push_back(g_filterNames[e_Colorizer_Row]);
}

/////////////////////////////////////////////////////////////////////

void ColorizerMgr::initUI ()
{
    FilterMgr::initUI();
	setObjectName(QStringLiteral("ColorizerWidget"));
	//m_widget->setObjectName(QStringLiteral("FilterWidget"));
}

void ColorizerMgr::doneUI ()
{
    FilterMgr::doneUI();
}

void ColorizerMgr::clear ()
{
}
