#pragma once
#include <QDockWidget>
#include "action.h"
#include "types.h"
#include "dockedconfig.h"

class QCloseEvent; class QMainWindow; struct DockManager; class MainWindow; struct DockWidget;

struct DockedWidgetBase : ActionAble {
	//Q_OBJECT
public:
	MainWindow * m_main_window;
	DockWidget * m_dockwidget;

	DockedWidgetBase (MainWindow * mw, QStringList const & path);
	virtual ~DockedWidgetBase ();

	virtual E_DataWidgetType type () const = 0;
	virtual QWidget * controlWidget () = 0;

	void setDockWidget (DockWidget * w) { m_dockwidget = w; }
	DockWidget * dockWidget () { return m_dockwidget; }
	DockWidget const * dockWidget () const { return m_dockwidget; }

	// TODO: this is a workaround to be able to have a QWidget-compatible this
	// delete the method after DockedWidgetBase class is rewritten to derive from QWidget (or QFrame)
	virtual QWidget * widget() = 0;
};


