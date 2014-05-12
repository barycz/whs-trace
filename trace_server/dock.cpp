#include "dock.h"
#include <QCloseEvent>
#include <QMainWindow>
#include "mainwindow.h"
#include "dockwidget.h"
#include <QApplication>

DockedWidgetBase::DockedWidgetBase (MainWindow * mw, QStringList const & path)
	: ActionAble(path)
	, m_main_window(mw)
	, m_dockwidget(0)
{
	qDebug("%s this=0x%08x", __FUNCTION__, this);
}

DockedWidgetBase::~DockedWidgetBase ()
{
	qDebug("%s this=0x%08x", __FUNCTION__, this);
	m_main_window->dockManager().removeActionAble(*this);
	m_dockwidget->setWidget(0);
	delete m_dockwidget;
	m_dockwidget = 0;
}

