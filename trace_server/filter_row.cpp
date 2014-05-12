#include "filter_row.h"
#include "constants.h"
#include "serialize.h"
#include <QPainter>
#include "utils_qstandarditem.h"
#include <boost/function.hpp>

FilterRow::FilterRow (QWidget * parent)
	: FilterBase(parent)
	, m_ui(new Ui_FilterRow)
	, m_data()
	, m_model(0)
	, m_delegate(0)
{
	initUI();
	setupModel();
}

FilterRow::~FilterRow ()
{
	destroyModel();
	doneUI();
}

void FilterRow::initUI ()
{
	m_ui->setupUi(this);
}

void FilterRow::doneUI ()
{
}

bool FilterRow::accept (DecodedCommand const & cmd) const
{
	int const row = cmd.m_src_row;

	E_RowMode rowmode = e_RowInclude;
	bool row_enabled = true;
	bool const row_present = isRowPresent(row, row_enabled, rowmode);

	bool excluded = false;
	if (row_present)
	{
		if (row_enabled && rowmode == e_RowForceInclude)
			return true; // forced levels (errors etc)
		excluded |= row_enabled;
	}
	return !excluded;
}

void FilterRow::defaultConfig ()
{
	m_data.clear();
}

void FilterRow::loadConfig (QString const & path)
{
	QString const fname = path + "/" + g_filterTag + "/" + typeName();
	if (!::loadConfigTemplate(*this, fname))
		defaultConfig();
}

void FilterRow::saveConfig (QString const & path)
{
	QString const fname = path + "/" + g_filterTag + "/" + typeName();
	::saveConfigTemplate(*this, fname);
}

void FilterRow::setConfigToUI ()
{
	m_model->clear();
	for (size_t i = 0, ie = m_data.size(); i < ie; ++i)
	{
		QStandardItem * root = m_model->invisibleRootItem();
		QStandardItem * child = findChildByText(root, m_data[i].m_row_str);
		if (child == 0)
		{
			QList<QStandardItem *> row_items = addRow(m_data[i].m_row_str, true);
			row_items[0]->setCheckState(m_data[i].m_is_enabled ? Qt::Checked : Qt::Unchecked);
			root->appendRow(row_items);
		}
	}
}

void FilterRow::applyConfig ()
{
	FilterBase::applyConfig();
	setConfigToUI();
	m_ui->view->sortByColumn(0, Qt::AscendingOrder);
}

void FilterRow::clear ()
{
	m_data.clear();
	m_model->clear();
	// @TODO m_row_model.clear();
}


///////// row filters
void FilterRow::setupModel ()
{
	if (!m_model)
	{
		qDebug("new tree view row model");
		m_model = new QStandardItemModel;
	}
	m_ui->view->setModel(m_model);
	m_ui->view->setSortingEnabled(true);
	m_ui->view->expandAll();
	//RowDelegate * d = new LevelDelegate(this);
	//m_ui->view->setItemDelegate(d);
	//m_ui->view->setRootIndex(m_model->indexFromItem(m_model->invisibleRootItem()));

	m_ui->view->setEditTriggers(QAbstractItemView::NoEditTriggers);
	connect(m_ui->view, SIGNAL(clicked(QModelIndex)), this, SLOT(onClickedAtRow(QModelIndex)));
	connect(m_ui->allButton, SIGNAL(clicked()), this, SLOT(onSelectAll()));
	connect(m_ui->noneButton, SIGNAL(clicked()), this, SLOT(onSelectNone()));
	m_ui->view->header()->hide();
}

void FilterRow::destroyModel ()
{
	//if (m_ui->view->itemDelegate())
	//	m_ui->view->setItemDelegate(0);
	if (m_ui->view->model() == m_model)
		m_ui->view->setModel(0);
	delete m_model;
	m_model = 0;
}

bool FilterRow::isRowPresent (int item, bool & enabled, E_RowMode & rowmode) const
{
	for (size_t i = 0, ie = m_data.size(); i < ie; ++i)
		if (m_data[i].m_row == item)
		{
			FilteredRow const & l = m_data[i];
			rowmode = static_cast<E_RowMode>(l.m_state);
			enabled = l.m_is_enabled;
			return true;
		}
	return false;
}

void FilterRow::appendRowToUI (FilteredRow const & f)
{
	QStandardItem * root = m_model->invisibleRootItem();
	QStandardItem * child = findChildByText(root, f.m_row_str);
	if (child == 0)
	{
		QList<QStandardItem *> row_items = addRow(f.m_row_str, true);
		row_items[0]->setCheckState(f.m_is_enabled ? Qt::Checked : Qt::Unchecked);
		root->appendRow(row_items);
	}
}

void FilterRow::appendRowFilter (int item)
{
	for (size_t i = 0, ie = m_data.size(); i < ie; ++i)
		if (m_data[i].m_row == item)
		{
			FilteredRow & f = m_data[i];
			f.m_is_enabled = true;
			appendRowToUI(f);
			return;
		}
	m_data.push_back(FilteredRow(item, true, e_RowInclude));
	appendRowToUI(m_data.back());

	std::sort(m_data.begin(), m_data.end());
	m_ui->view->sortByColumn(0, Qt::AscendingOrder);
}
void FilterRow::removeRowFilter (int item)
{
	for (size_t i = 0, ie = m_data.size(); i < ie; ++i)
		if (m_data[i].m_row == item)
		{
			FilteredRow & l = m_data[i];
			l.m_is_enabled = false;
			return;
		}
}
bool FilterRow::setRowMode (int item, bool enabled, E_RowMode rowmode)
{
	for (size_t i = 0, ie = m_data.size(); i < ie; ++i)
		if (m_data[i].m_row == item)
		{
			FilteredRow & l = m_data[i];
			l.m_state = rowmode;
			l.m_is_enabled = enabled;
			return true;
		}
	return false;
}

template <typename T, typename U>
void applyFnOnAllChildren_dup (T fn, U instance, QAbstractItemModel * abs_model, Qt::CheckState state)
{
	QStandardItemModel * model = static_cast<QStandardItemModel *>(abs_model);
	QStandardItem * root = model->invisibleRootItem();
	QList<QStandardItem *> l = listChildren(root);

	for (int i = 0, ie = l.size(); i < ie; ++i)
	{
		l.at(i)->setCheckState(state);
		QString const & data = model->data(l.at(i)->index(), Qt::DisplayRole).toString();
		fn(instance, data.toInt());
	}
}

void FilterRow::onSelectAll ()
{
	boost::function<void (FilterRow *, int)> f = &FilterRow::appendRowFilter; //@FIXME: inefficient!
	applyFnOnAllChildren_dup(f, this, m_model, Qt::Checked);
	emitFilterChangedSignal();
}
void FilterRow::onSelectNone ()
{
	boost::function<void (FilterRow *, int)> f = &FilterRow::removeRowFilter;
	applyFnOnAllChildren_dup(f, this, m_model, Qt::Unchecked);
	emitFilterChangedSignal();
}

void FilterRow::onClickedAtRow (QModelIndex idx)
{
	if (!idx.isValid()) return;
	QStandardItem * item = m_model->itemFromIndex(idx);
	Q_ASSERT(item);

	bool checked = (item->checkState() == Qt::Checked);

	if (idx.column() == 1)
	{
		QString const & filter_item = m_model->data(m_model->index(idx.row(), 0, QModelIndex()), Qt::DisplayRole).toString();
		QString const & mod = m_model->data(idx, Qt::DisplayRole).toString();

		E_RowMode const curr = stringToRowMod(mod.toStdString().c_str()[0]);
		size_t const i = (curr + 1) % e_max_rowmod_enum_value;
		E_RowMode const new_mode = static_cast<E_RowMode>(i);
		m_model->setData(idx, QString(rowModToString(new_mode)));

		setRowMode(filter_item.toInt(), !checked, new_mode);

		emitFilterChangedSignal();
	}
	else
	{
		QStandardItem * item = m_model->itemFromIndex(idx);
		Q_ASSERT(item);

		QString const & filter_item = m_model->data(idx, Qt::DisplayRole).toString();
		bool const orig_checked = (item->checkState() == Qt::Checked);
		if (orig_checked)
			appendRowFilter(filter_item.toInt());
		else
			removeRowFilter(filter_item.toInt());

		emitFilterChangedSignal();
	}
}

void FilterRow::recompile ()
{ }

void FilterRow::locateItem (QString const & item, bool scrollto, bool expand)
{
	/*QModelIndexList indexList = m_model->match(m_model->index(0, 0), Qt::DisplayRole, item);
	if (!indexList.empty())
	{
		QModelIndex const selectedIndex(indexList.first());
		getWidget()->setCurrentIndex(selectedIndex);
	}*/
}

