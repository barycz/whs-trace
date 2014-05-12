#pragma once
#include "filterbase.h"
#include "ui_filter_row.h"

#include "config.h"
#include <QObject>
#include <QStandardItemModel>
#include <QStyledItemDelegate>

struct FilteredRow {
	QString m_row_str;
	int m_row;
	bool m_is_enabled;
	int m_state;

	FilteredRow () { }
	FilteredRow (int row, bool enabled, int state)
        : m_row_str(QString::number(row)), m_row(row), m_is_enabled(enabled), m_state(state)
	{ }

	template <class ArchiveT>
	void serialize (ArchiveT & ar, unsigned const version)
	{
		ar & boost::serialization::make_nvp("row_str", m_row_str);
		ar & boost::serialization::make_nvp("row", m_row);
		ar & boost::serialization::make_nvp("is_enabled", m_is_enabled);
		ar & boost::serialization::make_nvp("state", m_state);
	}
};

inline bool operator< (FilteredRow const & lhs, FilteredRow const & rhs)
{
	return lhs.m_row < rhs.m_row;
}

struct FilterRow : FilterBase
{
	Ui_FilterRow * m_ui;

	FilterRow (QWidget * parent = 0);
	virtual ~FilterRow ();

	virtual void initUI ();
	virtual void doneUI ();

	virtual E_FilterType type () const { return e_Filter_Row; }

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
		ar & boost::serialization::make_nvp("row_filters", m_data);
	}

	// row specific
	void setupModel ();
	void destroyModel ();
	bool isRowPresent (int item, bool & enabled, E_RowMode & rowmode) const;
	void appendRowFilter (int item);
	void appendRowToUI (FilteredRow const & f);
	void removeRowFilter (int item);
	bool setRowMode (int item, bool enabled, E_RowMode rowmode);
	void recompile ();
	void setConfigToUI ();
	QTreeView * getWidget () { return m_ui->view; }
	QTreeView const * getWidget () const { return m_ui->view; }
	void locateItem (QString const & item, bool scrollto, bool expand);

	typedef std::vector<FilteredRow> row_filters_t;
	row_filters_t			m_data;
	QStandardItemModel *	m_model;
	QStyledItemDelegate *   m_delegate;

	Q_OBJECT
public slots:
	void onClickedAtRow (QModelIndex idx);
	void onSelectAll ();
	void onSelectNone ();
signals:
};

/*struct RowDelegate : public QStyledItemDelegate
{
	RowDelegate (QObject * parent = 0) : QStyledItemDelegate(parent) { }
	void paint (QPainter * painter, QStyleOptionViewItem const & option, QModelIndex const & index) const;
};
*/
