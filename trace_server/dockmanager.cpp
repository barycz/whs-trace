#include "dock.h"
#include "mainwindow.h"
#include "connection.h"
#include "dockmanagerconfig.h"
#include "dockdelegates.h"
#include "controlbar_dockmanager.h"
#include "serialize.h"
#include <ui_controlbarcommon.h>
#include <QScrollBar>
/*#include <ui_controlbarlogs.h>
#include <ui_controlbarplots.h>
#include <ui_controlbartables.h>
#include <ui_controlbargantts.h>*/

DockManager::DockManager (MainWindow * mw, QStringList const & path)
	: DockManagerView(mw), ActionAble(path)
	, m_main_window(mw)
	, m_dockwidget(0)
	, m_control_bar(0)
	, m_model(0)
	, m_config(g_traceServerName)
{
	qDebug("%s", __FUNCTION__);
	resizeColumnToContents(0);

	QString const name = path.join("/");
	DockWidget * const dock = new DockWidget(*this, name, m_main_window);
	dock->setObjectName(name);
	dock->setWindowTitle(name);
	dock->setAllowedAreas(Qt::AllDockWidgetAreas);
	m_main_window->addDockWidget(Qt::TopDockWidgetArea, dock);
	dock->setAttribute(Qt::WA_DeleteOnClose, false);
	dock->setWidget(this);

	//if (visible)
	//	m_main_window->restoreDockWidget(dock);
	m_dockwidget = dock;

	connect(m_dockwidget, SIGNAL(widgetVisibilityChanged(bool)), m_main_window, SLOT(onDockManagerVisibilityChanged(bool)));
	m_control_bar = new ControlBarCommon();

	connect(header(), SIGNAL(sectionResized(int, int, int)), this, SLOT(onColumnResized(int, int, int)));
	connect(m_dockwidget, SIGNAL(dockClosed()), mw, SLOT(onDockManagerClosed()));
	setAllColumnsShowFocus(false);
	setExpandsOnDoubleClick(false);
	//setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
	//setSizeAdjustPolicy(QAbstractScrollArea::AdjustIgnored);
	//header()->setSectionResizeMode(0, QHeaderView::Interactive);
	header()->setStretchLastSection(false);
	setEditTriggers(QAbstractItemView::NoEditTriggers);
	//setItemDelegateForColumn(e_Column_Close, new CloseButtonDelegate(*this, this));
	setItemDelegateForColumn(e_Column_Close, new CloseButtonDelegate(*this, this));
	setItemDelegateForColumn(e_Column_ControlWidget, new ControlWidgetDelegate(*this, this));
	setStyleSheet("QTreeView::item{ selection-background-color: #FFE7BA } QTreeView::item{ selection-color: #000000 }");
	//horizontalScrollBar()->setStyleSheet("QScrollBar:horizontal { border: 1px solid grey; height: 15px; } QScrollBar::handle:horizontal { background: white; min-width: 10px; }");
	/*
	setStyleSheet(QString::fromUtf8("QScrollBar:vertical {               
		"    border: 1px solid #999999;"
		"    background:white;"
		"    width:10px;    "
		"    margin: 0px 0px 0px 0px;"
		"}"
		"QScrollBar::handle:vertical {"
		"    background: qlineargradient(x1:0, y1:0, x2:1, y2:0,"
		"    stop: 0  rgb(32, 47, 130), stop: 0.5 rgb(32, 47, 130),  stop:1 rgb(32, 47, 130));"
		"    min-height: 0px;"
		""
		"}"
		"QScrollBar::add-line:vertical {"
		"    background: qlineargradient(x1:0, y1:0, x2:1, y2:0,"
		"    stop: 0  rgb(32, 47, 130), stop: 0.5 rgb(32, 47, 130),  stop:1 rgb(32, 47, 130));"
		"    height: px;"
		"    subcontrol-position: bottom;"
		"    subcontrol-origin: margin;"
		"}"
		"QScrollBar::sub-line:vertical {"
		"    background: qlineargradient(x1:0, y1:0, x2:1, y2:0,"
		"    stop: 0  rgb(32, 47, 130), stop: 0.5 rgb(32, 47, 130),  stop:1 rgb(32, 47, 130));"
		"    height: 0px;"
		"    subcontrol-position: top;"
		"    subcontrol-origin: margin;"
		"}"
		""));
*/	
}

void DockManager::loadConfig (QString const & cfgpath)
{
	QString const fname = cfgpath + "/" + g_dockManagerTag;
	DockManagerConfig config2(g_traceServerName);
	if (!::loadConfigTemplate(config2, fname))
	{
		m_config.defaultConfig();
	}
	else
	{
		m_config = config2;
		config2.m_data.root = 0; // @TODO: promyslet.. takle na to urcite zapomenu
	}

	m_model = new DockManagerModel(*this, this, &m_config.m_data);
	setModel(m_model);
	connect(this, SIGNAL(removeCurrentIndex(QModelIndex const &)), this, SLOT(onRemoveCurrentIndex(QModelIndex const &)));
	bool const on = true;
	addActionAble(*this, on, false, true);
}

void DockManager::applyConfig ()
{
	for (size_t i = 0, ie = m_config.m_columns_sizes.size(); i < ie; ++i)
		header()->resizeSection(static_cast<int>(i), m_config.m_columns_sizes[i]);

	if (m_model)
		m_model->syncExpandState(this);
}

void DockManager::saveConfig (QString const & path)
{
	QString const fname = path + "/" + g_dockManagerTag;
	::saveConfigTemplate(m_config, fname);
}

DockManager::~DockManager ()
{
	disconnect(m_dockwidget, SIGNAL(dockClosed()), m_main_window, SLOT(onDockManagerClosed()));
	disconnect(this, SIGNAL(removeCurrentIndex(QModelIndex const &)), this, SLOT(onRemoveCurrentIndex(QModelIndex const &)));
	disconnect(m_dockwidget, SIGNAL(widgetVisibilityChanged(bool)), m_main_window, SLOT(onDockManagerVisibilityChanged(bool)));
	disconnect(header(), SIGNAL(sectionResized(int, int, int)), this, SLOT(onColumnResized(int, int, int)));

	setParent(0);
	m_dockwidget->setWidget(0);
	delete m_dockwidget;
	m_dockwidget= 0;
	removeActionAble(*this);
}

DockWidget * DockManager::mkDockWidget (DockedWidgetBase & dwb, bool visible)
{
	return mkDockWidget(dwb, visible, Qt::BottomDockWidgetArea);
}

DockWidget * DockManager::mkDockWidget (ActionAble & aa, bool visible, Qt::DockWidgetArea area)
{
	qDebug("%s", __FUNCTION__);
	Q_ASSERT(aa.path().size() > 0);

	QString const name = aa.path().join("/");
	DockWidget * const dock = new DockWidget(*this, name, m_main_window);
	dock->setObjectName(name);
	dock->setWindowTitle(name);
	dock->setAllowedAreas(Qt::AllDockWidgetAreas);
	//dock->setWidget(docked_widget); // @NOTE: commented, it is set by caller
	m_main_window->addDockWidget(area, dock);
	dock->setAttribute(Qt::WA_DeleteOnClose, false);

	if (visible)
		m_main_window->restoreDockWidget(dock);
	return dock;
}

void DockManager::onWidgetClosed (DockWidget * w)
{
	qDebug("%s w=%08x", __FUNCTION__, w);
}

QModelIndex DockManager::addActionAble (ActionAble & aa, bool on, bool close_button, bool control_widget)
{
	qDebug("%s aa=%s show=%i", __FUNCTION__, aa.joinedPath().toStdString().c_str(), on);
	QModelIndex const idx = m_model->insertItemWithPath(aa.path(), on);
	QString const & name = aa.joinedPath();
	m_actionables.insert(name, &aa);
	m_model->initData(idx, QVariant(on ? Qt::Checked : Qt::Unchecked), Qt::CheckStateRole);

	if (close_button)
	{
		QModelIndex const jdx = m_model->index(idx.row(), e_Column_Close, idx.parent());
		openPersistentEditor(jdx);
	}
	if (control_widget)
	{
		QModelIndex const kdx = m_model->index(idx.row(), e_Column_ControlWidget, idx.parent());
		openPersistentEditor(kdx);
	}
	return idx;

}

QModelIndex DockManager::addActionAble (ActionAble & aa, bool on)
{
	return addActionAble(aa, on, true, true);
}

ActionAble const * DockManager::findActionAble (QString const & dst_joined) const
{
	actionables_t::const_iterator it = m_actionables.find(dst_joined);
	if (it != m_actionables.end() && it.key() == dst_joined)
		return *it;
	return 0;
}
ActionAble * DockManager::findActionAble (QString const & dst_joined)
{
	actionables_t::iterator it = m_actionables.find(dst_joined);
	if (it != m_actionables.end() && it.key() == dst_joined)
		return *it;
	return 0;
}

void DockManager::removeActionAble (ActionAble & aa)
{
	qDebug("%s aa=%s", __FUNCTION__, aa.joinedPath().toStdString().c_str());
	actionables_t::iterator it = m_actionables.find(aa.joinedPath());
	if (it != m_actionables.end() && it.key() == aa.joinedPath())
	{
		QModelIndex const idx = m_model->testItemWithPath(aa.path());
		if (idx.isValid())
			for (int j = 1; j < e_max_dockmgr_column; ++j)
				closePersistentEditor(m_model->index(idx.row(), j, idx.parent()));
		m_actionables.erase(it);
	}
}

void DockManager::onColumnResized (int idx, int , int new_size)
{
	if (idx < 0) return;
	size_t const curr_sz = m_config.m_columns_sizes.size();
	if (idx < curr_sz)
	{
		//qDebug("%s this=0x%08x hsize[%i]=%i", __FUNCTION__, this, idx, new_size);
	}
	else
	{
		m_config.m_columns_sizes.resize(idx + 1);
		for (size_t i = curr_sz; i < idx + 1; ++i)
			m_config.m_columns_sizes[i] = 32;
	}
	m_config.m_columns_sizes[idx] = new_size;
}

bool DockManager::handleAction (Action * a, E_ActionHandleType sync)
{
	QStringList const & src_path = a->m_src_path;
	QStringList const & dst_path = a->m_dst_path;
	QStringList const & my_addr = path();

	if (dst_path.size() == 0)
	{
		qWarning("DockManager::handleAction empty dst");
		return false;
	}

	Q_ASSERT(my_addr.size() == 1);
	int const lvl = dst_path.indexOf(my_addr.at(0), a->m_dst_curr_level);
	if (lvl == -1)
	{
		qWarning("DockManager::handleAction message not for me");
		return false;
	}
	else if (lvl == dst_path.size() - 1)
	{
		// message just for me! gr8!
		a->m_dst_curr_level = lvl;
		a->m_dst = this;
		return true;
	}
	else
	{
		// message for my children
		a->m_dst_curr_level = lvl + 1;

		if (a->m_dst_curr_level >= 0 && a->m_dst_curr_level < dst_path.size())
		{
			QString const dst_joined = dst_path.join("/");
			actionables_t::iterator it = m_actionables.find(dst_joined);
			if (it != m_actionables.end() && it.key() == dst_joined)
			{
				qDebug("delivering action to: %s", dst_joined.toStdString().c_str());
				ActionAble * const next_hop =  it.value();
				next_hop->handleAction(a, sync); //@NOTE: e_Close can invalidate iterator
			}
			else
			{
				//@TODO: find least common subpath
			}
		}
		else
		{
			qWarning("DockManager::handleAction hmm? what?");
		}
	}
	return false;
}

void DockManager::onCloseButton ()
{
	QVariant v = QObject::sender()->property("idx");
	if (v.canConvert<QModelIndex>())
	{
		QModelIndex const idx = v.value<QModelIndex>();
		if (TreeModel<DockedInfo>::node_t * const n = m_model->getItemFromIndex(idx))
		{
			QStringList const & dst_path = n->data.m_path;
			Action a;
			a.m_type = e_Close;
			a.m_src_path = path();
			a.m_src = this;
			a.m_dst_path = dst_path;
			handleAction(&a, e_Sync);
		}
	}
}


void DockManager::onRemoveCurrentIndex (QModelIndex const & idx)
{
	//model()->removeRows(idx.row(), 1, idx.parent());
}
