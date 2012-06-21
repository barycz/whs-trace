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
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QComboBox>
#include <QTreeView>
#include <QSystemTrayIcon>
#include "server.h"
#include "types.h"

namespace Ui {
	class MainWindow;
	class SettingsDialog;
	class HelpDialog;
}

class QSystemTrayIcon;
class QAction;
class QMenu;
class QListView;
class QStandardItemModel;
class QLabel;
class SessionState;

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	explicit MainWindow (QWidget * parent = 0, bool quit_delay = true);
	~MainWindow ();

	QTabWidget * getTabTrace ();
	QTabWidget const * getTabTrace () const;

	QString getAppDir () const { return m_appdir; }

	columns_setup_t const & getColumnSetup (size_t i) const { return m_columns_setup.at(i); }
	columns_setup_t & getColumnSetup (size_t i) { return m_columns_setup[i]; }
	columns_sizes_t const & getColumnSizes (size_t i) const { return m_columns_sizes.at(i); }
	columns_sizes_t & getColumnSizes (size_t i) { return m_columns_sizes[i]; }
	columns_align_t const & getColumnAlign (size_t i) const { return m_columns_align.at(i); }
	columns_align_t & getColumnAlign (size_t i) { return m_columns_align[i]; }
	columns_elide_t const & getColumnElide (size_t i) const { return m_columns_elide.at(i); }
	columns_align_t & getColumnElide (size_t i) { return m_columns_elide[i]; }

	Preset const & getFilterPresets (size_t i) const { return m_filter_presets[i]; }
	Preset & getFilterPresets (size_t i) { return m_filter_presets[i]; }
	int findPresetName (QString const & name)
	{
		for (size_t i = 0, ie = m_preset_names.size(); i < ie; ++i)
			if (m_preset_names[i] == name)
				return i;
		return -1;
	}
	
	void saveSession (SessionState const & s, QString const & preset_name) const;
	bool loadSession (SessionState & s, QString const & preset_name);
	size_t addPresetName (QString const & name)
	{
		m_preset_names.push_back(name);
		m_filter_presets.push_back(Preset());
		return m_preset_names.size() - 1;
	}

	size_t findAppName (QString const & appname)
	{
		for (size_t i = 0, ie = m_app_names.size(); i < ie; ++i)
			if (m_app_names[i] == appname)
				return i;
		
		m_app_names.push_back(appname);
		m_columns_setup.reserve(16);
		m_columns_setup.push_back(columns_setup_t());
		m_columns_sizes.reserve(16);
		m_columns_sizes.push_back(columns_sizes_t());
		m_columns_align.push_back(columns_align_t());
		m_columns_elide.push_back(columns_elide_t());
		return m_app_names.size() - 1;
	}
	QList<QColor> const & getThreadColors () const { return m_thread_colors; }
	QTreeView * getWidgetFile ();
	QTreeView const * getWidgetFile () const;
	QTreeView * getWidgetCtx ();
	QTreeView const * getWidgetCtx () const;
    QComboBox * getFilterRegex ();
    QComboBox const * getFilterRegex () const;
	QTreeView * getWidgetRegex ();
	QTreeView const * getWidgetRegex () const;
    QComboBox * getFilterColorRegex ();
    QComboBox const * getFilterColorRegex () const;
	QListView * getWidgetColorRegex ();
	QListView const * getWidgetColorRegex () const;
	QListView * getWidgetTID ();
	QListView const * getWidgetTID () const;
	QListView * getWidgetLvl ();
	QListView const * getWidgetLvl () const;
	void setLevel (int i);
	int getLevel () const;
	bool scopesEnabled () const;
	bool onTopEnabled () const;
	bool autoScrollEnabled () const;
	bool reuseTabEnabled () const;
	bool filterEnabled () const;
	bool buffEnabled () const;
	Qt::CheckState buffState () const;
	bool clrFltEnabled () const;
	bool statsEnabled () const;
	E_FilterMode fltMode () const;
	void changeEvent (QEvent* e);
	void dropEvent (QDropEvent * event);
	void dragEnterEvent (QDragEnterEvent *event);
	bool eventFilter (QObject * o, QEvent * e);
	void setPresetNameIntoComboBox (QString const & pname);

	unsigned getHotKey () const;
	Server const * getServer () const { return m_server; }
	Server * getServer () { return m_server; }

public slots:
	void onHotkeyShowOrHide ();
	void onOnTop (int);

private slots:
	void loadState ();
	void loadPresets ();
	void storeState ();
	void saveCurrentSession (QString const & preset_name);
	void storePresets ();
	void timerHit ();
	void onQuit ();
	void onEditFind ();
	void onEditFindNext ();
	void onEditFindPrev ();
	void onFileLoad ();
	void openFiles (QStringList const & list);
	void onFileSave ();
	void onFileExportToCSV ();
	void onSetup ();
	void closeEvent (QCloseEvent *event);
	void iconActivated (QSystemTrayIcon::ActivationReason reason);
	void onQSearchEditingFinished ();
	void onQFilterLineEditFinished ();
	void onSaveCurrentFileFilter ();
	void onSaveCurrentFileFilterTo (QString const & name);
	void onAddCurrentFileFilter ();
	void onRmCurrentFileFilter ();
	void onPresetActivate (int idx);
	void onRegexActivate (int idx);
	void onRegexAdd ();
	void onRegexRm ();
	void onColorRegexActivate (int idx);
	void onColorRegexAdd ();
	void onColorRegexRm ();
	void onShowHelp ();
	void onDumpFilters ();
	void onFilterModeActivate (int idx);
	void onReuseTabChanged (int state);
	void onFilterFile (int state);
	void onSettingsAppSelected (int idx);
	void onClickedAtSettingPooftahButton ();
	void onClickedAtSettingColumnSetup (QModelIndex idx);
	void onClickedAtSettingColumnSizes (QModelIndex idx);
	void onClickedAtSettingColumnAlign (QModelIndex idx);
	void onClickedAtSettingColumnElide (QModelIndex idx);
	void syncSettingsViews (QListView const * const invoker, QModelIndex const idx);
	void syncColorRegexOnPreset (Connection * conn);
	void syncRegexOnPreset (Connection * conn);

private:
	void showServerStatus ();
	void loadNetworkSettings ();
	void setupMenuBar ();
	void createActions ();
	void createTrayIcon ();

	Ui::MainWindow * ui;
	Ui::SettingsDialog * ui_settings;
	Ui::HelpDialog * m_help;
	unsigned m_hotkey;
	bool m_hidden;
	bool m_was_maximized;
	QList<QString> m_app_names;					/// registered applications
	QList<columns_setup_t> m_columns_setup;		/// column setup for each registered application
	QList<columns_sizes_t> m_columns_sizes;		/// column sizes for each registered application
	QList<columns_align_t> m_columns_align;		/// column align for each registered application
	QList<columns_elide_t> m_columns_elide;		/// column elide for each registered application
	QList<QColor> m_thread_colors;				/// predefined coloring of threads
	QList<QString> m_preset_names;				/// registered presets
	filter_presets_t m_filter_presets;			/// list of strings for each preset
	QString m_last_search;
	QTimer * m_timer;
	Server * m_server;
	QAction * m_minimize_action;
	QAction * m_maximize_action;
	QAction * m_restore_action;
	QAction * m_quit_action;
	QMenu * m_tray_menu;
	QSystemTrayIcon * m_tray_icon;
	QLabel * m_status_label;
	QDialog * m_settings_dialog;
	QString m_trace_addr;
	unsigned short m_trace_port;
	QString m_profiler_addr;
	unsigned short m_profiler_port;
	QString m_appdir;
};

#endif // MAINWINDOW_H
