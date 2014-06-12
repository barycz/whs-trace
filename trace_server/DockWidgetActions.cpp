// Copyright (c) 2014 WarhorseStudios
// author: Tomas Barak

#include "DockWidgetActions.h"
#include "mainwindow.h"
#include "dock.h"
#include "dockwidget.h"

DockWidgetAction::DockWidgetAction(DockedWidgetBase * dw, MainWindow * mainWin, const QString & text):
	QAction(text, mainWin), m_dock_widget(dw), m_main_window(mainWin)
{
	Q_ASSERT(dw);
	Q_ASSERT(mainWin);
	connect(dw->widget(), SIGNAL(destroyed()), this, SLOT(onTargetDestroyed()));
}

void DockWidgetAction::onTargetDestroyed()
{
	mainWindow()->rmWindowAction(this);
	deleteLater();
}

RestoreDockWidgetAction::RestoreDockWidgetAction(DockedWidgetBase * dw, MainWindow * mainWin):
	DockWidgetAction(dw, mainWin, "")
{
	setText(tr("Data Widget ") + dw->joinedPath());
	connect(this, SIGNAL(triggered()), this, SLOT(onTriggered()));
}

void RestoreDockWidgetAction::onTriggered()
{
	QDockWidget * dock = dockWidget()->dockWidget();
	QWidget * content = dockWidget()->widget();
	Q_ASSERT(dock);
	Q_ASSERT(content);
	content->setVisible(true);
	dock->setWidget(content);
	mainWindow()->restoreDockWidget(dock);
}
