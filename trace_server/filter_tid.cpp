#include "filter_tid.h"
#include "constants.h"
#include "serialize.h"
#include "utils_qstandarditem.h"
#include <boost/function.hpp>

FilterTid::FilterTid (QWidget * parent)
	: FilterBase(parent)
	, m_ui(new Ui_FilterTid)
  , m_model(0)
{
	initUI();
	setupModel();
}

FilterTid::~FilterTid ()
{
	destroyModel();
	doneUI();
}

void FilterTid::initUI ()
{
	m_ui->setupUi(this);
}

void FilterTid::doneUI ()
{
}

bool FilterTid::isPresent (QString const & item, bool & enabled) const
{
	for (size_t i = 0, ie = m_data.size(); i < ie; ++i)
		if (m_data.at(i).m_tid_str == item)
		{
			FilteredTid const & l = m_data.at(i);
			//mode = static_cast<E_TidMode>(l.m_state);
			enabled = l.m_is_enabled;
			return true;
		}
	return false;
}


bool FilterTid::accept (DecodedCommand const & cmd) const
{
	QString tid;
	if (!cmd.getString(tlv::tag_tid, tid))
		return true;

	bool enabled = true;
	bool const present = isPresent(tid, enabled);

	bool excluded = false;
	if (present)
	{
		//if (enabled && lvlmode == e_TidForceInclude) return true; // forced tids (errors etc)
		excluded |= !enabled;
	}
	return !excluded;
}


void FilterTid::defaultConfig ()
{
	m_data.clear();
}

void FilterTid::loadConfig (QString const & path)
{
	QString const fname = path + "/" + g_filterTag + "/" + typeName();
	if (!::loadConfigTemplate(*this, fname))
		defaultConfig();
}

void FilterTid::saveConfig (QString const & path)
{
	QString const fname = path + "/" + g_filterTag + "/" + typeName();
	::saveConfigTemplate(*this, fname);
}

void FilterTid::applyConfig ()
{
	FilterBase::applyConfig();
	//m_filter_state.merge_with(src.m_file_filters);
}

void FilterTid::clear ()
{
	m_data.clear();
	// @TODO m_tid_model.clear();
}


///////// tid filters
void FilterTid::setupModel ()
{
	if (!m_model)
		m_model = new QStandardItemModel;
	m_ui->view->setModel(m_model);

	m_ui->view->setEditTriggers(QAbstractItemView::NoEditTriggers);
	connect(m_ui->view, SIGNAL(clicked(QModelIndex)), this, SLOT(onClicked(QModelIndex)));
	connect(m_ui->allButton, SIGNAL(clicked()), this, SLOT(onSelectAll()));
	connect(m_ui->noneButton, SIGNAL(clicked()), this, SLOT(onSelectNone()));
}

void FilterTid::destroyModel ()
{
	if (m_ui->view->model() == m_model)
		m_ui->view->setModel(0);
	delete m_model;
	m_model = 0;
}

void FilterTid::append (QString const & item)
{
	for (size_t i = 0, ie = m_data.size(); i < ie; ++i)
		if (m_data[i].m_tid_str == item)
		{
			FilteredTid & l = m_data[i];
			l.m_is_enabled = true;
			return;
		}
	m_data.push_back(FilteredTid(item, true, 0));
	std::sort(m_data.begin(), m_data.end());
	m_ui->view->sortByColumn(0, Qt::AscendingOrder);

}
void FilterTid::remove (QString const & item)
{
	for (size_t i = 0, ie = m_data.size(); i < ie; ++i)
		if (m_data[i].m_tid_str == item)
		{
			FilteredTid & l = m_data[i];
			l.m_is_enabled = false;
			return;
		}
}

void FilterTid::recompile ()
{
}

void FilterTid::locateItem (QString const & item, bool scrollto, bool expand)
{
	QModelIndexList indexList = m_model->match(m_model->index(0, 0), Qt::DisplayRole, item);
	if (!indexList.empty())
	{
		QModelIndex const selectedIndex(indexList.first());
		getWidget()->setCurrentIndex(selectedIndex);
	}
}

// slots
void FilterTid::onClicked (QModelIndex idx)
{
	if (!idx.isValid()) return;
	QStandardItem * item = m_model->itemFromIndex(idx);
	Q_ASSERT(item);

    QString const & filter_item = m_model->data(idx, Qt::DisplayRole).toString();
    bool const orig_checked = (item->checkState() == Qt::Checked);
    if (orig_checked)
        append(filter_item);
    else
        remove(filter_item);

    emitFilterChangedSignal();
}

void FilterTid::onSelectAll ()
{
	boost::function<void (FilterTid *, QString const &)> f = &FilterTid::append;
	applyFnOnAllChildren(f, this, m_model, Qt::Checked);
	emitFilterChangedSignal();
}
void FilterTid::onSelectNone ()
{
	boost::function<void (FilterTid *, QString const &)> f = &FilterTid::remove;
	applyFnOnAllChildren(f, this, m_model, Qt::Unchecked);
	emitFilterChangedSignal();
}


