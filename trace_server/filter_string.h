#pragma once
#include "filterbase.h"
#include "ui_filter_string.h"
#include <boost/serialization/nvp.hpp>

#include "config.h"
#include <QList>
#include <QStandardItemModel>
#include <QStyledItemDelegate>

struct FilteredString {
	QString m_string;
	bool m_is_enabled;
	int m_state;

	bool match (QString const & str) const { return str.contains(m_string, Qt::CaseInsensitive); }

	FilteredString () { }
	FilteredString (QString const & s, bool enabled, int state)
        : m_string(s), m_is_enabled(enabled), m_state(state)
	{ }

	template <class ArchiveT>
	void serialize (ArchiveT & ar, unsigned const version)
	{
		ar & boost::serialization::make_nvp("string", m_string);
		ar & boost::serialization::make_nvp("is_enabled", m_is_enabled);
		ar & boost::serialization::make_nvp("state", m_state);
	}
};


struct FilterString : FilterBase
{
	Ui_FilterString * m_ui;

	FilterString (QWidget * parent = 0);
	virtual ~FilterString ();

	virtual void initUI ();
	virtual void doneUI ();

	virtual E_FilterType type () const { return e_Filter_String; }

	virtual bool accept (DecodedCommand const & cmd) const;

	virtual void defaultConfig ();
	virtual void loadConfig (QString const & path);
	virtual void saveConfig (QString const & path);
	virtual void applyConfig ();
	virtual void clear ();

	template <class ArchiveT>
	void serialize (ArchiveT & ar, unsigned const version)
	{
		FilterBase::serialize(ar, version);
		ar & boost::serialization::make_nvp("filtered_strings", m_data);
	}

	// string filtering
	void setupModel ();
	void destroyModel ();
	void appendToStringFilters (QString const & str, bool checked, int state);
	void appendToStringWidgets (FilteredString const & flt);
	void removeFromStringFilters(const QString& itemText);
	bool isMatchedStringExcluded (QString str) const;
	void setStringChecked (QString const & s, bool checked);
	void setStringState (QString const & s, int state);
	void recompile ();
	void locateItem (QString const & item, bool scrollto, bool expand);
	QTreeView * getWidget () { return m_ui->view; }
	QTreeView const * getWidget () const { return m_ui->view; }

	QList<FilteredString>	m_data;
	QStandardItemModel*	m_model;
	QStyledItemDelegate*   m_delegate;
		
	QList<FilteredString> m_enabledFilter;

	void CheckEnabledFilter();

	Q_OBJECT
public slots:
	void onClickedAtStringList (QModelIndex idx);
	void onStringRm ();
	void onStringAdd ();
signals:

};

struct StringDelegate : public QStyledItemDelegate
{
	StringDelegate (QObject * parent = 0) : QStyledItemDelegate(parent) { }
	void paint (QPainter * painter, QStyleOptionViewItem const & option, QModelIndex const & index) const;
};

