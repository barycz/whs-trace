#include "plotwidget.h"
#include "connection.h"
#include "mainwindow.h"
#include <qwt/qwt_plot_legenditem.h>
#include <qwt/qwt_legend.h>
#include <qwt/qwt_plot_panner.h>
#include <qwt/qwt_plot_zoomer.h>
#include <qwt/qwt_plot_magnifier.h>
#include <qwt/qwt_picker_machine.h>
#include <qwt/qwt_plot_marker.h>
#include <constants.h>
#include <serialize.h>
#include <QTimer>

namespace plot {

	PlotWidget::curves_t::iterator PlotWidget::mkCurve (QString const & subtag)
	{
		CurveConfig * cc = 0;
		bool found = false;
		for (size_t i = 0, ie = m_config.m_ccfg.size(); i < ie; ++i)
			if (m_config.m_ccfg[i].m_tag == subtag)
			{
				cc = &m_config.m_ccfg[i];
				found = true;
			}
		if (!found)
		{
			CurveConfig cc2;
			cc2.m_tag = subtag;
			cc2.m_label = subtag;
			cc2.m_color = allocColor();
			m_config.m_ccfg.push_back(cc2);
			cc = &m_config.m_ccfg.back();
		}

		QStringList p = path();
		p << subtag;
		Curve * curve = new Curve(*cc, p);
		curve->m_curve = new QwtPlotCurve(subtag);
		curve->m_data = new Data(m_config.m_history_ln);
		curve->m_curve->attach(this);
		// if (enabled)
		return m_curves.insert(subtag, curve);
	}

	QColor PlotWidget::allocColor () {
		QColor c = m_colors.front();
		m_colors.pop_front();
		return c;
	}

	PlotWidget::PlotWidget (Connection * conn, PlotConfig const & cfg, QString const & fname, QStringList const & path)
		: QwtPlot(0), DockedWidgetBase(conn->getMainWindow(), path)
		, m_connection(conn)
		, m_config(cfg)
		, m_config_ui(m_config, this)
		, m_curves()
		, m_timer(-1)
		, m_fname(fname)
		, m_legend(0)
	{
		qDebug("%s this=0x%08x", __FUNCTION__, this);
		setAutoReplot(false);
		//canvas()->setBorderRadius(0);
		plotLayout()->setAlignCanvasToScales(true);

		setContentsMargins(QMargins(0, 0, 0, 0));
		setMinimumSize(64,64);
		m_legend = new QwtLegend(this);
		if (m_config.m_legend_show)
		{
			insertLegend(m_legend, static_cast<QwtPlot::LegendPosition>(cfg.m_legend_pos));
			//m_legend->contentsWidget()->setVisible(cfg.m_legend_show);
		}

		for (size_t c = 0, ce = m_config.m_ccfg.size(); c < ce; ++c)
		{
			CurveConfig & cc = m_config.m_ccfg[c];
			mkCurve(cc.m_tag); // cc.m_tag is a subtag
		}

		for (size_t a = 0, ae = m_config.m_acfg.size(); a < ae; ++a)
		{
			AxisConfig & ac = m_config.m_acfg[a];
			// ...
		}

		setContextMenuPolicy(Qt::CustomContextMenu);
		connect(this, SIGNAL(customContextMenuRequested(QPoint const &)), this, SLOT(onShowContextMenu(QPoint const &)));

		m_colors.push_back(Qt::black);
		m_colors.push_back(Qt::red);
		m_colors.push_back(Qt::darkRed);
		m_colors.push_back(Qt::green);
		m_colors.push_back(Qt::darkGreen);
		m_colors.push_back(Qt::blue);
		m_colors.push_back(Qt::darkBlue);
		m_colors.push_back(Qt::cyan);
		m_colors.push_back(Qt::darkCyan);
		m_colors.push_back(Qt::magenta);
		m_colors.push_back(Qt::darkMagenta);
		m_colors.push_back(Qt::yellow);
		m_colors.push_back(Qt::darkYellow);
		
		setAutoReplot(true);

		QwtPlotMagnifier * lookglass = new QwtPlotMagnifier(canvas());
		canvas()->setFocusPolicy(Qt::WheelFocus);

		QwtPlotZoomer * zoomer = new QwtPlotZoomer(canvas());
		zoomer->setRubberBandPen( QColor( Qt::black ) );
		zoomer->setTrackerPen( QColor( Qt::black ) );
		zoomer->setMousePattern( QwtEventPattern::MouseSelect2, Qt::RightButton, Qt::ControlModifier );
		zoomer->setMousePattern( QwtEventPattern::MouseSelect3, Qt::RightButton, Qt::ShiftModifier);

		QwtPlotPicker * picker = new QwtPlotPicker(QwtPlot::xBottom, QwtPlot::yLeft,
											QwtPlotPicker::CrossRubberBand, QwtPicker::AlwaysOn,
											canvas());
		picker->setStateMachine(new QwtPickerDragPointMachine());
		picker->setRubberBandPen(QColor(Qt::green));
		picker->setRubberBand(QwtPicker::CrossRubberBand);
		picker->setTrackerPen(QColor(Qt::white));

		QwtPlotPanner * panner = new QwtPlotPanner(canvas());
		panner->setMouseButton(Qt::MidButton);

		//QwtPlotMarker * marker = new QwtPlotMarker();

		setConfigValuesToUI(m_config);
		QTimer::singleShot(0, this, SLOT(onApplyButton()));
		Ui::SettingsPlot * ui = m_config_ui.ui();
		connect(ui->applyButton, SIGNAL(clicked()), this, SLOT(onApplyButton()));
		connect(ui->clearDataButton, SIGNAL(clicked()), this, SLOT(onClearAllDataButton()));
		connect(ui->saveButton, SIGNAL(clicked()), this, SLOT(onSaveButton()));
		connect(ui->resetButton, SIGNAL(clicked()), this, SLOT(onResetButton()));
		connect(ui->defaultButton, SIGNAL(clicked()), this, SLOT(onDefaultButton()));
		connect(ui->curveComboBox, SIGNAL(activated(int)), this, SLOT(onCurveActivate(int)));
		connect(ui->clearCurveDataButton, SIGNAL(clicked()), this, SLOT(onClearCurveDataButton()));
		connect(ui->xAutoScaleCheckBox, SIGNAL(stateChanged(int)), SLOT(onXAutoScaleChanged(int)));
		connect(ui->yAutoScaleCheckBox, SIGNAL(stateChanged(int)), SLOT(onYAutoScaleChanged(int)));
		connect(ui->zAutoScaleCheckBox, SIGNAL(stateChanged(int)), SLOT(onZAutoScaleChanged(int)));
	}

	PlotWidget::~PlotWidget ()
	{
		qDebug("%s this=0x%08x", __FUNCTION__, this);
		stopUpdate();
		for (curves_t::iterator it = m_curves.begin(), ite = m_curves.end(); it != ite; ++it)
		{
			Curve * curve = *it;
			curve->m_curve->detach();
			delete curve;
		}
		m_curves.clear();
		disconnect(this, SIGNAL(customContextMenuRequested(QPoint const &)), this, SLOT(onShowPlotContextMenu(QPoint const &)));
	}

	void PlotWidget::onShow ()
	{
		show();
	}

	void PlotWidget::onHide ()
	{
		hide();
	}

	void PlotWidget::applyAxis (AxisConfig const & acfg)
	{
		QwtText title(acfg.m_label);
		QFont f = QFont("Verdana", 7);
		title.setFont(f);
		setAxisTitle(acfg.m_axis_pos, title);
		setAxisFont(acfg.m_axis_pos, f);
		if (!acfg.m_auto_scale)
			setAxisScale(acfg.m_axis_pos, acfg.m_from, acfg.m_to, acfg.m_step);
		else
			setAxisAutoScale(acfg.m_axis_pos, true);
		//setAxisScaleDraw(QwtPlot::xBottom, new TimeScaleDraw(cpuStat.upTime()));
		//setAxisLabelRotation(QwtPlot::xBottom, -50.0);
		//setAxisLabelAlignment(QwtPlot::xBottom, Qt::AlignLeft | Qt::AlignBottom);
	}

	void PlotWidget::applyConfig (PlotConfig const & pcfg)
	{
		qDebug("%s this=0x%08x", __FUNCTION__, this);
		setTitle(pcfg.m_title);
		if (pcfg.m_legend_show)
		{
			plotLayout()->setLegendPosition(static_cast<QwtPlot::LegendPosition>(pcfg.m_legend_pos));
			//m_legend->contentsWidget()->setVisible(static_cast<QwtPlot::LegendPosition>(pcfg.m_legend_show));
		}
		else
		{
			insertLegend(0);
		}
		for (size_t c = 0, ce = pcfg.m_ccfg.size(); c < ce; ++c)
		{
			CurveConfig const & cc = pcfg.m_ccfg[c];
			if (Curve * const curve = findOrCreateCurve(cc.m_tag))
			{
				curve->m_curve->setPen(QPen(cc.m_color));
				//curve->m_curve->setBrush(QBrush(cc.m_color));
				curve->m_curve->setTitle(cc.m_tag);
				curve->m_curve->setStyle(static_cast<QwtPlotCurve::CurveStyle>(cc.m_style - 1));
				QwtSymbol * symbol = new QwtSymbol(static_cast<QwtSymbol::Style>(cc.m_symbol - 1));
				symbol->setSize(cc.m_symbolsize);
				symbol->setPen(QPen(cc.m_symbolcolor));
				curve->m_curve->setSymbol(symbol);
				curve->m_curve->setBaseline(cc.m_pen_width);
				curve->m_curve->setLegendAttribute(QwtPlotCurve::LegendShowLine);
				curve->m_curve->setLegendAttribute(QwtPlotCurve::LegendShowSymbol);
				showCurve(curve->getCurve(), cc.m_show);
			}
		}

		if (m_timer != -1)
			killTimer(m_timer);
		m_timer = startTimer(pcfg.m_timer_delay_ms);

		applyAxis(pcfg.m_acfg[0]);
		applyAxis(pcfg.m_acfg[1]);
		updateAxes();
	}

	void PlotWidget::stopUpdate ()
	{
		if (m_timer != -1)
			killTimer(m_timer);
	}

	Curve * PlotWidget::findCurve (QString const & subtag)
	{
		curves_t::const_iterator it = m_curves.find(subtag);
		if (it == m_curves.end())
			return 0;
		return *it;
	}
	Curve * PlotWidget::findOrCreateCurve (QString const & subtag)
	{
		curves_t::const_iterator it = m_curves.find(subtag);
		if (it == m_curves.end())
			it = mkCurve(subtag);
		return *it;
	}

	void PlotWidget::timerEvent (QTimerEvent * e)
	{
		update();
	}

	void PlotWidget::update ()
	{
		if (!m_config.m_show)
			return;

		for (curves_t::iterator it = m_curves.begin(), ite = m_curves.end(); it != ite; ++it)
		{
			Curve & curve = **it;
			Data const & data = *curve.m_data;
			size_t n = data.m_data_x.size();
			if (!n)
				continue;

			curve.m_curve->setRawSamples(&data.m_data_x[0], &data.m_data_y[0], static_cast<int>(n));
		}

		replot();
	}

	void PlotWidget::showCurve (QwtPlotItem * item, bool on)
	{
		item->setVisible(on);
	#pragma message("!!! Qt5 incompatibility !!!")
		//if (QwtPlotLegendItem * legendItem = qobject_cast<QwtPlotLegendItem *>(legend()->find(item->winId())))
		//	legendItem->setChecked(on);
		replot();
	}

	void PlotWidget::onHideContextMenu ()
	{
		m_config_ui.onHidePlotContextMenu();
	}

	void PlotWidget::onShowContextMenu (QPoint const & pos)
	{
		QRect widgetRect = geometry();
		m_config_ui.onShowPlotContextMenu(QCursor::pos());
		Ui::SettingsPlot * ui = m_config_ui.ui();

		setConfigValuesToUI(m_config);
	}

	void PlotWidget::setConfigValuesToUI (PlotConfig const & pcfg)
	{
		qDebug("%s this=0x%08x", __FUNCTION__, this);
		Ui::SettingsPlot * ui = m_config_ui.ui();
		ui->curveComboBox->clear();
		for (size_t i = 0, ie = pcfg.m_ccfg.size(); i < ie; ++i)
		{
			CurveConfig const & ccfg = pcfg.m_ccfg[i];
			ui->curveComboBox->addItem(ccfg.m_tag);
		}

		bool const x_auto = pcfg.m_acfg[0].m_auto_scale;
		ui->xAutoScaleCheckBox->setCheckState(x_auto ? Qt::Checked : Qt::Unchecked);
		onXAutoScaleChanged(x_auto ? Qt::Checked : Qt::Unchecked);

		bool const y_auto = pcfg.m_acfg[0].m_auto_scale;
		ui->yAutoScaleCheckBox->setCheckState(y_auto ? Qt::Checked : Qt::Unchecked);
		onYAutoScaleChanged(y_auto ? Qt::Checked : Qt::Unchecked);

		bool const z_auto = pcfg.m_acfg[0].m_auto_scale;
		ui->zAutoScaleCheckBox->setCheckState(z_auto ? Qt::Checked : Qt::Unchecked);
		onZAutoScaleChanged(z_auto ? Qt::Checked : Qt::Unchecked);
		//m_config_ui.ui()->curveComboBox->setCurrentIndex(0);
		ui->symbolComboBox->clear();
		for (size_t i = 0; i < enum_to_string_ln_E_PlotSymbol(); ++i)
			ui->symbolComboBox->addItem(QString::fromLatin1(enum_to_string_E_PlotSymbol[i]));
		ui->styleComboBox->clear();
		for (size_t i = 0; i < enum_to_string_ln_E_CurveStyle(); ++i)
			ui->styleComboBox->addItem(QString::fromLatin1(enum_to_string_E_CurveStyle[i]));

		m_config.m_title = ui->titleLineEdit->text();
		ui->plotShowCheckBox->setCheckState(pcfg.m_show ? Qt::Checked : Qt::Unchecked);
		ui->autoScrollCheckBox->setCheckState(pcfg.m_auto_scroll ? Qt::Checked : Qt::Unchecked);
		ui->updateTimerSpinBox->setValue(pcfg.m_timer_delay_ms);

		//m_config.m_legend_pos = 
		ui->legendShowCheckBox->setChecked(pcfg.m_legend_show);

		if (m_config.m_acfg.size() >= 1)
			ui->xLabelLineEdit->setText(m_config.m_acfg[0].m_label);
		if (m_config.m_acfg.size() >= 2)
			ui->yLabelLineEdit->setText(m_config.m_acfg[1].m_label);
		onCurveActivate(0);
	}

	void PlotWidget::setUIValuesToConfig (PlotConfig & pcfg)
	{
		qDebug("%s this=0x%08x", __FUNCTION__, this);
		Ui::SettingsPlot * ui = m_config_ui.ui();
		pcfg.m_auto_scroll = ui->autoScrollCheckBox->isChecked();
		pcfg.m_timer_delay_ms = ui->updateTimerSpinBox->value();
		pcfg.m_title = ui->titleLineEdit->text();
		pcfg.m_show = ui->plotShowCheckBox->isChecked();
		pcfg.m_legend_show = ui->legendShowCheckBox->isChecked();
		//m_config.m_legend_pos = 

		int const curveidx = ui->curveComboBox->currentIndex();
		if (curveidx >= 0 && curveidx < pcfg.m_ccfg.size())
		{
			CurveConfig & ccfg = pcfg.m_ccfg[curveidx];
			ccfg.m_pen_width = ui->penWidthDblSpinBox->value();
			ccfg.m_style = ui->styleComboBox->currentIndex();
			ccfg.m_symbol = ui->symbolComboBox->currentIndex();
			ccfg.m_symbolsize = ui->symbolSizeSpinBox->value();
			ccfg.m_show = ui->showCurveCheckBox->checkState() == Qt::Checked;
			ccfg.m_color = m_config_ui.m_curve_color->currentColor();
			ccfg.m_symbolcolor = m_config_ui.m_symbol_color->currentColor();
		}

		pcfg.m_acfg[0].m_label = ui->xLabelLineEdit->text();
		pcfg.m_acfg[0].m_from = ui->xFromDblSpinBox->value();
		pcfg.m_acfg[0].m_to = ui->xToDblSpinBox->value();
		pcfg.m_acfg[0].m_step = ui->xStepDblSpinBox->value();
		pcfg.m_acfg[0].m_scale_type = ui->xScaleComboBox->currentIndex();
		pcfg.m_acfg[0].m_auto_scale = ui->xAutoScaleCheckBox->checkState() == Qt::Checked;
		pcfg.m_acfg[1].m_label = ui->yLabelLineEdit->text();
		pcfg.m_acfg[1].m_from = ui->yFromDblSpinBox->value();
		pcfg.m_acfg[1].m_to = ui->yToDblSpinBox->value();
		pcfg.m_acfg[1].m_step = ui->yStepDblSpinBox->value();
		pcfg.m_acfg[1].m_scale_type = ui->yScaleComboBox->currentIndex();
		pcfg.m_acfg[1].m_auto_scale = ui->yAutoScaleCheckBox->checkState() == Qt::Checked;
	}

	void PlotWidget::onApplyButton ()
	{
		qDebug("%s this=0x%08x", __FUNCTION__, this);
		setUIValuesToConfig(m_config);
		applyConfig(m_config);
		replot();
	}

	void PlotWidget::onXAutoScaleChanged (int state)
	{
		Ui::SettingsPlot * ui = m_config_ui.ui();
		bool enabled = state == Qt::Checked ? false : true;
		ui->xFromDblSpinBox->setEnabled(enabled);
		ui->xToDblSpinBox->setEnabled(enabled);
		ui->xStepDblSpinBox->setEnabled(enabled);
		ui->xScaleComboBox->setEnabled(enabled);
	}
	void PlotWidget::onYAutoScaleChanged (int state)
	{
		Ui::SettingsPlot * ui = m_config_ui.ui();
		bool enabled = state == Qt::Checked ? false : true;
		ui->yFromDblSpinBox->setEnabled(enabled);
		ui->yToDblSpinBox->setEnabled(enabled);
		ui->yStepDblSpinBox->setEnabled(enabled);
		ui->yScaleComboBox->setEnabled(enabled);
	}
	void PlotWidget::onZAutoScaleChanged (int state) { }

	void PlotWidget::applyConfig ()
	{
		qDebug("%s this=0x%08x", __FUNCTION__, this);
		applyConfig(m_config);
	}

	QString PlotWidget::getCurrentWidgetPath () const
	{
		QString const appdir = m_connection->getMainWindow()->getAppDir();
		QString const plotpath = appdir + "/" + m_connection->getCurrPreset() + "/" + g_PlotTag + "/" + m_config.m_tag;
		return plotpath;
	}

	void PlotWidget::loadConfig (QString const & preset_dir)
	{
		QString const tag_backup = m_config.m_tag;
		QString const plotpath = getCurrentWidgetPath();
		PlotConfig config;
		bool const loaded = loadConfigTemplate(config, plotpath + "/" + g_PlotFile);
		if (!loaded)
		{
			m_config.clear();
			m_config = config;
			m_config.m_tag = tag_backup; // defaultConfigFor destroys tag
		}
		else
		{
			m_config.clear();
			m_config = config;
		}
		loadAuxConfigs();
	}
	void PlotWidget::loadAuxConfigs ()
	{
		QString const plotpath = getCurrentWidgetPath();
		//m_config.m_find_config.clear();
		//loadConfigTemplate(m_config.m_find_config, plotpath + "/" + g_findTag);
		filterMgr()->loadConfig(plotpath);
		//colorizerMgr()->loadConfig(plotpath);
	}
	void PlotWidget::saveAuxConfigs ()
	{
		QString const plotpath = getCurrentWidgetPath();
		//saveConfigTemplate(m_config.m_find_config, plotpath + "/" + g_findTag);
		filterMgr()->saveConfig(plotpath);
		//colorizerMgr()->saveConfig(plotpath);
	}

	void PlotWidget::saveConfig (QString const & path) //@FIXME: unused arg
	{
		QString const plotpath = getCurrentWidgetPath();
		mkPath(plotpath);

		plot::PlotConfig tmp = m_config;
		saveConfigTemplate(tmp, plotpath + "/" + g_PlotFile);
		saveAuxConfigs();
	}

	void PlotWidget::onSaveButton ()
	{
		saveConfig(QString());
	}
	void PlotWidget::onResetButton () {}
	void PlotWidget::onDefaultButton ()
	{
		PlotConfig defaults;
		defaults.partialLoadFrom(m_config);
		setConfigValuesToUI(defaults);
	}

	void PlotWidget::onCurveActivate (int idx)
	{
		Ui::SettingsPlot * ui = m_config_ui.ui();
		QString const & curvename = ui->curveComboBox->currentText();
		for (size_t i = 0, ie = m_config.m_ccfg.size(); i < ie; ++i)
		{
			CurveConfig const & ccfg = m_config.m_ccfg[i];
			if (ccfg.m_tag == curvename)
			{
				ui->showCurveCheckBox->setCheckState(ccfg.m_show ? Qt::Checked : Qt::Unchecked);
				ui->penWidthDblSpinBox->setValue(ccfg.m_pen_width);
				ui->styleComboBox->setCurrentIndex(ccfg.m_style);
				ui->symbolComboBox->setCurrentIndex(ccfg.m_symbol);
				ui->symbolSizeSpinBox->setValue(ccfg.m_symbolsize);
				m_config_ui.m_curve_color->setCurrentColor(ccfg.m_color);
				m_config_ui.m_symbol_color->setCurrentColor(ccfg.m_color);
				/*ui->curveColorLabel->setAutoFillBackground(true);
				int const alpha  = 140;
				ui->curveColorLabel->setStyleSheet(tr("QLabel { background-color: rgba(%1, %2, %3, %4); }")
													.arg(ccfg.m_color.red())
													.arg(ccfg.m_color.green())
													.arg(ccfg.m_color.blue())
													.arg(alpha));*/
				break;
			}
		}
	}

	void PlotWidget::clearCurveData (QString const & subtag)
	{
		qDebug("%s this=0x%08x", __FUNCTION__, this);
		for (curves_t::iterator it = m_curves.begin(), ite = m_curves.end(); it != ite; ++it)
		{
			Curve * curve = *it;
			if (curve && curve->getName() == subtag && curve->m_data)
			{
				curve->m_data->clear();
				curve->m_curve->setRawSamples(0, 0, 0);
			}
		}
		update();
	}

	void PlotWidget::onClearAllDataButton ()
	{
		clearAllData();
	}

	void PlotWidget::clearAllData ()
	{
		for (curves_t::iterator it = m_curves.begin(), ite = m_curves.end(); it != ite; ++it)
		{
			Curve * curve = *it;
			if (curve->m_data)
			{
				curve->m_data->clear();
				curve->m_curve->setRawSamples(0, 0, 0);
			}
		}
		update();
	}

	void PlotWidget::onClearCurveDataButton ()
	{
		Ui::SettingsPlot * ui = m_config_ui.ui();
		clearCurveData(ui->curveComboBox->currentText());
	}

	void PlotWidget::commitCommands (E_ReceiveMode mode)
	{
		for (int i = 0, ie = m_queue.size(); i < ie; ++i)
		{
			DecodedCommand & cmd = m_queue[i];
			handleCommand(cmd, e_RecvSync);
		}
		m_queue.clear();
	}

	bool PlotWidget::handleAction (Action * a, E_ActionHandleType sync)
	{
		switch (a->type())
		{
			case e_Close:
			{
				m_connection->destroyDockedWidget(this);
				return true;
			}

			case e_Visibility:
			{
				m_connection->getMainWindow()->onDockRestoreButton();
				Q_ASSERT(m_args.size() > 0);
				bool const on = a->m_args.at(0).toBool();
				setVisible(on);
				return true;
			}

			case e_Find:
			default:
				return false;
		}
		return false;
	}

	void PlotWidget::setVisible (bool visible)
	{
		m_config.m_show = visible;
		m_dockwidget->setVisible(visible);
		QwtPlot::setVisible(visible);
	}

	void PlotWidget::handleCommand (DecodedCommand const & cmd, E_ReceiveMode mode)
	{
		if (mode == e_RecvSync)
		{
			if (cmd.m_hdr.cmd == tlv::cmd_plot_xy)
				handleDataXYCommand(cmd);
			else if (cmd.m_hdr.cmd == tlv::cmd_plot_clear)
				handlePlotClearCommand(cmd);
		}
		else
			m_queue.append(cmd);
	}

	bool PlotWidget::handleDataXYCommand (DecodedCommand const & cmd)
	{
		QString tag;
		double x = 0.0;
		double y = 0.0;
		for (size_t i=0, ie=cmd.m_tvs.size(); i < ie; ++i) // @TODO: precache
		{
			if (cmd.m_tvs[i].m_tag == tlv::tag_msg)
				tag = cmd.m_tvs[i].m_val;
			else if (cmd.m_tvs[i].m_tag == tlv::tag_x)
				x = cmd.m_tvs[i].m_val.toDouble(); // conversion float->double
			else if (cmd.m_tvs[i].m_tag == tlv::tag_y)
				y = cmd.m_tvs[i].m_val.toDouble(); // conversion float->double
		}

		int const slash_pos = tag.lastIndexOf(QChar('/'));
		QString subtag = tag;
		tag.chop(tag.size() - slash_pos);
		subtag.remove(0, slash_pos + 1);

		QStringList subpath = path();
		subpath << subtag;

		plot::Curve * curve = findCurve(subtag);
		if (!curve)
		{
			curve = *mkCurve(subtag);

			plot::CurveConfig const * ccfg = 0;
			m_config.findCurveConfig(subtag, ccfg); // config is created by mkCurve
			// @TODO: applyCurveConfig

			if (m_config.m_show)
			{
				bool const visible = ccfg ? ccfg->m_show : true;
				showCurve(curve->m_curve, visible);
			}
			else
			{
				bool const visible = ccfg ? ccfg->m_show : false;
				showCurve(curve->m_curve, visible);
			}
		}

		curve->m_data->push_back(x, y);

		// if (autoscroll && need_to) shift m_from;
		return true;
	}

	bool PlotWidget::handlePlotClearCommand (DecodedCommand const & cmd)
	{
		QString tag;
		for (size_t i=0, ie=cmd.m_tvs.size(); i < ie; ++i) // @TODO: precache
		{
			if (cmd.m_tvs[i].m_tag == tlv::tag_msg)
				tag = cmd.m_tvs[i].m_val;
		}
		int const slash_pos = tag.lastIndexOf(QChar('/'));
		QString subtag = tag;
		tag.chop(tag.size() - slash_pos);
		subtag.remove(0, slash_pos + 1);
		
		qDebug("clear plot: tag='%s' subtag='%s'", tag.toStdString().c_str(), subtag.toStdString().c_str());

		if (!subtag.isEmpty())
			clearCurveData(subtag);
		else
			clearAllData();
		return true;
	}


}

