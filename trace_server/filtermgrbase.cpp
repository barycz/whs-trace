#include "filtermgr.h"
#include <algorithm>
#include <QGridLayout>
#include <QComboBox>
#include <QModelIndexList>
#include "ui_combolist.h"
#include "serialize.h"
// serialization stuff

FilterMgrBase::FilterMgrBase (QWidget * parent)
	: FilterBase(parent)
	, m_tabFilters(0)
	, m_tabCtxMenu(0)
	, m_delegate(0)
	, m_tabCtxModel(0)
	, m_currTab(0)
{
	m_filters.reserve(e_filtertype_max_value);
	m_filter_order.reserve(e_filtertype_max_value);
}
FilterMgrBase::~FilterMgrBase ()
{
	qDebug("%s", __FUNCTION__);
}

bool FilterMgrBase::accept (DecodedCommand const & cmd) const
{
	bool accepted = true;
	for (int i = 0, ie = m_filters.size(); i < ie; ++i)
	{
		FilterBase * b = m_filters[i];
		if (b->enabled())
			accepted &= b->accept(cmd);
	}
	return accepted;
}



void FilterMgrBase::loadConfig (QString const & path)
{
	QString const fname = path + "/" + g_filterTag + "/" + typeName();
	if (!::loadConfigTemplate(*this, fname))
		defaultConfig();

	recreateFilters();

	for (int i = 0, ie = m_filters.size(); i < ie; ++i)
		m_filters[i]->loadConfig(path);
}

void FilterMgrBase::saveConfig (QString const & path)
{
	m_currTab = m_tabFilters->currentIndex();
	QString const fname = path + "/" + g_filterTag + "/" + typeName();
	::saveConfigTemplate(*this, fname);

	for (int i = 0, ie = m_filters.size(); i < ie; ++i)
		m_filters[i]->saveConfig(path);
}

void FilterMgrBase::applyConfig ()
{
	m_tabFilters->setCurrentIndex(m_currTab);
	for (int i = 0, ie = m_filters.size(); i < ie; ++i)
		m_filters[i]->applyConfig();
}


bool FilterMgrBase::someFilterEnabled () const
{
	bool some_filter_enabled = false;
	for (int i = 0, ie = m_filters.size(); i < ie; ++i)
		if (m_filters[i] && m_filters[i]->enabled())
			some_filter_enabled |= 1;
	return some_filter_enabled;
}

bool FilterMgrBase::enabled () const
{
	return FilterBase::enabled() && someFilterEnabled();
}



void FilterMgrBase::addFilter (FilterBase * b)
{
	m_filters.push_back(b);
}
void FilterMgrBase::rmFilter (FilterBase * & b)
{
	m_filters.erase(std::remove(m_filters.begin(), m_filters.end(), b), m_filters.end());
	delete b;
	b = 0;
}


void FilterMgrBase::mvFilter (int from, int to)
{
	m_filters.move(from, to);
	m_filter_order.move(from, to);
}

void FilterMgrBase::onTabMoved (int from, int to)
{
	mvFilter(from, to);
	//TODO: emit signal to recalc model?
}

void FilterMgrBase::connectFiltersTo (QWidget * w)
{
	connect(m_tabCtxMenu->ui->commitButton, SIGNAL(clicked()), w, SLOT(onRefillFilters()));
	for (int i = 0, ie = m_filters.size(); i < ie; ++i)
		connect(m_filters[i], SIGNAL(filterChangedSignal()), w, SLOT(onFilterChanged()));
}

void FilterMgrBase::disconnectFiltersTo (QWidget * w)
{
	for (int i = 0, ie = m_filters.size(); i < ie; ++i)
		disconnect(m_filters[i], SIGNAL(filterChangedSignal()), w, SLOT(onFilterChanged()));
}

void FilterMgrBase::clearUI ()
{
	if (m_tabCtxModel && m_tabCtxModel->hasChildren())
		m_tabCtxModel->removeRows(0, m_tabCtxModel->rowCount());
}

void FilterMgrBase::setConfigToUI ()
{
	clearUI();
	for (int i = 0, ie = m_filters.size(); i < ie; ++i)
	{
		//QComboBox * cbx = new QComboBox(m_tabCtxMenu);
		//fillComboBoxWithFilters(cbx);
		//cbx->setCurrentIndex(m_filters[i]->type());
		if (m_filters[i])
		{
			QStandardItem * const qitem = new QStandardItem(m_filters[i]->typeName());
			qitem->setCheckable(true);
			qitem->setCheckState(m_filters[i]->enabled() ? Qt::Checked : Qt::Unchecked);
			m_tabCtxModel->appendRow(qitem);
		}
	}
}

void FilterMgrBase::onCtxAddButton ()
{
	QComboBox * cbx = new QComboBox(m_tabCtxMenu);
	fillComboBoxWithFilters(cbx);

//@TODO
	QStandardItem * const qitem = new QStandardItem(g_filterNames[1]);
	qitem->setCheckable(true);
	qitem->setCheckState(Qt::Checked);
	m_tabCtxModel->appendRow(qitem);
}

void FilterMgrBase::onCtxRmButton ()
{
	QModelIndexList const idxs = m_tabCtxMenu->ui->filterView->selectionModel()->selectedIndexes();
	foreach (QModelIndex index, idxs)
	{
		m_tabCtxModel->removeRow(index.row());
	}
}

void FilterMgrBase::onCtxCommitButton ()
{
	setUIToConfig();
	applyConfig();
}

void FilterMgrBase::setUIToConfig ()
{
	m_filter_order.clear();
	m_filter_order.reserve(e_filtertype_max_value);

	for (int i = 0, ie = m_tabCtxModel->rowCount(); i < ie; ++i)
	{
		QStandardItem const * const qitem = m_tabCtxModel->item(i, 0);
		if (qitem)
		{
			QString const & s = qitem->text();
			m_filter_order.push_back(s);
		}
	}
	recreateFilters();
    emit refillFilters();
}

void FilterMgrBase::onShowContextMenu (QPoint const & pt)
{
	setConfigToUI();

	bool const visible = m_tabCtxMenu->isVisible();
    if (!visible)
    {
		QPoint global_pos = mapToGlobal(pt);
		m_tabCtxMenu->move(global_pos);
    }

	m_tabCtxMenu->setVisible(!visible);
}

void FilterMgrBase::onHideContextMenu ()
{
	m_tabCtxMenu->setVisible(false);
}

void FilterMgrBase::clear ()
{
}

void FilterMgrBase::onFilterEnabledChanged ()
{
	bool const some_enabled = someFilterEnabled();
	if (enabled() ^ some_enabled)
	{
		enable(some_enabled);
		emit filterEnabledChanged();
		qDebug("%s signal filterEnabledChanged", __FUNCTION__);
	}
	else
	{
		emitFilterChangedSignal();
	}
}

void FilterMgrBase::focusToFilter (E_FilterType type)
{
	for (int i = 0, ie = m_filters.size(); i < ie; ++i)
	{
		if (m_filters[i] && m_filters[i]->type() == type)
		{
			m_tabFilters->setCurrentIndex(i);
			return;
		}
	}
}

void FilterMgrBase::setConnection( Connection * connection )
{
	foreach(FilterBase * flt, m_filters)
	{
		if(flt)
		{
			flt->setConnection(connection);
		}
	}
}

