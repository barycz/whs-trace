#pragma once
#include "filterbase.h"
#include <QTabWidget>
#include <QStyledItemDelegate>
#include <QComboBox>
#include "combolist.h"
#include "movablelistmodel.h"
#include "movabletabwidget.h"

struct FilterMgrBase : FilterBase 
{
	QStringList 				m_filter_order;
	typedef QList<FilterBase *> filters_t;
	filters_t					m_filters;	/// user-order respecting filters

	FilterMgrBase (QWidget * parent = 0);
	virtual ~FilterMgrBase () = 0;

	virtual void initUI () = 0;
	virtual void doneUI () = 0;

	virtual E_FilterType type () const = 0;

	virtual bool accept (DecodedCommand const & cmd) const;
	virtual bool enabled () const;
	virtual bool someFilterEnabled () const;
	virtual void addFilter (FilterBase * b);
	virtual void rmFilter (FilterBase * & b);
	virtual void mvFilter (int from, int to);
	virtual FilterBase * filterFactory (E_FilterType t, QWidget * parent) = 0;
	virtual void recreateFilters () = 0;
	virtual void fillComboBoxWithFilters (QComboBox * cbx) = 0;

	virtual void defaultConfig () = 0;
	virtual void loadConfig (QString const & path);
	virtual void saveConfig (QString const & path);
	virtual void applyConfig ();
	virtual void clear ();

	void setConnection(Connection * connection);

	template <class ArchiveT>
	void serialize (ArchiveT & ar, unsigned const version)
	{
		FilterBase::serialize(ar, version);
		ar & boost::serialization::make_nvp("filter_order", m_filter_order);
		ar & boost::serialization::make_nvp("curr_tab", m_currTab);
	}

	void connectFiltersTo (QWidget * w);
	void disconnectFiltersTo (QWidget * w);

	void clearUI ();
	void setConfigToUI ();
	void setUIToConfig ();
	void focusToFilter (E_FilterType type);

public slots:
	void onFilterEnabledChanged ();
	void onShowContextMenu (QPoint const & pt);
	void onHideContextMenu ();
	void onCtxAddButton ();
	void onCtxRmButton ();
	void onCtxCommitButton ();
	void onTabMoved (int from, int to);
signals:
    void refillFilters ();

public:
    MovableTabWidget *		m_tabFilters;
	ComboList *				m_tabCtxMenu;
	QStyledItemDelegate *	m_delegate;
	MyListModel *			m_tabCtxModel;
	int						m_currTab;
	Q_OBJECT
};


