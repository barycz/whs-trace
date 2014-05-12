#pragma once
#include <QString>
#include <QDockWidget>
#include <QMultiMap>
#include <QSpinBox>
#include "action.h"
#include "dockedconfig.h"
#include "dockmanagerview.h"
#include "dockmanagermodel.h"
#include "dockmanagerconfig.h"
#include "controlbarcommon.h"
#include <QStyledItemDelegate>
class QCloseEvent; class QPushButton;
class MainWindow;
struct DockWidget;
class ControlBarCommon;
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

	typedef QMultiMap<QString, ActionAble *> actionables_t;
	actionables_t		m_actionables;
	MainWindow * 		m_main_window;
	QDockWidget * 		m_dockwidget;
	ControlBarCommon * m_control_bar;
	DockManagerModel *	m_model;
	DockManagerConfig	m_config;

	DockWidget * mkDockWidget (DockedWidgetBase & dwb, bool visible);
	DockWidget * mkDockWidget (ActionAble & aa, bool visible, Qt::DockWidgetArea area);

	QModelIndex addActionAble (ActionAble & aa, bool on);
	void removeActionAble (ActionAble & aa);
	ActionAble const * findActionAble (QString const & dst_joined) const;
	Ui::ControlBarCommon * controlUI () { return m_control_bar->ui; }
	Ui::ControlBarCommon const * controlUI () const { return m_control_bar->ui; }

	void loadConfig (QString const & path);
	void saveConfig (QString const & path);
	void applyConfig ();

	virtual bool handleAction (Action * a, E_ActionHandleType sync);
	virtual QWidget * controlWidget () { return m_control_bar; }

public slots:
	void onWidgetClosed (DockWidget * w);
	void onClicked (QModelIndex idx);
	void onCloseButton ();

protected slots:
	void onColumnResized (int column, int oldSize, int newSize);

protected:
	bool findClickedActionAble (QPushButton const * const b, TreeModel<DockedInfo>::node_t const * node, QStringList & aa) const;
};

class ButtonColumnDelegate : public QStyledItemDelegate
{
    Q_OBJECT
 
public:
 
    explicit ButtonColumnDelegate(DockManager & mgr, QObject *parent = 0);
    ~ButtonColumnDelegate();
 
    QWidget * createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    void setEditorData(QWidget *editor, const QModelIndex &index) const;
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const;
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const;
 
public slots:
    void cellEntered(const QModelIndex &index);
 
private:
    DockManager & m_dock_mgr;
    QPushButton * btn;
    bool isOneCellInEditMode;
    QPersistentModelIndex currentEditedCellIndex;
};
