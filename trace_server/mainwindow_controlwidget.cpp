#include "mainwindow.h"
#include "connection.h"
#include "utils_history.h"
#include "utils.h"
#include <ui_controlbarcommon.h>
#include <ui_settings.h>
#include <QMessageBox>

void MainWindow::onLevelValueChanged (int val)
{
	qDebug("level changed: %u", val);
	m_config.m_level = val;
	for (connections_t::iterator it = m_connections.begin(), ite = m_connections.end(); it != ite; ++it)
		(*it)->setLevelValue(val);
}

void MainWindow::onBufferingStateChanged (int state)
{
	m_config.m_buffered = state == Qt::Checked ? true : false;
	for (connections_t::iterator it = m_connections.begin(), ite = m_connections.end(); it != ite; ++it)
		(*it)->setBufferingState(state);
}

void MainWindow::onPresetChanged (int idx)
{
	m_config.m_preset_history.m_current_item = idx;
	m_config.saveHistory(m_appdir);
}

void MainWindow::onPresetApply ()
{
	QString const preset = getCurrentPresetName();
	onPresetApply(preset);
}

void MainWindow::onPresetSave ()
{
	QString const txt = getCurrentPresetName();
    onPresetSave(txt);
}

void MainWindow::onPresetAdd ()
{
	qDebug("%s", __FUNCTION__);
	QString const preset_name = promptAndCreatePresetName();
	onPresetSave(preset_name);
}

void MainWindow::onPresetRm ()
{
	qDebug("%s", __FUNCTION__);
	QString const preset_name = m_dock_mgr.controlUI()->presetComboBox->currentText();

	if (preset_name.isEmpty())
		return;
	qDebug("removing preset_name=%s", preset_name.toStdString().c_str());

	QString const fname = mkPresetPath(m_config.m_appdir, preset_name);
	qDebug("confirm to remove session file=%s", fname.toStdString().c_str());
	QMessageBox msg_box;
	QPushButton * b_del = msg_box.addButton(tr("Yes, Delete"), QMessageBox::ActionRole);
	QPushButton * b_abort = msg_box.addButton(QMessageBox::Abort);
	msg_box.exec();
	if (msg_box.clickedButton() == b_abort)
		return;

	QFile qf(fname);
	qf.remove();

	removeStringFromHistory(preset_name, m_dock_mgr.controlUI()->presetComboBox, m_config.m_preset_history);
	m_config.saveHistory(m_appdir);
	m_dock_mgr.controlUI()->presetComboBox->setCurrentIndex(-1);
}

void MainWindow::onPresetReset ()
{
}

void MainWindow::onLogsStateChanged (int state)
{
	m_config.m_logs_recv_level = state;
	for (connections_t::iterator it = m_connections.begin(), ite = m_connections.end(); it != ite; ++it)
		(*it)->setLogsState(state);
}
void MainWindow::onPlotsStateChanged (int state)
{
	m_config.m_plots_recv_level = state;
	for (connections_t::iterator it = m_connections.begin(), ite = m_connections.end(); it != ite; ++it)
		(*it)->setPlotsState(state);
}
void MainWindow::onTablesStateChanged (int state)
{
	m_config.m_tables_recv_level = state;
	for (connections_t::iterator it = m_connections.begin(), ite = m_connections.end(); it != ite; ++it)
		(*it)->setTablesState(state);
}
void MainWindow::onGanttsStateChanged (int state)
{
	m_config.m_gantts_recv_level = state;
	for (connections_t::iterator it = m_connections.begin(), ite = m_connections.end(); it != ite; ++it)
		(*it)->setGanttsState(state);
}

void MainWindow::setConfigValuesToUI (GlobalConfig const & cfg)
{
	m_dock_mgr.controlUI()->levelSpinBox->setValue(cfg.m_level);
	m_dock_mgr.controlUI()->buffCheckBox->setChecked(cfg.m_buffered);
	syncHistoryToWidget(m_dock_mgr.controlUI()->presetComboBox, cfg.m_preset_history);
	m_dock_mgr.controlUI()->logSlider->setValue(cfg.m_logs_recv_level);
	m_dock_mgr.controlUI()->plotSlider->setValue(cfg.m_plots_recv_level);
	m_dock_mgr.controlUI()->tableSlider->setValue(cfg.m_tables_recv_level);
	m_dock_mgr.controlUI()->ganttSlider->setValue(cfg.m_gantts_recv_level);
}

void MainWindow::setUIValuesToConfig (GlobalConfig & cfg)
{
}

//bool MainWindow::onTopEnabled () const { return ui_settings->onTopCheckBox->isChecked(); }


