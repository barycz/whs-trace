#include "filtermgr.h"
#include <algorithm>
#include <QGridLayout>
#include <QComboBox>
#include <QModelIndexList>
#include "ui_combolist.h"
#include "serialize.h"
// serialization stuff

FilterBase * FilterMgr::filterFactory (E_FilterType t, QWidget * parent)
{
	switch (t)
	{
		case e_Filter_Mgr: return new FilterMgr (parent);
		case e_Filter_Script: return new FilterScript (parent);
		case e_Filter_String: return new FilterString (parent);
		case e_Filter_Regex: return new FilterRegex (parent);
		case e_Filter_Ctx: return new FilterCtx (parent);
		case e_Filter_Lvl: return new FilterLvl (parent);
		case e_Filter_Tid: return new FilterTid (parent);
		case e_Filter_FileLine: return new FilterFileLine (parent);
		case e_Filter_Row: return new FilterRow (parent);
		case e_Filter_Time: return new FilterTime (parent);
		//case e_Filter_Function: return new FilterFunction (parent);
		//case e_Filter_dt: return new Filterdt (parent);
		default: return 0;
	}
}


/////////////////// FILTER MGR ///////////////////////////////

FilterMgr::FilterMgr (QWidget * parent)
	: FilterMgrBase(parent)
{
	m_cache.resize(e_filtertype_max_value);
	initUI();
}

FilterMgr::~FilterMgr ()
{
	qDebug("%s", __FUNCTION__);
}

void FilterMgr::addFilter (FilterBase * b)
{
	E_FilterType const t = b->type();
	FilterMgrBase::addFilter(b);
	m_cache[t] = b;
}
void FilterMgr::rmFilter (FilterBase * & b)
{
	E_FilterType const t = b->type();
	FilterMgrBase::rmFilter(b);
	m_cache[t] = 0;
}
void FilterMgr::defaultConfig ()
{
	m_filter_order.clear();
	// @TODO clear others? probably no
	m_filter_order.push_back(g_filterNames[e_Filter_String]);
	m_filter_order.push_back(g_filterNames[e_Filter_Ctx]);
	m_filter_order.push_back(g_filterNames[e_Filter_Lvl]);
	m_filter_order.push_back(g_filterNames[e_Filter_FileLine]);
	m_filter_order.push_back(g_filterNames[e_Filter_Row]);
}

void FilterMgr::fillComboBoxWithFilters (QComboBox * cbx)
{
	for (int i = 0; i < e_Colorizer_Mgr; ++i)
		cbx->addItem(g_filterNames[i]);
}

void FilterMgr::recreateFilters ()
{
	m_filters.clear();
	m_filters.reserve(m_filter_order.size());
	//m_cache.clear();
	m_cache.resize(e_filtertype_max_value);

#define TRACE_INSANE_FILTER_TAB_BUTTONS 0

#if TRACE_INSANE_FILTER_TAB_BUTTONS
	for (int i = 0, ie = m_tabFilters->tabBar()->count(); i < ie; ++i)
	{
		m_tabFilters->tabBar()->setTabButton(i, QTabBar::LeftSide, 0);
	}
#endif

	m_tabFilters->clear();
	for (int i = 0, ie = m_filter_order.size(); i < ie; ++i)
	{
		E_FilterType const t = filterName2Type(m_filter_order[i]);
		FilterBase * fb = 0;
		if (m_cache[t] != 0)
		{
			fb = m_cache[t];
		}
		else
		{
			fb = filterFactory(t, 0);
			connect(fb, SIGNAL(filterEnabledChanged()), this, SLOT(onFilterEnabledChanged()));
			m_cache[t] = fb;
		}
		m_filters.push_back(fb);
		m_tabFilters->insertTab(i, fb, fb->typeName());
#if TRACE_INSANE_FILTER_TAB_BUTTONS
		m_tabFilters->tabBar()->setTabButton(i, QTabBar::LeftSide, fb->m_button);
#endif
	}

	for (size_t c = 0, ce = m_cache.size(); c < ce; ++c)
	{
		if (m_cache[c] == 0)
			continue;

		bool used = 0;
		for (int i = 0, ie = m_filters.size(); i < ie; ++i)
			if (m_cache[c] && m_cache[c] == m_filters[i])
			{
				used |= 1;
				break;
			}
		if (used)
			continue;

		delete m_cache[c];
		m_cache[c] = 0;
	}
}

/////////////////////////////////////////////////////////////////////

namespace {

	struct ComboBoxDelegate : public QStyledItemDelegate
	{
		FilterMgrBase * m_filter;
		QComboBox mutable * m_cbx;
		ComboBoxDelegate (QObject * parent = 0, FilterMgrBase * f = 0);

		QWidget * createEditor (QWidget * parent, QStyleOptionViewItem const & option, QModelIndex const & index) const;
		void setEditorData (QWidget * editor, QModelIndex const & index) const;
		void setModelData (QWidget * editor, QAbstractItemModel * model, QModelIndex const & index) const;
		void updateEditorGeometry (QWidget * editor, QStyleOptionViewItem const & option, QModelIndex const & index) const;
		//Q_OBJECT
		QSize sizeHint (QStyleOptionViewItem const & option, QModelIndex const & index) const
		{
			return QSize(64, 25);
		}
	};

	ComboBoxDelegate::ComboBoxDelegate (QObject * parent, FilterMgrBase * f)
		: QStyledItemDelegate(parent)
		, m_filter(f)
		, m_cbx(0)
		
	{
	}

	QWidget * ComboBoxDelegate::createEditor (QWidget * parent, QStyleOptionViewItem const & /* option */, QModelIndex const & /* index */) const
	{
		QComboBox * const editor = new QComboBox(parent);
		m_filter->fillComboBoxWithFilters(editor);
		return editor;
	}

	void ComboBoxDelegate::setEditorData (QWidget * editor, QModelIndex const & index) const
	{
		QString const value = index.model()->data(index, Qt::EditRole).toString();
		QComboBox * const cbx = static_cast<QComboBox *>(editor);
		cbx->setCurrentIndex(cbx->findText(value));
	}

	void ComboBoxDelegate::setModelData (QWidget * editor, QAbstractItemModel *model, QModelIndex const & index) const
	{
		QComboBox * cbx = static_cast<QComboBox *>(editor);
		QString value = cbx->currentText();
		model->setData(index, value, Qt::EditRole);
	}

	void ComboBoxDelegate::updateEditorGeometry(QWidget *editor, QStyleOptionViewItem const & option, QModelIndex const & /* index */) const
	{
		editor->setGeometry(option.rect);
	}
}

void FilterMgr::initUI ()
{
	setObjectName(QStringLiteral("FilterWidget"));
	//m_widget->setObjectName(QStringLiteral("FilterWidget"));

	resize(380, 305);
	QGridLayout * gridLayout = new QGridLayout(this);
	gridLayout->setSpacing(0);
	gridLayout->setContentsMargins(0, 0, 0, 0);
	gridLayout->setObjectName(QStringLiteral("gridLayout"));

	m_tabFilters = new MovableTabWidget(this);
	m_tabFilters->setObjectName(QStringLiteral("tabFilters"));
	m_tabFilters->setEnabled(true);
	m_tabFilters->setMinimumSize(QSize(128, 0));
	m_tabFilters->setMovable(true);
	QFont font;
	font.setFamily(QStringLiteral("Verdana"));
	font.setPointSize(7);
	m_tabFilters->setFont(font);
	m_tabFilters->setLayoutDirection(Qt::LeftToRight);
	m_tabFilters->setTabPosition(QTabWidget::North);
	m_tabFilters->setUsesScrollButtons(true);
	connect(m_tabFilters, SIGNAL(tabMovedSignal(int, int)), this, SLOT(onTabMoved(int, int)));

	gridLayout->addWidget(m_tabFilters, 0, 0, 1, 1);
	setLayout(gridLayout);
	//m_widget = m_widget;

	m_tabCtxMenu = new ComboList();
	m_tabCtxModel = new MyListModel(this);
	m_tabCtxModel->m_flags |= Qt::ItemIsEditable;
	m_tabCtxMenu->ui->filterView->setModel(m_tabCtxModel);
	m_tabCtxMenu->ui->filterView->setResizeMode(QListView::Adjust);
	m_tabCtxMenu->ui->filterView->setDropIndicatorShown(true);
	m_tabCtxMenu->ui->filterView->setMovement(QListView::Snap);
	m_tabCtxMenu->ui->filterView->setDragDropMode(QAbstractItemView::InternalMove);
	//m_tabCtxMenu->ui->filterView->setEditTriggers(QAbstractItemView::AllEditTriggers);
	m_tabCtxMenu->ui->filterView->setEditTriggers(QAbstractItemView::CurrentChanged);
	m_delegate = new ComboBoxDelegate(m_tabCtxMenu, this);
	m_tabCtxMenu->ui->filterView->setItemDelegate(m_delegate);
	m_tabCtxMenu->setVisible(0);

	connect(m_tabCtxMenu->ui->addButton, SIGNAL(clicked()), this, SLOT(onCtxAddButton()));
	connect(m_tabCtxMenu->ui->rmButton, SIGNAL(clicked()), this, SLOT(onCtxRmButton()));
	connect(m_tabCtxMenu->ui->commitButton, SIGNAL(clicked()), this, SLOT(onCtxCommitButton()));

	setContextMenuPolicy(Qt::CustomContextMenu);
	connect(this, SIGNAL(customContextMenuRequested(QPoint const &)), this, SLOT(onShowContextMenu(QPoint const &)));
}

void FilterMgr::doneUI ()
{
	disconnect(this, SIGNAL(customContextMenuRequested(QPoint const &)), this, SLOT(onShowContextMenu(QPoint const &)));
	disconnect(m_tabCtxMenu->ui->addButton, SIGNAL(clicked()), this, SLOT(onCtxAddButton()));
	disconnect(m_tabCtxMenu->ui->rmButton, SIGNAL(clicked()), this, SLOT(onCtxRmButton()));
}

void FilterMgr::clear ()
{
}

FilterBase * FilterMgr::mkFilter (E_FilterType t)
{
	if (FilterBase * existing_filter = m_cache[t])
		return existing_filter;

	QString const & name = g_filterNames[t];
	m_filter_order.push_back(name);
	recreateFilters();

	FilterBase * b = m_cache[t];
	if (b)
	{
		b->applyConfig();
	}
	return b;
}

