#include "filter_lvl.h"
#include "constants.h"
#include "serialize.h"
#include <QPainter>
#include "utils_qstandarditem.h"
#include <boost/function.hpp>
#include "appdata.h"

FilterLvl::FilterLvl (QWidget * parent)
	: FilterBase(parent)
	, m_ui(new Ui_FilterLvl)
	, m_data()
	, m_model(0)
	, m_delegate(0)
{
	initUI();
	setupModel();
}

FilterLvl::~FilterLvl ()
{
	destroyModel();
	doneUI();
}

void FilterLvl::initUI ()
{
	m_ui->setupUi(this);
}

void FilterLvl::doneUI ()
{
}

bool FilterLvl::accept (DecodedCommand const & cmd) const
{
	QString lvl;
	if (!cmd.getString(tlv::tag_lvl, lvl))
		return true;

	E_LevelMode lvlmode = e_LvlInclude;
	bool lvl_enabled = true;
	bool const lvl_present = isPresent(lvl, lvl_enabled, lvlmode);

	bool excluded = false;
	if (lvl_present)
	{
		if (lvl_enabled && lvlmode == e_LvlForceInclude)
			return true; // forced levels (errors etc)
		excluded |= !lvl_enabled;
	}
	return !excluded;
}

void FilterLvl::defaultConfig ()
{
	m_data.clear();
}

void FilterLvl::loadConfig (QString const & path)
{
	QString const fname = path + "/" + g_filterTag + "/" + typeName();
	if (!::loadConfigTemplate(*this, fname))
		defaultConfig();
}

void FilterLvl::saveConfig (QString const & path)
{
	QString const fname = path + "/" + g_filterTag + "/" + typeName();
	::saveConfigTemplate(*this, fname);
}

void FilterLvl::setConfigToUI ()
{
	for (size_t i = 0, ie = m_data.size(); i < ie; ++i)
	{
		QStandardItem * root = m_model->invisibleRootItem();
		QStandardItem * child = findChildByText(root, m_data[i].m_level_str);
		if (child == 0)
		{
			//FilteredContext & fc = m_data[i];
			QList<QStandardItem *> row_items = addRow(m_data[i].m_level_str, true);
			row_items[0]->setCheckState(m_data[i].m_is_enabled ? Qt::Checked : Qt::Unchecked);
			root->appendRow(row_items);
		}
	}
}

void FilterLvl::applyConfig ()
{
	FilterBase::applyConfig();
	setConfigToUI();
	m_ui->view->sortByColumn(0, Qt::AscendingOrder);
	//m_lvl_filters = src.m_lvl_filters;
}

void FilterLvl::clear ()
{
	m_data.clear();
	// @TODO m_lvl_model.clear();
}


///////// lvl filters
void FilterLvl::setupModel ()
{
	if (!m_model)
	{
		qDebug("new tree view lvl model");
		m_model = new QStandardItemModel;
	}
	m_ui->view->setModel(m_model);
	m_ui->view->setSortingEnabled(true);
	m_ui->view->expandAll();
	LevelDelegate * d = new LevelDelegate(this);
	m_ui->view->setItemDelegate(d);
	//m_ui->view->setRootIndex(m_model->indexFromItem(m_model->invisibleRootItem()));

	m_ui->view->setEditTriggers(QAbstractItemView::NoEditTriggers);
	connect(m_ui->view, SIGNAL(clicked(QModelIndex)), this, SLOT(onClicked(QModelIndex)));
	connect(m_ui->allButton, SIGNAL(clicked()), this, SLOT(onSelectAll()));
	connect(m_ui->noneButton, SIGNAL(clicked()), this, SLOT(onSelectNone()));
	m_ui->view->header()->hide();
}

void FilterLvl::destroyModel ()
{
	//if (m_ui->view->itemDelegate())
	//	m_ui->view->setItemDelegate(0);
	if (m_ui->view->model() == m_model)
		m_ui->view->setModel(0);
	delete m_model;
	m_model = 0;
}

bool FilterLvl::isPresent (QString const & item, bool & enabled, E_LevelMode & lvlmode) const
{
	for (size_t i = 0, ie = m_data.size(); i < ie; ++i)
		if (m_data.at(i).m_level_str == item)
		{
			FilteredLevel const & l = m_data.at(i);
			lvlmode = static_cast<E_LevelMode>(l.m_state);
			enabled = l.m_is_enabled;
			return true;
		}
	return false;
}
void FilterLvl::append (QString const & item)
{
	for (size_t i = 0, ie = m_data.size(); i < ie; ++i)
		if (m_data[i].m_level_str == item)
		{
			FilteredLevel & l = m_data[i];
			l.m_is_enabled = true;
			return;
		}
	m_data.push_back(FilteredLevel(item, true, e_LvlInclude));
	std::sort(m_data.begin(), m_data.end());
	m_ui->view->sortByColumn(0, Qt::AscendingOrder);
}
void FilterLvl::remove (QString const & item)
{
	for (size_t i = 0, ie = m_data.size(); i < ie; ++i)
		if (m_data[i].m_level_str == item)
		{
			FilteredLevel & l = m_data[i];
			l.m_is_enabled = false;
			return;
		}
}
bool FilterLvl::setMode (QString const & item, bool enabled, E_LevelMode lvlmode)
{
	for (size_t i = 0, ie = m_data.size(); i < ie; ++i)
		if (m_data.at(i).m_level_str == item)
		{
			FilteredLevel & l = m_data[i];
			l.m_state = lvlmode;
			l.m_is_enabled = enabled;
			return true;
		}
	return false;

}

void FilterLvl::onSelectAll ()
{
	boost::function<void (FilterLvl *, QString const &)> f = &FilterLvl::append;
	applyFnOnAllChildren(f, this, m_model, Qt::Checked);
	emitFilterChangedSignal();
}
void FilterLvl::onSelectNone ()
{
	boost::function<void (FilterLvl *, QString const &)> f = &FilterLvl::remove;
	applyFnOnAllChildren(f, this, m_model, Qt::Unchecked);
	emitFilterChangedSignal();
}

void FilterLvl::onClicked (QModelIndex idx)
{
	if (!idx.isValid()) return;
	QStandardItem * item = m_model->itemFromIndex(idx);
	Q_ASSERT(item);

	if (idx.column() == 1)
	{
		QString const & filter_item = m_model->data(m_model->index(idx.row(), 0, QModelIndex()), Qt::DisplayRole).toString();
		QString const & mod = m_model->data(idx, Qt::DisplayRole).toString();

		E_LevelMode const curr = stringToLvlMod(mod.toStdString().c_str()[0]);
		size_t const i = (curr + 1) % e_max_lvlmod_enum_value;
		E_LevelMode const new_mode = static_cast<E_LevelMode>(i);
		m_model->setData(idx, QString(lvlModToString(new_mode)));

		bool const checked = (item->checkState() == Qt::Checked);
		setMode(filter_item, !checked, new_mode);

		emitFilterChangedSignal();
	}
	else
	{
		QString const & filter_item = m_model->data(idx, Qt::DisplayRole).toString();
		bool const orig_checked = (item->checkState() == Qt::Checked);
		if (orig_checked)
			append(filter_item);
		else
			remove(filter_item);

		emitFilterChangedSignal();
	}
}

void FilterLvl::recompile ()
{ }

void FilterLvl::locateItem (QString const & item, bool scrollto, bool expand)
{
	QModelIndexList indexList = m_model->match(m_model->index(0, 0), Qt::DisplayRole, item);
	if (!indexList.empty())
	{
		QModelIndex const selectedIndex(indexList.first());
		getWidget()->setCurrentIndex(selectedIndex);
	}
}

void FilterLvl::setAppData( AppData const * appdata )
{
	static_cast<LevelDelegate *>(m_ui->view->itemDelegate())->setAppData(appdata);
}


//////// delegate
void LevelDelegate::paint (QPainter * painter, QStyleOptionViewItem const & option, QModelIndex const & index) const
{
	//if (m_app_data && m_app_data->getDictCtx().m_names.size())
	painter->save();
	QStyleOptionViewItemV4 option4 = option;
	initStyleOption(&option4, index);

	if (index.column() == 0)
	{
		QVariant value = index.data(Qt::DisplayRole);
		if (value.isValid() && !value.isNull())
		{
			QString const qs = value.toString();
			int const lvl = qs.toInt();
			if (m_app_data && lvl >= 0 && m_app_data->getDictLvl().m_names.size())
			{
				option4.text = m_app_data->getDictLvl().findNameFor(qs);
			}
			else
			{
				option4.text = qs;
			}

			if (QWidget const * widget = option4.widget)
			{
				QStyle * style = widget->style();
				style->drawControl(QStyle::CE_ItemViewItem, &option4, painter, widget);
			}
		}
	}
	else if (index.column() == 1)
	{
		QVariant value = index.data(Qt::DisplayRole);
		if (value.isValid() && !value.isNull())
		{
			QString const qs = value.toString();

			E_LevelMode const mode = stringToLvlMod(qs.at(0).toLatin1());
			QString const verbose = lvlmodsStr[mode];
			option4.text = verbose;

			if (QWidget const * widget = option4.widget)
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

