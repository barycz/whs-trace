
#include <boost/function.hpp>

#include <QPainter>
#include <QSpinBox>

#include "ui_filter_ctx.h"
#include "filter_ctx.h"
#include "constants.h"
#include "serialize.h"
#include "utils_qstandarditem.h"
#include "appdata.h"
#include "connection.h"

FilterCtx::FilterCtx (QWidget * parent)
	: FilterBase(parent)
	, m_ui(new Ui_FilterCtx)
{
	connect(this, SIGNAL(connectionChanged(Connection*)), this, SLOT(onConnectionChanged(Connection*)));
	initUI();
	setupModel();
}

FilterCtx::~FilterCtx ()
{
	destroyModel();
	doneUI();
}

void FilterCtx::initUI ()
{
	m_ui->setupUi(this);
}

void FilterCtx::doneUI ()
{
}

bool FilterCtx::accept (DecodedCommand const & cmd) const
{
	QString ctxstr;
	if (!cmd.getString(tlv::tag_ctx, ctxstr))
		return true;

	QString lvlstr;
	int lvl = 0;
	if (cmd.getString(tlv::tag_lvl, lvlstr))
	{
		lvl = lvlstr.toInt();
	}

	return model().shouldPassFilter(FilteredContext::ContextToNumber(ctxstr), lvl);
}


void FilterCtx::defaultConfig ()
{
	m_model.resetFilterData();
}

void FilterCtx::loadConfig (QString const & path)
{
	QString const fname = path + "/" + g_filterTag + "/" + typeName();
	if (!::loadConfigTemplate(*this, fname))
		defaultConfig();
}

void FilterCtx::saveConfig (QString const & path)
{
	QString const fname = path + "/" + g_filterTag + "/" + typeName();
	::saveConfigTemplate(*this, fname);
}

void FilterCtx::setConfigToUI ()
{
	// TODO: check if this can be empty (model should signal data change alone)
}

void FilterCtx::applyConfig ()
{
	FilterBase::applyConfig();
	setConfigToUI();
	m_ui->view->sortByColumn(FilterCtxModel::E_C_Name, Qt::AscendingOrder);
}

void FilterCtx::clear ()
{
	m_model.resetFilterData();
}

///////// ctx filters
void FilterCtx::setupModel ()
{
	m_ui->view->setModel(&m_model);
	m_ui->view->sortByColumn(FilterCtxModel::E_C_Name, Qt::AscendingOrder);
	m_ui->view->expandAll();
	m_ui->view->setItemDelegateForColumn(FilterCtxModel::E_C_Name, &m_ctxNameDelegate);
	m_ui->view->setItemDelegateForColumn(FilterCtxModel::E_C_Level, &m_ctxLevelDelegate);
	m_ui->view->setColumnWidth(FilterCtxModel::E_C_Level, 50);

	connect(m_ui->allButton, SIGNAL(clicked()), this, SLOT(onSelectAll()));
	connect(m_ui->noneButton, SIGNAL(clicked()), this, SLOT(onSelectNone()));
	connect(&m_model, SIGNAL(dataChanged(QModelIndex, QModelIndex, QVector<int>)), this, SLOT(onModelDataChanged()));
	connect(&m_model, SIGNAL(rowsInserted(QModelIndex, int, int)), this, SLOT(onModelDataChanged()));
}

void FilterCtx::destroyModel ()
{
}

bool FilterCtx::isCtxPresent (QString const & item, bool & enabled) const
{
	return m_model.isCtxPresent(item, enabled);
}

void FilterCtx::appendCtxFilter (QString const & item)
{
	int level = connection() ? connection()->connectionConfig().m_level : 0;
	m_model.updateFilteredContext(FilteredContext(item, true, 0, level));
}

void FilterCtx::setAppData (AppData const * appdata)
{
	m_ctxNameDelegate.setAppData(appdata);
}


//////// slots
void FilterCtx::onSelectAll ()
{
	m_model.setAllToEnabled(true);
}

void FilterCtx::onSelectNone ()
{
	m_model.setAllToEnabled(false);
}

void FilterCtx::recompile ()
{ }

void FilterCtx::locateItem (QString const & item, bool scrollto, bool expand)
{
	QModelIndexList indexList = m_model.match(m_model.index(0, 0), Qt::DisplayRole, item);
	if (!indexList.empty())
	{
		QModelIndex const selectedIndex(indexList.first());
		getWidget()->setCurrentIndex(selectedIndex);
	}
}

QTreeView * FilterCtx::getWidget()
{
	return m_ui->view;
}

QTreeView const * FilterCtx::getWidget() const
{
	return m_ui->view;
}

void FilterCtx::onModelDataChanged()
{
	setCtxLevelsToConnection();
	emitFilterChangedSignal();
}

void FilterCtx::onConnectionChanged( Connection * connection )
{
	if(connection)
	{
		connect(connection, SIGNAL(levelChanged(int)), this, SLOT(onConnectionLevelChanged(int)));
		setCtxLevelsToConnection();
	}
}

void FilterCtx::onConnectionLevelChanged( int level )
{
	m_model.setAllToLevel(level);
}

void FilterCtx::setCtxLevelsToConnection()
{
	if(connection() == nullptr)
	{
		return;
	}

	QVector<context_t> ctx;
	QVector<int> lvl;
	std::for_each(model().filterData().begin(), model().filterData().end(), [&](const FilteredContext & it)
	{
		ctx.push_back(it.m_ctx);
		lvl.push_back(it.m_level);
	});

	connection()->setContextLevels(ctx.size(), ctx.data(), lvl.data());
}

//////// model
FilterCtxModel::FilterCtxModel( QObject * parent /*= nullptr*/ ) : QAbstractItemModel(parent)
{

}

QVariant FilterCtxModel::data(const QModelIndex &index, int role) const
{
	if (!index.isValid())
		return QVariant();

	unsigned id = static_cast<unsigned>(index.internalId());
	if(id > filterData().size())
	{
		return QVariant();
	}

	const FilteredContext & ctx = filterData()[id];
	if(index.column() == E_C_Name)
	{
		if (role == Qt::DisplayRole)
		{
			return ctx.m_ctx_str;
		}
		else if(role == Qt::CheckStateRole)
		{
			return ctx.m_is_enabled ? Qt::Checked : Qt::Unchecked;
		}
	}
	else if(index.column() == E_C_Level)
	{
		if (role == Qt::DisplayRole || role == Qt::EditRole)
		{
			return ctx.m_level;
		}
	}

	return QVariant();
}

bool FilterCtxModel::setData( const QModelIndex & index, const QVariant & value, int role /*= Qt::EditRole*/ )
{
	if (!index.isValid())
		return false;

	unsigned id = static_cast<unsigned>(index.internalId());
	if(id > filterData().size())
	{
		return false;
	}

	FilteredContext & ctx = filterData()[id];
	if(index.column() == E_C_Name && role == Qt::CheckStateRole)
	{
		auto oldVal = ctx.m_is_enabled;
		ctx.m_is_enabled = value.toInt() != Qt::Unchecked;
		if(oldVal != ctx.m_is_enabled)
		{
			emit dataChanged(index, index);
		}
		return true;
	}
	else if(index.column() == E_C_Level && role == Qt::EditRole)
	{
		auto oldVal = ctx.m_level;
		ctx.m_level = value.toInt();
		if(oldVal != ctx.m_level)
		{
			emit dataChanged(index, index);
		}
		return true;
	}

	return false;
}

Qt::ItemFlags FilterCtxModel::flags(const QModelIndex &index) const
{
	if (!index.isValid())
		return 0;

	Qt::ItemFlags ret = Qt::ItemIsEnabled | Qt::ItemIsSelectable;
	if(index.column() == E_C_Name)
	{
		ret |= Qt::ItemIsUserCheckable;
	}
	else if(index.column() == E_C_Level)
	{
		ret |= Qt::ItemIsEditable;
	}
	//auto ret = QAbstractItemModel::flags(index);
	return ret;
}

QModelIndex FilterCtxModel::index( int row, int column, const QModelIndex &parent /*= QModelIndex()*/ ) const
{
	if (!hasIndex(row, column, parent))
		return QModelIndex();

	Q_ASSERT(row < m_filterData.size());
	Q_ASSERT(column < E_C_Last);

	return createIndexImp(row, column);
}

int FilterCtxModel::rowCount( const QModelIndex &parent /*= QModelIndex()*/ ) const
{
	return isRoot(parent) ? static_cast<int>(m_filterData.size()) : 0;
}

int FilterCtxModel::columnCount( const QModelIndex &parent /*= QModelIndex()*/ ) const
{
	return isRoot(parent) ? E_C_Last : 0;
}

QVariant FilterCtxModel::headerData( int section, Qt::Orientation orientation, int role /*= Qt::DisplayRole*/ ) const
{
	if(role != Qt::DisplayRole || orientation != Qt::Horizontal)
	{
		return QVariant();
	}
	if(section == E_C_Name)
	{
		return tr("Context");
	}
	else if(section == E_C_Level)
	{
		return tr("Level");
	}
	else
	{
		return QVariant();
	}
}

void FilterCtxModel::resetFilterData( const ctx_filters_t & filterData )
{
	beginResetModel();
	m_filterData = filterData;
	endResetModel();
}

void FilterCtxModel::appendFilteredContext( const FilteredContext & filteredCtx )
{
	size_t newIndex = m_filterData.size();
	beginInsertRows(QModelIndex(), newIndex, newIndex);
	m_filterData.push_back(filteredCtx);
	endInsertRows();
}

bool FilterCtxModel::isCtxPresent( QString const & ctx, bool & enabled ) const
{
	foreach(const FilteredContext & it, filterData())
	{
		if(it.m_ctx_str == ctx)
		{
			enabled = it.m_is_enabled;
			return true;
		}
	}
	return false;
}

bool FilterCtxModel::shouldPassFilter( const context_t ctx, const int lvl ) const
{
	foreach(const FilteredContext & it, filterData())
	{
		if(it.m_ctx == ctx)
		{
			return it.m_level >= lvl && it.m_is_enabled;
		}
	}

	return true;
}

void FilterCtxModel::updateFilteredContext( const FilteredContext & ctx )
{
	for(size_t r = 0; r < filterData().size(); ++r)
	{
		FilteredContext & it = filterData().at(r);
		if(it.m_ctx_str == ctx.m_ctx_str)
		{
			QModelIndex bgn = createIndexImp(r, E_C_Name);
			QModelIndex end = createIndexImp(r, E_C_Last - 1);
			it = ctx;
			emit dataChanged(bgn, end);
			return;
		}
	}

	appendFilteredContext(ctx);
}

void FilterCtxModel::setAllToEnabled( bool enabled )
{
	std::for_each(filterData().begin(), filterData().end(), [&](FilteredContext & ctx)
	{
		ctx.m_is_enabled = enabled;
	});
	emit dataChanged(QModelIndex(), QModelIndex());
}

void FilterCtxModel::setAllToLevel( int level )
{
	std::for_each(filterData().begin(), filterData().end(), [&](FilteredContext & ctx)
	{
		ctx.m_level = level;
	});
	emit dataChanged(QModelIndex(), QModelIndex());
}

//////// delegate
CtxDelegate::~CtxDelegate ()
{
	qDebug("%s", __FUNCTION__);
}

void CtxDelegate::paint (QPainter * painter, QStyleOptionViewItem const & option, QModelIndex const & index) const
{
	painter->save();
	QStyleOptionViewItemV4 option4 = option;
	initStyleOption(&option4, index);

	if (m_app_data && m_app_data->getDictCtx().m_names.size())
	{
		QVariant const value = index.data(Qt::DisplayRole);
		if (value.isValid() && !value.isNull())
		{
			Dict const & dict = m_app_data->getDictCtx();

			option4.text = dict.findNameFor(value.toString());
			QWidget const * widget = option4.widget;
			if (widget)
			{
				QStyle * style = widget->style();
				style->drawControl(QStyle::CE_ItemViewItem, &option4, painter, widget);
			}
		}
	}
	else
		QStyledItemDelegate::paint(painter, option4, index);
	painter->restore();
}

QWidget * CtxLevelDelegate::createEditor( QWidget * parent, const QStyleOptionViewItem & option, const QModelIndex & index ) const
{
	Q_UNUSED(option);
	QSpinBox * editor = new QSpinBox(parent);
	editor->setMinimum(0);
	//editor->setMaximum(trace::e_max_trace_level - 1);
	return editor;
}

void CtxLevelDelegate::setEditorData( QWidget *editor, const QModelIndex &index ) const
{
	int value = index.model()->data(index, Qt::EditRole).toInt();
	QSpinBox *spinBox = static_cast<QSpinBox*>(editor);
	spinBox->setValue(value);
}

void CtxLevelDelegate::setModelData( QWidget *editor, QAbstractItemModel *model, const QModelIndex &index ) const
{
	QSpinBox *spinBox = static_cast<QSpinBox*>(editor);
	spinBox->interpretText();
	int value = spinBox->value();

	model->setData(index, value, Qt::EditRole);
}

void CtxLevelDelegate::updateEditorGeometry( QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex & ) const
{
	editor->setGeometry(option.rect);
}
