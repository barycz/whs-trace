#include "connection.h"
#include <QTcpSocket>
#include <QMessageBox>
#include <tlv_parser/tlv_encoder.h>
#include "utils.h"
#include "utils_history.h"
#include "controlbarcommon.h"
#include "mainwindow.h"
#include <ui_controlbarcommon.h>

QString Connection::getClosestPresetName ()
{
	// bit clumsy, but no time to loose
	QString preset_name;
	QString const conn_name = m_control_bar->ui->presetComboBox->currentText();
	QString const parent_name = m_main_window->getCurrentPresetName();
	if (!parent_name.isEmpty() && validatePresetName(parent_name))
		preset_name = parent_name;
	else if (!conn_name.isEmpty() && validatePresetName(conn_name))
		preset_name = conn_name;
	else
		preset_name = g_defaultPresetName;
	return preset_name;
}

E_FeatureStates Connection::getClosestFeatureState (E_DataWidgetType type) const
{
	switch (type)
	{
		case e_data_log  : return static_cast<E_FeatureStates>(m_control_bar->ui->logSlider->value());
		case e_data_plot : return static_cast<E_FeatureStates>(m_control_bar->ui->plotSlider->value());
		case e_data_table: return static_cast<E_FeatureStates>(m_control_bar->ui->tableSlider->value());
		case e_data_gantt: return static_cast<E_FeatureStates>(m_control_bar->ui->ganttSlider->value());
		case e_data_frame: return static_cast<E_FeatureStates>(m_control_bar->ui->ganttSlider->value());
		default: return e_FtrDisabled;
	}
}

void Connection::setConfigValuesToUI (ConnectionConfig const & cfg)
{
	m_control_bar->ui->levelSpinBox->setValue(cfg.m_level);
	m_control_bar->ui->buffCheckBox->setChecked(cfg.m_buffered);
	syncHistoryToWidget(m_control_bar->ui->presetComboBox, cfg.m_preset_history);
	m_control_bar->ui->logSlider->setValue(cfg.m_logs_recv_level);
	m_control_bar->ui->plotSlider->setValue(cfg.m_plots_recv_level);
	m_control_bar->ui->tableSlider->setValue(cfg.m_tables_recv_level);
	m_control_bar->ui->ganttSlider->setValue(cfg.m_gantts_recv_level);
}

void Connection::setUIValuesToConfig (ConnectionConfig & cfg)
{
	cfg.m_level = m_control_bar->ui->levelSpinBox->value();
	cfg.m_buffered = m_control_bar->ui->buffCheckBox->isChecked();
	//
	cfg.m_logs_recv_level = static_cast<E_FeatureStates>(m_control_bar->ui->logSlider->value());
	cfg.m_plots_recv_level = static_cast<E_FeatureStates>(m_control_bar->ui->plotSlider->value());
	cfg.m_tables_recv_level = static_cast<E_FeatureStates>(m_control_bar->ui->tableSlider->value());
	cfg.m_gantts_recv_level = static_cast<E_FeatureStates>(m_control_bar->ui->ganttSlider->value()); 
}

void Connection::setLevelValue (int i) { m_control_bar->ui->levelSpinBox->setValue(i); }
void Connection::setBufferingState (int state) { m_control_bar->ui->buffCheckBox->setChecked(state); }
void Connection::setLogsState (int state) { m_control_bar->ui->logSlider->setValue(state); }
void Connection::setPlotsState (int state) { m_control_bar->ui->plotSlider->setValue(state); }
void Connection::setTablesState (int state) { m_control_bar->ui->tableSlider->setValue(state); }
void Connection::setGanttsState (int state) { m_control_bar->ui->ganttSlider->setValue(state); }

void Connection::onLogsStateChanged (int state)
{
	m_config.m_logs_recv_level = state;
	saveDefaultPreset();

}
void Connection::onPlotsStateChanged (int state)
{
	m_config.m_plots_recv_level = state;
	saveDefaultPreset();
}
void Connection::onTablesStateChanged (int state)
{
	m_config.m_tables_recv_level = state;
	saveDefaultPreset();
}
void Connection::onGanttsStateChanged (int state)
{
	m_config.m_gantts_recv_level = state;
	saveDefaultPreset();
}


void Connection::onLevelValueChanged (int val)
{
	char tlv_buff[16];
#ifdef __linux__
	int const result = snprintf(tlv_buff, 16, "%u", val);
#else
	int const result = _snprintf_s(tlv_buff, 16, "%u", val);
#endif

	if (result > 0)
	{	
		m_config.m_level = val;
		char buff[256];
		using namespace tlv;
		Encoder_v1 e(cmd_set_level, buff, 256);
		e.Encode(TLV(tag_lvl, tlv_buff));
		if (m_tcpstream && e.Commit())
		{
			m_tcpstream->write(e.buffer, e.total_len);
			m_tcpstream->flush();
		}

		saveDefaultPreset();
	}
}

void Connection::onBufferingStateChanged (int state)
{
	bool const buffering_enabled = state == Qt::Checked;

	char tlv_buff[16];
#ifdef __linux__
	int const result = snprintf(tlv_buff, 16, "%u", buffering_enabled);
#else
	int const result = _snprintf_s(tlv_buff, 16, "%u", buffering_enabled);
#endif

	if (result > 0)
	{
		m_config.m_buffered = buffering_enabled;

		qDebug("Connection::onBufferingStateChanged to_state=%i", buffering_enabled);
		char buff[256];
		using namespace tlv;
		Encoder_v1 e(cmd_set_buffering, buff, 256);
		e.Encode(TLV(tag_bool, tlv_buff));
		if (m_tcpstream && e.Commit())
		{
			m_tcpstream->write(e.buffer, e.total_len);
			m_tcpstream->flush();
		}

		saveDefaultPreset();
	}
}

void Connection::onPresetChanged (int idx)
{
	m_config.m_preset_history.m_current_item = idx;
	QString const preset_name = getCurrentPresetName();
	QString const path = mkAppPresetPath(getGlobalConfig().m_appdir, m_app_name, preset_name);
	m_config.saveHistory(path);
}

void Connection::onPresetApply ()
{
	QString const txt = getCurrentPresetName();
	onPresetApply(txt);
}

void Connection::onPresetSave ()
{
	QString const txt = getCurrentPresetName();
	onPresetSave(txt);
}

void Connection::saveDefaultPreset()
{
	QString const txt = getCurrentPresetName();
	if (txt == g_defaultPresetName)
		onPresetSave(txt);
}

void Connection::onPresetAdd ()
{
	qDebug("%s", __FUNCTION__);
	QString const preset_name = promptAndCreatePresetName();
	onPresetSave(preset_name);		
}

void Connection::onPresetRm ()
{
	qDebug("%s", __FUNCTION__);
	QString const preset_name = m_control_bar->ui->presetComboBox->currentText();

	if (preset_name.isEmpty())
		return;

	qDebug("removing preset_name=%s", preset_name.toStdString().c_str());
	
	QString const path = mkAppPresetPath(getGlobalConfig().m_appdir, m_app_name, preset_name);
	qDebug("confirm to remove session file=%s", path.toStdString().c_str());

	QMessageBox msg_box;
	QPushButton * b_del = msg_box.addButton(tr("Yes, Delete"), QMessageBox::ActionRole);
	QPushButton * b_abort = msg_box.addButton(QMessageBox::Abort);
	msg_box.exec();
	if (msg_box.clickedButton() == b_abort)
		return;

	QFile qf(path);
	qf.remove();

	removeStringFromHistory(preset_name, m_control_bar->ui->presetComboBox, m_config.m_preset_history);
	m_control_bar->ui->presetComboBox->setCurrentIndex(-1);
}

void Connection::onPresetReset ()
{
}

//////////////////////////////////////////////////

QString Connection::getCurrentPresetName () const
{
	QString txt = m_control_bar->ui->presetComboBox->currentText();
	if (txt.isEmpty())
		txt = g_defaultPresetName;
	return txt;
}

void Connection::setPresetAsCurrent (QString const & pname)
{
	m_control_bar->ui->presetComboBox->setCurrentIndex(m_control_bar->ui->presetComboBox->findText(pname));
}

void Connection::onPresetApply (QString const & preset_name)
{
	QString const path = mkAppPresetPath(getGlobalConfig().m_appdir, m_app_name, preset_name);
	mentionStringInHistory_Ref(preset_name, m_control_bar->ui->presetComboBox, m_config.m_preset_history);
	m_config.saveHistory(path);

	if (checkAppPresetPath(getGlobalConfig().m_appdir, m_app_name, preset_name))
	{
		m_curr_preset = preset_name;

		loadConfigs(path);
		applyConfigs();
		m_main_window->loadLayout(path + "/" + g_presetLayoutName);
	}
	/*else
	{
		removeStringFromHistory(preset_name, m_control_bar->ui->presetComboBox, m_config.m_preset_history);
		m_config.saveHistory(path);
		m_control_bar->ui->presetComboBox->setCurrentIndex(-1);
	}*/
	setPresetAsCurrent(preset_name);
}

void Connection::onPresetSave (QString const & preset_name)
{
	qDebug("%s %s", __FUNCTION__, preset_name.toStdString().c_str());
	if (!validatePresetName(preset_name))
		return;

	qDebug("SaveAs to preset_name=%s", preset_name.toStdString().c_str());
	QString const path = mkAppPresetPath(getGlobalConfig().m_appdir, m_app_name, preset_name);
	mentionStringInHistory_Ref(preset_name, m_control_bar->ui->presetComboBox, m_config.m_preset_history);
	m_config.saveHistory(path);
	setPresetAsCurrent(preset_name);

	createAppPresetPath(getGlobalConfig().m_appdir, m_app_name, preset_name);
		
	m_curr_preset = preset_name; // @TODO: ?? or only on apply preset?

	saveConfigs(path);
	m_main_window->saveLayout(path + "/" + g_presetLayoutName);
}

/*void Connection::mentionInPresetHistory (QString const & str)
{
	if (str.isEmpty() || !validatePresetName(str))
		return;

	mentionStringInHistory_NoRef(str, m_control_bar->ui->presetComboBox, m_config.m_preset_history);
	QString const path = mkAppPresetPath(getGlobalConfig().m_appdir, m_app_name, preset_name);
	m_config.saveHistory(path);
}*/


