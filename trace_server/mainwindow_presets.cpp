#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "ui_settings.h"
#include "ui_help.h"
#include "connection.h"
#include "dock.h"
#include "dockmanager.h"
#include <ui_controlbarcommon.h>
#include <QMessageBox>
#include <QInputDialog>
#include "constants.h"
#include "utils.h"
#include "utils_history.h"
#include "serialize.h"

void MainWindow::loadConfig (QString const & path)
{
	QString const fname = path + "/" + g_MainConfigTag;
	GlobalConfig config2;
	if (!::loadConfigTemplate(config2, fname))
	{
		m_config.fillDefaultConfig();
	}
	else
	{
		m_config = config2;
	}
}

void MainWindow::saveConfig (QString const & path)
{
	QString const fname = path + "/" + g_MainConfigTag;
	::saveConfigTemplate(m_config, fname);
}

void MainWindow::setPresetAsCurrent (QString const & pname)
{
	m_dock_mgr.controlUI()->presetComboBox->setCurrentIndex(m_dock_mgr.controlUI()->presetComboBox->findText(pname));
}

QString MainWindow::getCurrentPresetName () const
{
	QString txt = m_dock_mgr.controlUI()->presetComboBox->currentText();
	if (txt.isEmpty())
		txt = g_defaultPresetName;
	return txt;
}

void MainWindow::onSave ()
{
	QString const txt = getCurrentPresetName();
	onPresetSave(txt);
}

void MainWindow::onSaveAs (QString const & preset_name)
{
	onPresetSave(preset_name);
}

void MainWindow::onPresetApply (QString const & preset_name)
{
	mentionStringInHistory_Ref(preset_name, m_dock_mgr.controlUI()->presetComboBox, m_config.m_preset_history);
	m_config.saveHistory();
	setPresetAsCurrent(preset_name);

	for (size_t i = 0; i < m_connections.size(); ++i)
	{
		m_connections[i]->onPresetApply(preset_name);
	}

	//loadLayout(preset_name);
}

void MainWindow::onPresetSave (QString const & preset_name)
{
	qDebug("%s %s", __FUNCTION__, preset_name.toStdString().c_str());
	if (!validatePresetName(preset_name))
		return;

	qDebug("SaveAs to preset_name=%s", preset_name.toStdString().c_str());
	mentionStringInHistory_Ref(preset_name, m_dock_mgr.controlUI()->presetComboBox, m_config.m_preset_history);
	m_config.saveHistory();
	setPresetAsCurrent(preset_name);

	for (size_t i = 0; i < m_connections.size(); ++i)
		m_connections[i]->onPresetSave(preset_name);

	storeState();
}

/*void MainWindow::mentionInPresetHistory (QString const & str)
{
	if (str.isEmpty() || !validatePresetName(str))
		return;

	mentionStringInHistory_NoRef(str, m_dock_mgr.controlUI()->presetComboBox, m_config.m_preset_history);
	m_config.saveHistory();
}*/

void MainWindow::saveLayout (QString const & fname)
{
	QFile file(fname);
	if (!file.open(QFile::WriteOnly))
	{
		QString msg = tr("Failed to open %1\n%2").arg(fname).arg(file.errorString());
		QMessageBox::warning(this, tr("Error"), msg);
		return;
	}

	QByteArray geo_data = saveGeometry();
	QByteArray layout_data = saveState();

	bool ok = file.putChar(static_cast<uchar>(geo_data.size()));
	if (ok)
		ok = file.write(geo_data) == geo_data.size();
	if (ok)
		ok = file.write(layout_data) == layout_data.size();

	if (!ok)
	{
		QString msg = tr("Error writing to %1\n%2").arg(fname).arg(file.errorString());
		QMessageBox::warning(this, tr("Error"), msg);
		return;
	}
}

void MainWindow::loadLayout (QString const & fname)
{
	QFile file(fname);
	if (!file.open(QFile::ReadOnly))
		return;

	uchar geo_size;
	QByteArray geo_data;
	QByteArray layout_data;

	bool ok = file.getChar(reinterpret_cast<char *>(&geo_size));
	if (ok) {
		geo_data = file.read(geo_size);
		ok = geo_data.size() == geo_size;
	}
	if (ok) {
		layout_data = file.readAll();
		ok = layout_data.size() > 0;
	}

	if (ok)
		ok = restoreGeometry(geo_data);
	if (ok)
		ok = restoreState(layout_data);

	if (!ok) {
		QString msg = tr("Error reading %1").arg(fname);
		QMessageBox::warning(this, tr("Error"), msg);
		return;
	}
}

void MainWindow::storeState ()
{
	qDebug("%s", __FUNCTION__);
	QSettings settings("MojoMir", "TraceServer");

	settings.setValue("geometry", saveGeometry());
	settings.setValue("windowState", saveState());

	m_dock_mgr.saveConfig(m_config.m_appdir);

	QString const preset_name = getCurrentPresetName();
	saveLayout(preset_name);
}

void MainWindow::restoreDockedWidgetGeometry ()
{
	QSettings settings("MojoMir", "TraceServer");

	//restoreGeometry(settings.value("geometry").toByteArray());
	restoreState(settings.value("windowState").toByteArray());
}

void MainWindow::loadState ()
{
	qDebug("%s", __FUNCTION__);
	m_config.loadHistory();

	QSettings settings("MojoMir", "TraceServer");
	restoreGeometry(settings.value("geometry").toByteArray());
	restoreState(settings.value("windowState").toByteArray());

	if (m_start_level == -1)
	{
		qDebug("reading saved level from cfg");
		m_dock_mgr.controlUI()->levelSpinBox->setValue(settings.value("levelSpinBox", 3).toInt());
	}
	else
	{
		qDebug("reading level from command line");
		m_dock_mgr.controlUI()->levelSpinBox->setValue(m_start_level);
	}

	m_dock_mgr.loadConfig(m_config.m_appdir);
	m_dock_mgr.applyConfig();
	ui->dockManagerButton->blockSignals(1);
	ui->dockManagerButton->setChecked(m_dock_mgr.m_config.m_show);
	ui->dockManagerButton->blockSignals(0);
	m_dock_mgr.m_dockwidget->setVisible(m_dock_mgr.m_config.m_show);

	unsigned const hotkeyCode = settings.value("hotkeyCode").toInt();
	m_config.m_hotkey = hotkeyCode ? hotkeyCode : VK_SCROLL;
	registerHotKey();
	//qApp->uninstallEventFilter(this); // @FIXME
	qApp->installEventFilter(this);

	QString const preset_name = getCurrentPresetName();
	loadLayout(preset_name);
}

void MainWindow::onDockManagerVisibilityChanged (bool state)
{
	m_dock_mgr.m_config.m_show = state;
	ui->dockManagerButton->setChecked(m_dock_mgr.m_config.m_show);
}
