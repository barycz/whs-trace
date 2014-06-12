#pragma once
#include <qwidget.h>
#include <QColorDialog>
#include <qwt/qwt_plot.h>
#include <qwt/qwt_plot_canvas.h>
#include <qwt/qwt_plot_layout.h>
#include <qwt/qwt_symbol.h>
#include <cmd.h>
#include "action.h"
#include "curves.h"
#include "plotctxmenu.h"
#include "plottypes.h"
#include "dock.h"

class QwtPlotCurve;
class QwtPlotMarker;
class QwtLegend;

namespace plot {

	class PlotWidget : public QwtPlot, public DockedWidgetBase
	{
		Q_OBJECT
	public:
		enum { e_type = e_data_plot };
		typedef QMap<QString, Curve *> curves_t;

		PlotWidget (Connection * conn, PlotConfig const & cfg, QString const & fname, QStringList const & path);
		virtual ~PlotWidget ();

		virtual E_DataWidgetType type () const { return e_data_plot; }
		virtual QWidget * controlWidget () { return 0; }
		PlotConfig & config () { return m_config; }
		PlotConfig const & config () const { return m_config; }
		void loadConfig (QString const & path);
		void saveConfig (QString const & path);
		void loadAuxConfigs ();
		void saveAuxConfigs ();
		void applyConfig ();
		QString getCurrentWidgetPath () const;
		void exportStorageToCSV (QString const & filename) { } //TODO

		FilterMgr * filterMgr () { return m_config_ui.m_ui->widget; }
		FilterMgr const * filterMgr () const { return m_config_ui.m_ui->widget; }

		QList<DecodedCommand> m_queue;
		void handleCommand (DecodedCommand const & cmd, E_ReceiveMode mode);
		void commitCommands (E_ReceiveMode mode);

		virtual bool handleAction (Action * a, E_ActionHandleType sync);
		virtual void setVisible (bool visible);

		void stopUpdate ();

		virtual QWidget * widget() { return this; }

	protected:
		void applyAxis (AxisConfig const & acfg);
		void applyConfig (PlotConfig const & pcfg);

		PlotConfig & getConfig () { return m_config; }
		PlotConfig const & getConfig () const { return m_config; }
		void setConfigValuesToUI (PlotConfig const & cfg);
		void setUIValuesToConfig (PlotConfig & cfg);
		Curve * findCurve (QString const & subtag);
		Curve * findOrCreateCurve (QString const & subtag);
		curves_t::iterator mkCurve (QString const & subtag);

		bool handleDataXYCommand (DecodedCommand const & cmd);
		bool handlePlotClearCommand (DecodedCommand const & cmd);

		void clearAllData ();
		void clearCurveData (QString const & subtag);

		void timerEvent (QTimerEvent * e);
		virtual void update ();
		QColor allocColor ();

	public Q_SLOTS:
		void showCurve (QwtPlotItem * item, bool on);

		void onShow ();
		void onHide ();
		void onHideContextMenu ();
		void onShowContextMenu (QPoint const & pos);
		void onXAutoScaleChanged (int state);
		void onYAutoScaleChanged (int state);
		void onZAutoScaleChanged (int state);
		void onSaveButton ();
		void onApplyButton ();
		void onResetButton ();
		void onDefaultButton ();
		void onCurveActivate (int idx);
		void onClearAllDataButton ();
		void onClearCurveDataButton ();

	protected:
		Connection * m_connection;
		curves_t m_curves;
		int m_timer;
		PlotConfig m_config;
		plot::CtxPlotConfig m_config_ui;
		QList<QColor> m_colors;
		QString m_fname;
		QwtLegend * m_legend;
		//std::vector<QwtPlotMarker *> m_markers;
	};
}

