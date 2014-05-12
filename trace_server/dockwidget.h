#pragma once
#include <QDockWidget>

class QCloseEvent; class QMainWindow; struct DockManager;

struct DockWidget : public QDockWidget
{
	Q_OBJECT
	friend struct DockManager;
public:

	explicit DockWidget (DockManager & mgr, QString const & name, QMainWindow * const window);
	virtual ~DockWidget ();
	virtual void closeEvent (QCloseEvent * event);

	void hideEvent (QHideEvent *) { emit widgetVisibilityChanged(false); }
	void showEvent (QShowEvent *) { emit widgetVisibilityChanged(true); }

signals:
	void dockClosed (DockWidget * w);
	void widgetVisibilityChanged (bool state);

private:
	DockManager & m_mgr;
};

