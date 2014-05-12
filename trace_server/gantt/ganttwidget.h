#pragma once
#include <QWidget>
#include <QDockWidget>
#include <QFrame>
#include "ganttconfig.h"
#include "ganttdata.h"
#include "syncwidgets.h"
//#include "ganttctxmenu.h"
//#include "frameview.h"
#include "action.h"
#include "types.h"
#include "dock.h"
#include "cmd.h"
#include "filtermgr.h"

class Connection;
QT_FORWARD_DECLARE_CLASS(QSplitter)
QT_FORWARD_DECLARE_CLASS(FilterWidget)

namespace gantt {
	QT_FORWARD_DECLARE_CLASS(GanttView)
	QT_FORWARD_DECLARE_CLASS(FrameView)
	struct CtxGanttConfig;
}

namespace gantt {

	class GanttWidget : public QFrame, public DockedWidgetBase
	{
		Q_OBJECT
	public:
		enum { e_type = e_data_gantt };
		GanttWidget (Connection * conn, GanttConfig const & cfg, QString const & fname, QStringList const & path);

		virtual QWidget * controlWidget () { return 0; }
		virtual E_DataWidgetType type () const { return e_data_gantt; }
		GanttConfig & config () { return m_config; }
		GanttConfig const & config () const { return m_config; }
		void loadConfig (QString const & path);
		void saveConfig (QString const & path);
		void applyConfig ();
		void exportStorageToCSV (QString const & filename) { }

		void commitCommands (E_ReceiveMode mode);
		QList<DecodedCommand> m_queue;

		void applyConfig (GanttConfig & pcfg);
		virtual ~GanttWidget ();

		GanttConfig & getConfig () { return m_config; }
		GanttConfig const & getConfig () const { return m_config; }

		typedef QMap<QString, GanttView *> ganttviews_t;
		GanttView * findGanttView (QString const & subtag);
		GanttView * findOrCreateGanttView (QString const & subtag);
		ganttviews_t::iterator mkGanttView (QString const & subtag);
		void syncGanttViews (GanttView * src, QPointF interval);
		virtual bool handleAction (Action * a, E_ActionHandleType sync);
		virtual void setVisible (bool visible);
		void handleCommand (DecodedCommand const & cmd, E_ReceiveMode mode);

		FrameView * findOrCreateFrameView (int sync_group);

		FilterMgr * filterMgr ();
		FilterMgr const * filterMgr () const;


		//void appendGantt (QString const & time, QString const & tid, QString const & fgc, QString const & bgc, QString const & msg);
		void appendFrameEnd (DecodedData & data);

		//void findNearestTimeRow (unsigned long long t);
		//void requestWheelEventSync (QWheelEvent * ev, QGanttView const * source);
		//void requestActionSync (unsigned long long t, int cursorAction, Qt::KeyboardModifiers modifiers, QGanttView const * source);

	protected:
		//virtual void wheelEvent (QWheelEvent * event);
		//virtual QModelIndex	moveCursor (CursorAction cursorAction, Qt::KeyboardModifiers modifiers);

	public Q_SLOTS:

		void showGanttView (GanttView * item, bool on);

		void onShow ();
		void onHide ();
		void onHideContextMenu ();
		void onShowContextMenu (QPoint const & pos);
		void setConfigValuesToUI (GanttConfig const & cfg);
		void setUIValuesToConfig (GanttConfig & cfg);
		void setViewConfigValuesToUI (GanttViewConfig const & gvcfg);
		void setUIValuesToViewConfig (GanttViewConfig & cfg);
		void onSaveButton ();
		void onApplyButton ();
		void onResetButton ();
		void onDefaultButton ();
		void onFitAllButton ();
		void onFitFrameButton ();
		void onPrevFrameButton ();
		void onNextFrameButton ();
		void onFrameValueChanged (int);
		//void scrollTo (QModelIndex const & index, ScrollHint hint);
		
		void onGanttViewActivate (int idx);
		void onClearAllDataButton ();
		void onClearGanttViewDataButton ();

		void performSynchronization (E_SyncMode mode, int sync_group, unsigned long long time, void * source);

	signals:
		void requestSynchronization (E_SyncMode mode, int sync_group, unsigned long long time, void * source);

	protected:
		GanttConfig m_config;
		gantt::CtxGanttConfig * m_config_ui;
		QString m_fname;
		Connection * m_connection;
		QSplitter * m_layout;
		ganttviews_t m_ganttviews;
		QVector<QString> m_subtags;
	};
}

