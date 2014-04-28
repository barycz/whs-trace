#pragma once
#include <QDockWidget>
#include "action.h"
#include "types.h"
#include "dockedconfig.h"

struct DockWidget : public QDockWidget
{
	Q_OBJECT
	friend struct DockManager;
public:

	explicit DockWidget (DockManager & mgr, QString const & name, QMainWindow * const window);
	virtual ~DockWidget ();
	virtual void closeEvent (QCloseEvent * event);

Q_SIGNALS:
	void dockClosed (DockWidget * w);

private:
	DockManager & m_mgr;
};


class QCloseEvent; class QMainWindow; struct DockManager;

struct DockedWidgetBase : ActionAble {
	//Q_OBJECT
public:
	DockWidget * m_dockwidget;

	DockedWidgetBase (QStringList const & path);
	virtual ~DockedWidgetBase ();

	virtual E_DataWidgetType type () const = 0;
	virtual QWidget * controlWidget () = 0;

	void setDockWidget (DockWidget * w) { m_dockwidget = w; }
	DockWidget * dockWidget () { return m_dockwidget; }
	DockWidget const * dockWidget () const { return m_dockwidget; }
	//virtual DockedConfigBase const & dockedConfig () const = 0;
	//virtual DockedConfigBase & dockedConfig () = 0;
	//virtual QWidget * dockedWidget () = 0;
};


