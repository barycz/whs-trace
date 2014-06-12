/**
 * Copyright (C) 2011 Mojmir Svoboda
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is furnished to do
 * so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 **/
#pragma once
#include <QMainWindow>
#include <QSystemTrayIcon>
#include <tlv_parser/tlv_parser.h>
#include "config.h"
#include "dockmanager.h"

namespace Ui {
	class MainWindow;
	class SettingsDialog;
	class HelpDialog;
}

class Server;
class QAction;
class QMenu;
class QListView;
class QStandardItemModel;
class QLabel;
class QToolButton;
class TreeView;
class QTreeView;
class QTimer;
class Connection;
class SetupDialogCSV;

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	explicit MainWindow (QWidget * parent, bool quit_delay, bool dump_mode, QString const & log_name, int level);
	~MainWindow ();

	QTabWidget * getTabTrace ();
	QTabWidget const * getTabTrace () const;

	QString getAppDir () const { return m_config.m_appdir; }

	// presets
	void saveLayout (QString const & fname);
	void loadLayout (QString const & fname);

	void setPresetAsCurrent (QString const & pname);
	void mentionInPresetHistory (QString const & str);
	QString getCurrentPresetName () const;
	void saveDefaultPreset();

	// global config
	GlobalConfig const & getConfig () const { return m_config; }
	bool dumpModeEnabled () const { return m_config.m_dump_mode; }
	unsigned getHotKey () const;
	bool onTopEnabled () const;
	void mentionStringInRecentHistory_NoRef (QString const & str, History<QString> & h);
	void removeStringFromRecentHistory (QString const & str, History<QString> & h);
	void mentionStringInRecentHistory_Ref (QString const & str, History<QString> & h);
	void syncHistoryToRecent (History<QString> const & h);

	// drag and drop
	void changeEvent (QEvent * e);
	void dropEvent (QDropEvent * event);
	void dragEnterEvent (QDragEnterEvent * event);
	bool eventFilter (QObject * o, QEvent * e);
	void keyPressEvent (QKeyEvent * e);

	// docking stuff
	DockManager const & dockManager () const { return m_dock_mgr; }
	DockManager & dockManager () { return m_dock_mgr; }
	QString const & dockedName () const { return m_docked_name; }
	void addWindowAction (QAction * action);
	void rmWindowAction (QAction * action);

	// files and streams
	void createTailDataStream (QString const & fname);
	void createTailLogStream (QString const & fname, QString const & separator);
	void importDataStream (QString const & fname);
	void copyStorageTo (QString const & filename);
	Connection * findConnectionByName (QString const & app_name);
	Connection * createNewConnection ();
	Connection * createNewConnection (QString const & app_name);
	void markConnectionForClose (Connection * conn);
	bool executeSetupDialogCSV (QString const & fname);

public slots:
	void newConnection (Connection * connection);
	void onStatusChanged (QString const & status);
	void onCloseMarkedConnections ();
	void onReloadFile ();
	void requestReloadFile (QString const & fname) { m_reload_fnames.push_back(fname); }

	void onHotkeyShowOrHide ();
	void hide ();
	void showNormal ();
	void showMaximized ();
	void onOnTop (int);
	void restoreDockedWidgetGeometry ();
	void onDockRestoreButton ();
	void onDockManagerButton ();
	void onDockManagerVisibilityChanged (bool state);
	void onDockManagerClosed ();
	void onSave ();
	void onSaveAs ();

	void onChangeSetupDialogCSV (int n);
	void onChangeSeparatorDialogCSV (int n);
	void onChangeColumnDialogCSV (QModelIndex const &);
	void onChangeColumnRadioDialogCSV (bool toggled);
	void onChangeColumnImport ();
	void onChangeColumnReset ();
	void onChangeColumnSkipAll ();

	friend class Connection;
private slots:

	void loadState ();
	void storeGeometry ();
	void storeState ();
	void onTimerHit ();
	void onQuit ();
	//void onFocusChanged (QWidget * old, QWidget * now);
	void onQuitReally ();
	void openFiles (QStringList const & list);
	void onCloseConnection (Connection * c);

	void onFileLoad ();
	void onFileTail ();
	void onFileLoadCSV ();
	void onLogTail ();
	void tailFiles (QStringList const & list);
	void onSaveData ();
	void onExportDataToCSV ();
	void closeEvent (QCloseEvent *event);
	void iconActivated (QSystemTrayIcon::ActivationReason reason);
	void onRecentFile ();

	// control widget
	void onLevelValueChanged (int i);
	void onBufferingStateChanged (int state);
	void onPresetChanged (int idx);
	void onPresetEdited ();
	void onPresetApply ();
	void onPresetSave ();
	void onPresetAdd ();
	void onPresetRm ();
	void onPresetReset ();
	void onLogsStateChanged (int state);
	void onPlotsStateChanged (int state);
	void onTablesStateChanged (int state);
	void onGanttsStateChanged (int state);
	void onPresetApply (QString const & preset_name);
	void onPresetSave (QString const & preset_name);
	void onRemoveConfigurationFiles ();

	void onWidgetAdded(const QString & path, DockedWidgetBase * dw);
	void onWidgetRemoved(const QString & path, QWidget * w);

	//void onSaveCurrentStateTo (QString const & name);
	//void onPresetActivate (int idx);

	void onShowHelp ();

private:
	void showServerStatus ();
	void setupMenuBar ();
	void createTrayActions ();
	void createTrayIcon ();
	void registerHotKey ();
	void saveConfig ();
	void loadConfig ();
	void setConfigValuesToUI (GlobalConfig const & cfg);
	void setUIValuesToConfig (GlobalConfig & cfg);

	typedef std::vector<Connection *> connections_t;
	connections_t 		m_connections;

	Ui::MainWindow * 	ui;
	Ui::SettingsDialog * ui_settings;
	QDialog *			m_settings_dialog;
	SetupDialogCSV *	m_setup_dialog_csv;

	Ui::HelpDialog * 	m_help;
	GlobalConfig 		m_config;

	QTimer *  			m_timer;
	Server *  			m_server;
	QMenu * 			m_windows_menu;
	QMenu * 			m_file_menu;
	QMenu *				m_tools_menu;
	QAction * 			m_before_action;
	std::vector<QAction *> m_recent_files;
	QAction * 			m_minimize_action;
	QAction * 			m_maximize_action;
	QAction * 			m_restore_action;
	QAction * 			m_quit_action;
	QMenu *   			m_tray_menu;
	QAction *			m_widget_manager_action;
	QSystemTrayIcon * 	m_tray_icon;
	QLabel *			m_status_label;
	std::vector<QString> m_reload_fnames;

	// docked widgets
	DockManager 		m_dock_mgr;
	QString				m_docked_name;
	QString 			m_log_name;
	QString				m_appdir;

	int 				m_start_level; // @TODO: to config?
	float 				m_time_units;
};

