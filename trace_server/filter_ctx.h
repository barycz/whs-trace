#pragma once

#include <boost/serialization/nvp.hpp>
#include <boost/serialization/version.hpp>

#include <QAbstractItemModel>
#include <QStyledItemDelegate>

#include "filterbase.h"
#include "config.h"

class Ui_FilterCtx;
struct AppData;

struct FilteredContext {
	QString m_ctx_str;
	context_t m_ctx;
	bool m_is_enabled;
	int m_state;
	int m_level;

	FilteredContext (): m_ctx(0), m_is_enabled(false), m_state(0), m_level(0) { }
	FilteredContext (QString ctx, bool enabled, int state, int level = 0):
		m_ctx_str(ctx), m_ctx(ctx.toULongLong(nullptr, 16)), m_is_enabled(enabled), m_state(state), m_level(level)
	{ }

	template <class ArchiveT>
	void serialize (ArchiveT & ar, unsigned const version)
	{
		ar & boost::serialization::make_nvp("ctx_str", m_ctx_str);
		ar & boost::serialization::make_nvp("ctx", m_ctx);
		if(version < 2 && ArchiveT::is_loading::value)
		{
			m_ctx = m_ctx_str.toULongLong(nullptr, 16);
		}
		ar & boost::serialization::make_nvp("is_enabled", m_is_enabled);
		ar & boost::serialization::make_nvp("state", m_state);
		if(version > 0)
		{
			ar & boost::serialization::make_nvp("level", m_level);
		}
	}
};

inline bool operator< (FilteredContext const & lhs, FilteredContext const & rhs)
{
	return lhs.m_ctx < rhs.m_ctx;
}

BOOST_CLASS_VERSION(FilteredContext, 2)

class FilterCtxModel: public QAbstractItemModel
{
	Q_OBJECT
public:
	friend class boost::serialization::access;

	enum E_Columns
	{
		E_C_Name = 0,
		E_C_Level = 1,
		E_C_Last
	};

	typedef std::vector<FilteredContext> ctx_filters_t;

	FilterCtxModel(QObject * parent = nullptr);

	// QAbstractItemModel
	QVariant data(const QModelIndex &index, int role) const Q_DECL_OVERRIDE;
	bool setData(const QModelIndex & index, const QVariant & value, int role = Qt::EditRole) Q_DECL_OVERRIDE;
	QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;
	Qt::ItemFlags flags(const QModelIndex &index) const Q_DECL_OVERRIDE;
	QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
	QModelIndex parent(const QModelIndex &) const Q_DECL_OVERRIDE { return QModelIndex(); }
	int rowCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
	int columnCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
	// ~QAbstractItemModel

	QModelIndex createIndexImp(int row, int column) const { return createIndex(row, column, static_cast<quintptr>(row)); }

	bool isRoot(const QModelIndex & idx) const { return idx.isValid() == false; }

	void resetFilterData(const ctx_filters_t & filterData = ctx_filters_t());
	void appendFilteredContext(const FilteredContext & filteredCtx);
	bool isCtxPresent(QString const & ctx, bool & enabled) const;
	/// update existing or insert new filtered context
	void updateFilteredContext(const FilteredContext & ctx);
	void setAllToEnabled(bool enabled = true);
	void setAllToLevel(int level);

	const ctx_filters_t & filterData() const { return m_filterData; }

private:
	template<class Archive>
	void save(Archive & ar, const unsigned int version) const
	{
		ar & boost::serialization::make_nvp("data", m_filterData);
	}

	template<class Archive>
	void load(Archive & ar, const unsigned int version)
	{
		ctx_filters_t tmpData;
		ar & boost::serialization::make_nvp("data", tmpData);
		resetFilterData(tmpData);
	}

	BOOST_SERIALIZATION_SPLIT_MEMBER()

	ctx_filters_t & filterData() { return m_filterData; }

	ctx_filters_t m_filterData;
};

struct CtxDelegate : public QStyledItemDelegate
{
	AppData const * m_app_data;
	CtxDelegate (QObject * parent = 0) : QStyledItemDelegate(parent), m_app_data(0) { }
	~CtxDelegate ();

	void paint (QPainter * painter, QStyleOptionViewItem const & option, QModelIndex const & index) const;

	void setAppData (AppData const * appdata) { m_app_data = appdata; }
};

class CtxLevelDelegate: public QStyledItemDelegate
{
public:
	CtxLevelDelegate(QObject * parent = nullptr): QStyledItemDelegate(parent) {}
	~CtxLevelDelegate() {}

	QWidget * createEditor(QWidget * parent, const QStyleOptionViewItem & option, const QModelIndex & index) const Q_DECL_OVERRIDE;
	void setEditorData(QWidget *editor, const QModelIndex &index) const Q_DECL_OVERRIDE;
	void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const Q_DECL_OVERRIDE;
	void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const Q_DECL_OVERRIDE;

private:
};

class FilterCtx : public FilterBase
{
	Q_OBJECT
public:
	friend class boost::serialization::access;

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

	// ctx specific
	void setAppData (AppData const * appdata);
	void setupModel ();
	void destroyModel ();
	bool isCtxPresent (QString const & item, bool & enabled) const;
	void appendCtxFilter (QString const & item);
	void recompile ();
	void setConfigToUI ();
	void locateItem (QString const & item, bool scrollto, bool expand);
	QTreeView * getWidget ();
	QTreeView const * getWidget () const;

	const FilterCtxModel & model() const { return m_model; }

public slots:
	void onSelectAll ();
	void onSelectNone ();

private slots:
	void onModelDataChanged();
	void onConnectionChanged(Connection * connection);
	void onConnectionLevelChanged(int level);

private:
	template <class ArchiveT>
	void serialize (ArchiveT & ar, unsigned const version)
	{
		if(version > 0)
		{
			ar & boost::serialization::make_nvp("base", boost::serialization::base_object<FilterBase>(*this));
			ar & boost::serialization::make_nvp("ctx_filter_model", m_model);
		}
		else if(ArchiveT::is_loading::value)
		{
			FilterBase::serialize(ar, version);
			FilterCtxModel::ctx_filters_t tmpData;
			ar & boost::serialization::make_nvp("ctx_filters", tmpData);
			m_model.resetFilterData(tmpData);
		}
		else
		{
			Q_ASSERT(0);
		}
	}

	void setCtxLevelsToConnection();

	Ui_FilterCtx * m_ui;
	FilterCtxModel m_model;

	CtxDelegate m_ctxNameDelegate;
	CtxLevelDelegate m_ctxLevelDelegate;
};

BOOST_CLASS_VERSION(FilterCtx, 1)
