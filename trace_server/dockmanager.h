#pragma once
#include <QString>
#include <QMultiMap>
#include "action.h"
#include "dockmanagerview.h"
#include "dockedconfig.h"
#include "dockmanagermodel.h"
#include "dockmanagerconfig.h"
#include "controlbarcommon.h"
class MainWindow;
struct DockWidget;
namespace Ui { class ControlBarCommon; }

struct DockManager : DockManagerView, ActionAble
{
	Q_OBJECT
public:

	DockManager (MainWindow * mw, QStringList const & path);
	~DockManager ();

	enum {
		e_Column_Name,
		e_Column_Close,
		e_Column_ControlWidget,
		e_max_dockmgr_column
	};

	typedef DockManagerModel::node_t node_t;
	typedef QMultiMap<QString, ActionAble *> actionables_t;
	actionables_t		m_actionables;
	MainWindow * 		m_main_window;
	DockWidget * 		m_dockwidget;
	ControlBarCommon * 	m_control_bar;
	DockManagerModel *	m_model;
	DockManagerConfig	m_config;

	DockWidget * mkDockWidget (DockedWidgetBase & dwb, bool visible);
	DockWidget * mkDockWidget (ActionAble & aa, bool visible, Qt::DockWidgetArea area);

	QModelIndex addActionAble (ActionAble & aa, bool on);
	QModelIndex addActionAble (ActionAble & aa, bool on, bool close_button, bool control_widget);
	void removeActionAble (ActionAble & aa);
	ActionAble const * findActionAble (QString const & dst_joined) const;
	ActionAble * findActionAble (QString const & dst_joined);
	Ui::ControlBarCommon * controlUI () { return m_control_bar->ui; }
	Ui::ControlBarCommon const * controlUI () const { return m_control_bar->ui; }

	void loadConfig (QString const & path);
	void saveConfig (QString const & path);
	void applyConfig ();

	virtual bool handleAction (Action * a, E_ActionHandleType sync);
	virtual QWidget * controlWidget () { return m_control_bar; }

public slots:
	void onWidgetClosed (DockWidget * w);
	void onCloseButton ();

protected slots:
	void onColumnResized (int column, int oldSize, int newSize);
	void onRemoveCurrentIndex (QModelIndex const & idx);

protected:
};


