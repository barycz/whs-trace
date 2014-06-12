#include "filter_string.h"
#include "constants.h"
#include "serialize.h"
#include "utils_qstandarditem.h"
#include <boost/function.hpp>
#include <QPainter>
// serialization stuff
#include <boost/serialization/type_info_implementation.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/serialization/serialization.hpp>
#include <boost/serialization/vector.hpp>
#include <serialize/ser_qt.h>
#include <fstream>

FilterString::FilterString (QWidget * parent)
	: FilterBase(parent)
	, m_ui(new Ui_FilterString)
	, m_data()
	, m_model(0)
	, m_delegate(0)
{
	initUI();
	setupModel();
}

FilterString::~FilterString ()
{
	destroyModel();
	doneUI();
}

void FilterString::initUI ()
{
	m_ui->setupUi(this);
}

void FilterString::doneUI ()
{
}

void FilterString::CheckEnabledFilter()
{	
	m_enabledFilter.clear();
	for (auto it = m_data.begin(); it != m_data.end(); ++it)
	{
		if (it->m_is_enabled)
			m_enabledFilter.push_back(*it);
	}
}

bool FilterString::accept(const DecodedCommand& cmd) const
{	
	if (m_enabledFilter.size() == 0)
		return true;

	int acceptedFilters = 0;
	for (auto it = m_enabledFilter.begin(); it != m_enabledFilter.end(); ++it)
	{
		int excluded = 0;
		for (size_t c = 0; c < cmd.m_tvs.size(); ++c)
		{
			const QString& val = cmd.m_tvs[c].m_val;
			const auto matched = it->match(val);

			// Any column for inclusion
			if (matched && it->m_state == e_Include)
			{
				acceptedFilters++;
				break;
			}

			// Excluded from all columns
			if (!matched && it->m_state == e_Exclude)
				++excluded;
		}

		if (excluded > 0 && excluded == cmd.m_tvs.size())
			acceptedFilters++;
	}

	// Text must be accepted by all filter
	return m_enabledFilter.count() == acceptedFilters;
}

void FilterString::defaultConfig ()
{
	m_data.clear();
}

void FilterString::loadConfig (QString const & path)
{
	QString const fname = path + "/" + g_filterTag + "/" + typeName();
	if (!::loadConfigTemplate(*this, fname))
		defaultConfig();

	for (auto it = m_data.begin(); it != m_data.end(); ++it)
	{
		QStandardItem * root = m_model->invisibleRootItem();
		QStandardItem * child = findChildByText(root, it->m_string);
		if (child == 0)
		{
			QList<QStandardItem *> row_items = addTriRow(it->m_string, it->m_is_enabled ? Qt::Checked : Qt::Unchecked, it->m_state == e_Include);
			root->appendRow(row_items);
		}
	}

	CheckEnabledFilter();
	emitFilterChangedSignal();
}

void FilterString::saveConfig (QString const & path)
{
	QString const fname = path + "/" + g_filterTag + "/" + typeName();
	::saveConfigTemplate(*this, fname);
}

void FilterString::applyConfig ()
{
	FilterBase::applyConfig();
}

void FilterString::clear ()
{
	m_data.clear();
	// @TODO m_string_model.clear();
}


///////////////////
void FilterString::setupModel ()
{
	if (!m_model)
		m_model = new QStandardItemModel;
	m_ui->view->setModel(m_model);
	connect(m_ui->qFilterLineEdit, SIGNAL(returnPressed()), this, SLOT(onStringAdd()));
	m_ui->view->setEditTriggers(QAbstractItemView::NoEditTriggers);
	m_ui->view->header()->hide();
	connect(m_ui->view, SIGNAL(clicked(QModelIndex)), this, SLOT(onClickedAtStringList(QModelIndex)));
	//connect(m_ui->view, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(onDoubleClickedAtStringList(QModelIndex)));
	//connect(ui->comboBoxString, SIGNAL(activated(int)), this, SLOT(onStringActivate(int)));
	connect(m_ui->buttonAddString, SIGNAL(clicked()), this, SLOT(onStringAdd()));
	connect(m_ui->buttonRmString, SIGNAL(clicked()), this, SLOT(onStringRm()));

	m_ui->view->setItemDelegate(new StringDelegate(this));
}

void FilterString::destroyModel ()
{
	if (m_ui->view->itemDelegate())
		m_ui->view->setItemDelegate(0);
	if (m_ui->view->model() == m_model)
		m_ui->view->setModel(0);
	delete m_model;
	m_model = 0;
	delete m_delegate;
	m_delegate = 0;
}


bool FilterString::isMatchedStringExcluded (QString str) const
{
	for (int i = 0, ie = m_data.size(); i < ie; ++i)
	{
		FilteredString const & fr = m_data.at(i);
		if (fr.match(str))
		{
			if (!fr.m_is_enabled)
				return false;
			else
			{
				return fr.m_state ? false : true;
			}
		}
	}
	return false;
}
void FilterString::setStringState (QString const & s, int state)
{
	for (int i = 0, ie = m_data.size(); i < ie; ++i)
	{
		FilteredString & fr = m_data[i];
		if (fr.m_string == s)
		{
			fr.m_state = state;
		}
	}

	CheckEnabledFilter();
}
void FilterString::setStringChecked (QString const & s, bool checked)
{
	for (int i = 0, ie = m_data.size(); i < ie; ++i)
	{
		FilteredString & fr = m_data[i];
		if (fr.m_string == s)
		{
			fr.m_is_enabled = checked;
		}
	}

	CheckEnabledFilter();
}
void FilterString::removeFromStringFilters(const QString& itemText)
{
	for (int i = 0, ie = m_data.size(); i < ie; ++i)
	{
		FilteredString & fr = m_data[i];
		if (fr.m_string == itemText)
		{
			m_data.removeAt(i);
			return;
		}
	}
	CheckEnabledFilter();
	emitFilterChangedSignal();
}
void FilterString::appendToStringFilters (QString const & s, bool enabled, int state)
{
	for (int i = 0, ie = m_data.size(); i < ie; ++i)
		if (m_data[i].m_string == s)
			return;
	m_data.push_back(FilteredString(s, enabled, state));
	CheckEnabledFilter();
	emitFilterChangedSignal();
}

void FilterString::appendToStringWidgets (FilteredString const & flt)
{
	QStandardItem * root = m_model->invisibleRootItem();
	QStandardItem * child = findChildByText(root, flt.m_string);
	if (child == 0)
	{
		bool const mode = static_cast<bool>(flt.m_state);
		QList<QStandardItem *> row_items = addTriRow(flt.m_string, flt.m_is_enabled ? Qt::Checked : Qt::Unchecked, mode);
		row_items[0]->setCheckState(flt.m_is_enabled ? Qt::Checked : Qt::Unchecked);
		root->appendRow(row_items);
	}
}


void FilterString::onClickedAtStringList (QModelIndex idx)
{
	if (!idx.isValid())
		return;

	if (idx.column() == 1)
	{
		QString const & filter_item = m_model->data(m_model->index(idx.row(), 0, QModelIndex()), Qt::DisplayRole).toString();
		QString const & mod = m_model->data(idx, Qt::DisplayRole).toString();
		E_FilterMode const curr = stringToFltMod(mod.toStdString().c_str()[0]);
		size_t const i = (curr + 1) % e_max_fltmod_enum_value;
		E_FilterMode const new_mode = static_cast<E_FilterMode>(i);
		m_model->setData(idx, QString(fltModToString(new_mode)));
				
		setStringState(filter_item, new_mode);
		recompile();
		emitFilterChangedSignal();
	}
	else
	{
		QStandardItem * item = m_model->itemFromIndex(idx);
		Q_ASSERT(item);
		const bool checked = item->checkState() == Qt::Checked;
		
		QString const & mod = m_model->data(m_model->index(idx.row(), 1, QModelIndex()), Qt::DisplayRole).toString();
		E_FilterMode const curr = stringToFltMod(mod.toStdString().c_str()[0]);		
		QString const & val = m_model->data(idx, Qt::DisplayRole).toString();
		
		setStringState(val, curr);
		setStringChecked(val, checked);
		CheckEnabledFilter();
		recompile();
		emitFilterChangedSignal();
	}
}

void FilterString::recompile ()
{ }

void FilterString::onStringRm()
{	
	auto index = m_ui->view->currentIndex();
	QStandardItem* item = m_model->item(index.row(), 0);
	if (item)
	{		
		m_model->takeRow(index.row());
		removeFromStringFilters(item->text());
		recompile();
		emitFilterChangedSignal();
	}
}

void FilterString::onStringAdd()
{
	const QString qItem = m_ui->qFilterLineEdit->text();
	if (!qItem.length())
		return;

	QStandardItem * root = m_model->invisibleRootItem();
	QStandardItem * child = findChildByText(root, qItem);
	if (child == 0)
	{
		QList<QStandardItem *> row_items = addTriRow(qItem, Qt::Checked, true);
		root->appendRow(row_items);
		appendToStringFilters(qItem, true, e_Include);		
		recompile();		
	}
}

void FilterString::locateItem (QString const & item, bool scrollto, bool expand)
{
	QModelIndexList indexList = m_model->match(m_model->index(0, 0), Qt::DisplayRole, item);
	if (!indexList.empty())
	{
		QModelIndex const selectedIndex(indexList.first());
		getWidget()->setCurrentIndex(selectedIndex);
	}
}

//////// delegate
void StringDelegate::paint (QPainter * painter, QStyleOptionViewItem const & option, QModelIndex const & index) const
{
    painter->save();
    QStyleOptionViewItemV4 option4 = option;
    initStyleOption(&option4, index);

	if (index.column() == 1)
	{
		QVariant value = index.data(Qt::DisplayRole);
		if (value.isValid() && !value.isNull())
		{
			QString const qs = value.toString();

			E_FilterMode const mode = stringToFltMod(qs.at(0).toLatin1());
			QString const verbose = fltmodsStr[mode];
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





