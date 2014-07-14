#pragma once
#include "filterbase.h"
#include "ui_filter_ctx.h"
#include <boost/serialization/nvp.hpp>

#include "config.h"
#include <QList>
#include <QStandardItemModel>
#include <QStyledItemDelegate>

struct AppData;

struct FilteredContext {
	QString m_ctx_str;
	unsigned long long m_ctx;
	bool m_is_enabled;
	int m_state;

	FilteredContext () { }
	FilteredContext (QString ctx, bool enabled, int state)
        : m_ctx_str(ctx), m_ctx(ctx.toULongLong()), m_is_enabled(enabled), m_state(state)
	{ }

	template <class ArchiveT>
	void serialize (ArchiveT & ar, unsigned const version)
	{
		ar & boost::serialization::make_nvp("ctx_str", m_ctx_str);
		ar & boost::serialization::make_nvp("ctx", m_ctx);
		ar & boost::serialization::make_nvp("is_enabled", m_is_enabled);
		ar & boost::serialization::make_nvp("state", m_state);
	}
};

inline bool operator< (FilteredContext const & lhs, FilteredContext const & rhs)
{
	return lhs.m_ctx < rhs.m_ctx;
}

struct FilterCtx : FilterBase
{
	Ui_FilterCtx * m_ui;

	FilterCtx (QWidget * parent = 0);
	virtual ~FilterCtx ();

	virtual void initUI ();
	virtual void doneUI ();

	virtual E_FilterType type () const { return e_Filter_Ctx; }

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
		ar & boost::serialization::make_nvp("ctx_filters", m_data);
	}

	// ctx specific
	void setAppData (AppData const * appdata);
	void setupModel ();
	void destroyModel ();
	bool isCtxPresent (QString const & item, bool & enabled) const;
	void appendCtxFilter (QString const & item);
	void removeCtxFilter (QString const & item);
	void recompile ();
	void setConfigToUI ();
	void appendToCtxWidgets (FilteredContext const & flt);
	void locateItem (QString const & item, bool scrollto, bool expand);
	QTreeView * getWidget () { return m_ui->view; }
	QTreeView const * getWidget () const { return m_ui->view; }

	typedef std::vector<FilteredContext> ctx_filters_t;
	ctx_filters_t			m_data;
	QStandardItemModel *	m_model;

	Q_OBJECT
public slots:
	void onClickedAtCtx (QModelIndex idx);
	void onSelectAll ();
	void onSelectNone ();
signals:
};

struct CtxDelegate : public QStyledItemDelegate
{
	AppData const * m_app_data;
	CtxDelegate (QObject * parent = 0) : QStyledItemDelegate(parent), m_app_data(0) { }
	~CtxDelegate ();
	void paint (QPainter * painter, QStyleOptionViewItem const & option, QModelIndex const & index) const;

	void setAppData (AppData const * appdata) { m_app_data = appdata; }
};


