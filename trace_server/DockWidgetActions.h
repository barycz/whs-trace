// Copyright (c) 2014 WarhorseStudios
// author: Tomas Barak

#pragma once

#include <QAction>

class MainWindow;
struct DockedWidgetBase;

class DockWidgetAction: public QAction
{
	Q_OBJECT
	DockWidgetAction(const DockWidgetAction&);
public:
	DockWidgetAction(DockedWidgetBase * dw, MainWindow * mainWin, const QString & text = "");

	DockedWidgetBase * dockWidget() { return m_dock_widget; }
	const DockedWidgetBase * dockWidget() const { return m_dock_widget; }

	MainWindow * mainWindow() { return m_main_window; }
	const MainWindow * mainWindow() const { return m_main_window; }

protected slots:
	void onTargetDestroyed();

protected:
	DockedWidgetBase * const m_dock_widget;
	MainWindow * const m_main_window;
};

class RestoreDockWidgetAction: public DockWidgetAction
{
	Q_OBJECT
public:
	RestoreDockWidgetAction(DockedWidgetBase * dw, MainWindow * mainWin);

private slots:
	void onTriggered();
};
