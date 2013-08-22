#pragma once
#include <QString>
#include <QDockWidget>
#include <QMultiMap>
#include "treeview.h"
#include "treemodel.h"
#include "dockedwidget.h"
#include "action.h"
class QCloseEvent;
class QMainWindow;
class MainWindow;
struct DockManager;


struct DockedWidgetBase : ActionAble {

	DockedWidgetBase (QStringList const & path)
		: ActionAble(path)
		, m_idx()
		, m_dockpath(path), m_wd(0) 
	{ }
	virtual ~DockedWidgetBase () { }

	QModelIndex m_idx;
	QStringList m_dockpath;
	QDockWidget * m_wd;

	virtual E_DataWidgetType type () const = 0;

	QStringList const & dockPath () const { return m_dockpath; }
};



struct DockWidget : public QDockWidget
{
	Q_OBJECT
	friend struct DockManager;
public:

	explicit DockWidget (DockManager & mgr, QString const & name, QMainWindow * const window);
	virtual void closeEvent (QCloseEvent * event);

Q_SIGNALS:
	void dockClosed (DockWidget * w);

private:

	DockManager & m_mgr;
};

class DockTreeModel : public TreeModel<DockedInfo>
{
	Q_OBJECT
public:

	explicit DockTreeModel (QObject * parent = 0, tree_data_t * data = 0);
	~DockTreeModel ();

	QModelIndex insertItemWithPath (QStringList const & path, bool checked);

	TreeModel<DockedInfo>::node_t const * getItemFromIndex (QModelIndex const & index) const { return itemFromIndex(index); }
};


struct DockManager : QObject, ActionAble
{
	Q_OBJECT
public:
	DockManager (MainWindow * mw, QStringList const & path);
	~DockManager ();

	QMultiMap<QString, QDockWidget *> m_widgets; // @TODO: hashed container?
	MainWindow * 		m_main_window;
	DockWidget * 		m_docked_widgets;
	TreeView * 			m_docked_widgets_tree_view;
	DockTreeModel *		m_docked_widgets_model;
	typedef tree_filter<DockedInfo> data_filters_t;
	data_filters_t		m_docked_widgets_state;

	DockWidget * mkDockWidget (DockedWidgetBase & dwb, bool visible);
	DockWidget * mkDockWidget (DockedWidgetBase & dwb, bool visible, Qt::DockWidgetArea area);
	QModelIndex addDockedTreeItem (DockedWidgetBase & dwb, bool on);
	QModelIndex addLeafTreeItem (QStringList const & path, bool on);

	virtual bool handleAction (Action * a, bool sync);

public slots:
	void onWidgetClosed (DockWidget * w);
	void onClickedAtDockedWidgets (QModelIndex idx);

};


