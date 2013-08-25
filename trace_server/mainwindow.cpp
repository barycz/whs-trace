#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "ui_settings.h"
#include "ui_help.h"
#include "server.h"
#include "connection.h"
#include <QTime>
#include <QTableView>
#include <QListView>
#include <QShortcut>
#include <QInputDialog>
#include <QFileDialog>
#include <QFontDialog>
#include <QMessageBox>
#include <QSettings>
#include <QMetaType>
#include <QMimeData>
#include <QVariant>
#include <QDropEvent>
#include <QDragEnterEvent>
#include <QItemDelegate>
#include <QStandardItemModel>
#include <QUrl>
#include <QtPlugin>
#include <QClipboard>
#include "utils.h"
#include "../tlv_parser/tlv_parser.h"
#include "help.h"
#include "version.h"
#include "constants.h"
#include "dock.h"
#include "utils_qstandarditem.h"
#include "utils_qsettings.h"

///////////  qt5 stuff
#include <QWindow>
#include <QtGui/5.1.1/QtGui/qpa/qplatformnativeinterface.h>
static QWindow * windowForWidget(const QWidget* widget)
{
	if (QWindow* window = widget->windowHandle()) { return window; }
	if (const QWidget* nativeParent = widget->nativeParentWidget()) { return nativeParent->windowHandle(); } 
	return 0; 
}
HWND getHWNDForWidget (QWidget const * widget)
{
	if (QWindow* window = ::windowForWidget(widget))
	{
		if (window->handle()) 
		{
			return static_cast<HWND>(QGuiApplication::platformNativeInterface()->nativeResourceForWindow(QByteArrayLiteral("handle"), window));
		}
	}
	return 0;
} 
/////////// 



#ifdef WIN32
#	define WIN32_LEAN_AND_MEAN
#	include <windows.h>
#endif

#if (defined WIN32) && (defined STATIC)
	Q_IMPORT_PLUGIN(QWindowsIntegrationPlugin);
	Q_IMPORT_PLUGIN(QICOPlugin);
//	Q_IMPORT_PLUGIN(qsvg); //@TODO: NEZAPOMENOUT ODKOMENTOVAT!
#endif

void MainWindow::loadNetworkSettings ()
{
	QSettings settings("MojoMir", "TraceServer");
	m_config.m_trace_addr = settings.value("trace_addr", "127.0.0.1").toString();
	m_config.m_trace_port = settings.value("trace_port", Server::default_port).toInt();
	m_config.m_profiler_addr = settings.value("profiler_addr", "127.0.0.1").toString();
	m_config.m_profiler_port = settings.value("profiler_port", 13147).toInt();
}

MainWindow::MainWindow (QWidget * parent, bool quit_delay, bool dump_mode, QString const & log_name, int level)
	: QMainWindow(parent)
	, m_time_units(0.001f)
	, ui(new Ui::MainWindow)
	, ui_settings(0)
	, m_help(new Ui::HelpDialog)
	, m_timer(new QTimer(this))
	, m_server(0)
	, m_minimize_action(0)
	, m_maximize_action(0)
	, m_restore_action(0)
	, m_quit_action(0)
	, m_tray_menu(0)
	, m_tray_icon(0)
	, m_settings_dialog(0)
	, m_dock_mgr(this, QStringList(QString("trace_server")))
	, m_docked_name("trace_server")
	, m_log_name(log_name)
	, m_start_level(level)
{
	qDebug("================================================================================");
	qDebug("%s this=0x%08x", __FUNCTION__, this);
	//QDir::setSearchPaths("icons", QStringList(QDir::currentPath()));
	ui->setupUi(this);
	ui->tabTrace->setTabsClosable(true);

	m_settings_dialog = new QDialog(this);
	m_settings_dialog->setWindowFlags(Qt::Sheet);
	ui_settings = new Ui::SettingsDialog();
	ui_settings->setupUi(m_settings_dialog);
	//ui_settings->separatorComboBox->addItem("\\t");
	//ui_settings->separatorComboBox->addItem("\\n");

	QString const homedir = QDir::homePath();
	m_config.m_appdir = homedir + "/.flogging";
	m_config.m_dump_mode = dump_mode;

	m_config.m_columns_setup.reserve(16);
	m_config.m_columns_sizes.reserve(16);
	m_config.m_columns_align.reserve(16);
	m_config.m_columns_elide.reserve(16);

	// tray stuff
	createActions();
	createTrayIcon();
	QIcon icon(":images/Icon1.ico");
	setWindowIcon(icon);
	m_tray_icon->setVisible(true);
	m_tray_icon->show();

	setAcceptDrops(true);
	setDockNestingEnabled(true);
	setAnimated(false);

	QSettings settings("MojoMir", "TraceServer");
	bool const on_top = settings.value("onTopCheckBox", false).toBool();
	if (on_top)
	{
		onOnTop(on_top);
	}
	ui_settings->onTopCheckBox->setChecked(on_top);

	loadNetworkSettings();
	m_server = new Server(m_config.m_trace_addr, m_config.m_trace_port, this, quit_delay);
	connect(m_server, SIGNAL(newConnection(Connection *)), this, SLOT(newConnection(Connection *)));
	showServerStatus();

	/*size_t const n = tlv::get_tag_count();
	QString msg_tag;
	for (size_t i = tlv::tag_time; i < n; ++i)
	{
		char const * name = tlv::get_tag_name(i);
		if (name)
		{
			QString qname = QString::fromStdString(name);
			if (i == tlv::tag_msg)
				msg_tag = qname;
			//ui->qSearchColumnComboBox->addItem(qname);
		}
	}
	ui->qSearchColumnComboBox->addItem("trace_server");
	ui->qSearchColumnComboBox->setCurrentIndex(ui->qSearchColumnComboBox->findText(msg_tag));*/

	m_timer->setInterval(5000);
	connect(m_timer, SIGNAL(timeout()) , this, SLOT(timerHit()));
	m_timer->start();
	setupMenuBar();

	connect(ui->tabTrace, SIGNAL(currentChanged(int)), this, SLOT(onTabTraceFocus(int)));
	connect(ui->tabTrace, SIGNAL(tabCloseRequested(int)), this, SLOT(onCloseTabWithIndex(int)));

	connect(ui->dtToolButton, SIGNAL(clicked()), this, SLOT(ondtToolButton()));

	connect(ui->timeComboBox, SIGNAL(activated(int)), this, SLOT(onTimeUnitsChanged(int)));
	connect(ui->levelSpinBox, SIGNAL(valueChanged(int)), this, SLOT(onLevelValueChanged(int)));
	connect(ui->plotSlider, SIGNAL(valueChanged(int)), this, SLOT(onPlotStateChanged(int)));
	connect(ui->tableSlider, SIGNAL(valueChanged(int)), this, SLOT(onTablesStateChanged(int)));
	connect(ui->dockedWidgetsToolButton, SIGNAL(clicked()), this, SLOT(onDockedWidgetsToolButton()));
	connect(ui->buffCheckBox, SIGNAL(stateChanged(int)), this, SLOT(onBufferingStateChanged(int)));
	
	//connect(ui_settings->tableFontToolButton, SIGNAL(clicked()), this, SLOT(onTableFontToolButton()));
	connect(ui_settings->onTopCheckBox, SIGNAL(stateChanged(int)), this, SLOT(onOnTop(int)));//@FIXME: this has some issues
	connect(ui_settings->reuseTabCheckBox, SIGNAL(stateChanged(int)), this, SLOT(onReuseTabChanged(int)));

	//connect(ui->clrFiltersCheckBox, SIGNAL(stateChanged(int)), this, SLOT(onClrFiltersStateChanged(int)));
	connect(ui->activatePresetButton, SIGNAL(clicked()), this, SLOT(onPresetActivate()));
	connect(ui->presetComboBox, SIGNAL(activated(int)), this, SLOT(onPresetChanged(int)));
	connect(ui->presetAddButton, SIGNAL(clicked()), this, SLOT(onAddPreset()));
	connect(ui->presetRmButton, SIGNAL(clicked()), this, SLOT(onRmCurrentState()));
	connect(ui->presetSaveButton, SIGNAL(clicked()), this, SLOT(onSaveCurrentState()));
	connect(ui->presetResetButton, SIGNAL(clicked()), this, SLOT(onClearCurrentState()));


	m_status_label = new QLabel(m_server->getStatus());
	QString human_version(g_Version);
	human_version.chop(human_version.lastIndexOf(QChar('-')));
	QLabel * version_label = new QLabel(tr("Ver: %1").arg(human_version));
	statusBar()->addPermanentWidget(version_label);
	statusBar()->addWidget(m_status_label);


	QTimer::singleShot(0, this, SLOT(loadState()));	// trigger lazy load of settings
	setWindowTitle("trace_server");
	setObjectName("trace_server");
}

MainWindow::~MainWindow()
{
	qDebug("%s this=0x%08x", __FUNCTION__, this);
#ifdef WIN32
	UnregisterHotKey(getHWNDForWidget(this), 0);
#endif
	m_server->setParent(0);
	delete m_server;
	delete m_tray_icon;
	delete m_tray_menu;
	delete m_help;
	delete ui;
	delete ui_settings;
}

void MainWindow::hide ()
{
	qDebug("%s", __FUNCTION__);
	m_config.m_hidden = true;
	QMainWindow::hide();
}

void MainWindow::showNormal ()
{
	qDebug("%s", __FUNCTION__);
	m_config.m_hidden = false;
	QMainWindow::showNormal();
}

void MainWindow::showMaximized ()
{
	qDebug("%s", __FUNCTION__);
	m_config.m_hidden = false;
	QMainWindow::showMaximized();
}

void MainWindow::createActions ()
{
	qDebug("%s", __FUNCTION__);
	m_minimize_action = new QAction(tr("Mi&nimize"), this);
	connect(m_minimize_action, SIGNAL(triggered()), this, SLOT(hide()));

	m_maximize_action = new QAction(tr("Ma&ximize"), this);
	connect(m_maximize_action, SIGNAL(triggered()), this, SLOT(showMaximized()));

	m_restore_action = new QAction(tr("&Restore"), this);
	connect(m_restore_action, SIGNAL(triggered()), this, SLOT(showNormal()));

	m_quit_action = new QAction(tr("&Quit"), this);
	connect(m_quit_action, SIGNAL(triggered()), this, SLOT(onQuit()));
}

void MainWindow::createTrayIcon ()
{
	qDebug("%s", __FUNCTION__);
	m_tray_menu = new QMenu(this);
	m_tray_menu->addAction(m_minimize_action);
	m_tray_menu->addAction(m_maximize_action);
	m_tray_menu->addAction(m_restore_action);
	m_tray_menu->addSeparator();
	m_tray_menu->addAction(m_quit_action);

	QIcon icon(":/images/Icon1.ico");
	m_tray_icon = new QSystemTrayIcon(icon, this);
	m_tray_icon->setContextMenu(m_tray_menu);

	connect(m_tray_icon, SIGNAL(messageClicked()), this, SLOT(messageClicked()));
	connect(m_tray_icon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), this, SLOT(iconActivated(QSystemTrayIcon::ActivationReason)));
}

void MainWindow::dropEvent (QDropEvent * event)
{
	QMimeData const * mimeData = event->mimeData();

	if (mimeData->hasUrls())
	{
		QStringList pathList;
		QList<QUrl> urlList = mimeData->urls();

		for (int i = 0; i < urlList.size() && i < 32; ++i)
			pathList.append(urlList.at(i).toLocalFile());
		openFiles(pathList);
	}

	event->acceptProposedAction();
}

void MainWindow::dragEnterEvent (QDragEnterEvent *event)
{
	event->acceptProposedAction();
}

void MainWindow::showServerStatus ()
{
	statusBar()->showMessage(m_server->getStatus());
}

void MainWindow::timerHit ()
{
	showServerStatus();
}

QTabWidget * MainWindow::getTabTrace () { return ui->tabTrace; }
QTabWidget const * MainWindow::getTabTrace () const { return ui->tabTrace; }



//QTreeView const * MainWindow::getDockedWidgetsTreeView () const { return m_docked_widgets_tree_view; }

bool MainWindow::onTopEnabled () const { return ui_settings->onTopCheckBox->isChecked(); }
int MainWindow::plotState () const { return ui->plotSlider->value(); }
int MainWindow::tableState () const { return ui->tableSlider->value(); }
int MainWindow::ganttState () const { return ui->ganttSlider->value(); }
bool MainWindow::reuseTabEnabled () const { return true; 
	
	
	
	
	ui_settings->reuseTabCheckBox->isChecked(); }
bool MainWindow::buffEnabled () const { return ui->buffCheckBox->isChecked(); }
Qt::CheckState MainWindow::buffState () const { return ui->buffCheckBox->checkState(); }
//bool MainWindow::statsEnabled () const { return ui_settings->traceStatsCheckBox->isChecked(); }
bool MainWindow::dtEnabled () const { return ui->dtToolButton->isChecked(); }


void MainWindow::setLevel (int i)
{
	bool const old = ui->levelSpinBox->blockSignals(true);
	ui->levelSpinBox->setValue(i);
	ui->levelSpinBox->blockSignals(old);
}

int MainWindow::getLevel () const
{
	int const current = ui->levelSpinBox->value();
	return current;
}

void MainWindow::onQuit ()
{
	qDebug("onQuit: hide systray, store state, qApp->quit");

	m_tray_icon->setVisible(false);
	m_tray_icon->hide();
	storeState();

	QTimer::singleShot(0, this, SLOT(onQuitReally()));	// trigger lazy quit
}

void MainWindow::onQuitReally ()
{
	qDebug("%s", __FUNCTION__);
	qApp->quit();
}

/*void MainWindow::onReuseTabChanged (int state)
{
	ui_settings->clrFiltersCheckBox->setEnabled(state);
}*/

void MainWindow::onDockedWidgetsToolButton ()
{
	if (ui->dockedWidgetsToolButton->isChecked())
	{
		m_dock_mgr.m_docked_widgets->show();
		restoreDockWidget(m_dock_mgr.m_docked_widgets);
	}
	else
	{
		m_dock_mgr.m_docked_widgets->hide();
	}
}

void MainWindow::onDockManagerClosed ()
{
	ui->dockedWidgetsToolButton->setChecked(false);
}

void MainWindow::onTablesStateChanged (int state) { }

void MainWindow::tailFiles (QStringList const & files)
{
	for (int i = 0, ie = files.size(); i < ie; ++i)
	{
		QString fname = files.at(i);
		if (fname != "")
			createTailDataStream(fname);
	}
}

void MainWindow::onFileTail ()
{
	QString fname = QFileDialog::getOpenFileName(this, tr("Open File"), "", tr("*.*"));
	QStringList files;
	files << fname;
	tailFiles(files);
}

void MainWindow::onLogTail ()
{
	//setupSeparatorChar("|");
	//createTailLogStream(m_log_name, "|");
}

void MainWindow::openFiles (QStringList const & files)
{
	for (int i = 0, ie = files.size(); i < ie; ++i)
	{
		QString fname = files.at(i);
		if (fname != "")
		{
			importDataStream(fname);
		}
	}
}

void MainWindow::onFileLoad ()
{
	// @TODO: multi-selection?
	QString fname = QFileDialog::getOpenFileName(this, tr("Open File"), "", tr("TLV Trace Files (*.tlv_trace)"));
	QStringList files;
	files << fname;
	openFiles(files);
}

void MainWindow::onFileSave ()
{
	QString filename = QFileDialog::getSaveFileName(this, tr("Save File"), "", tr("Trace Files (*.tlv_trace)"));

	if (filename != "")
	{
		copyStorageTo(filename);
	}
}

void MainWindow::onFileExportToCSV ()
{
	QString filename = QFileDialog::getSaveFileName(this, tr("Save File"), "", tr("Trace Files (*.csv)"));

	if (filename != "")
	{
		//exportStorageToCSV(filename);
	}
}

void MainWindow::onOnTop (int const state)
{
	if (state == 0)
	{
		Qt::WindowFlags const old = windowFlags();
		Qt::WindowFlags const newflags = Qt::Window | (old & ~(Qt::WindowStaysOnTopHint));
		setWindowFlags(newflags);
		show();
	}
	else
	{
		//setWindowFlags(Qt::WindowStaysOnTopHint); //@NOTE: win users lacks the min and max buttons. sigh.
		setWindowFlags(Qt::WindowStaysOnTopHint);
		show();
	}
}

void MainWindow::onHotkeyShowOrHide ()
{
	bool const not_on_top = !isActiveWindow();
	qDebug("onHotkeyShowOrHide() isActive=%u", not_on_top);
	m_config.m_hidden = !m_config.m_hidden;
		
	if (m_config.m_hidden)
	{
		m_config.m_was_maximized = isMaximized();
		hide();
	}
	else
	{
		if (m_config.m_was_maximized)
			showMaximized();
		else
			showNormal();
		raise();
		activateWindow();
	}
}


void MainWindow::onShowHelp ()
{
	QDialog dialog(this);
	dialog.setWindowFlags(Qt::Sheet);
	m_help->setupUi(&dialog);
	m_help->helpTextEdit->clear();
	m_help->helpTextEdit->setHtml(QString(html_help));
	m_help->helpTextEdit->setReadOnly(true);
	dialog.exec();
}

void MainWindow::storeGeometry ()
{
	qDebug("%s", __FUNCTION__);
	QSettings settings("MojoMir", "TraceServer");
	settings.setValue("geometry", saveGeometry());
	settings.setValue("windowState", saveState());
}

void MainWindow::onSaveAllButton ()
{
	storeState();
	qDebug("%s", __FUNCTION__);
	storeGeometry();

	if (Connection * conn = findCurrentConnection())
	{
		conn->onSaveAll();
	}
}

void MainWindow::onDockRestoreButton ()
{
	qDebug("%s", __FUNCTION__);
	QSettings settings("MojoMir", "TraceServer");
	restoreState(settings.value("windowState").toByteArray());
	restoreGeometry(settings.value("geometry").toByteArray());
}

void MainWindow::setupMenuBar ()
{
	qDebug("%s", __FUNCTION__);
	// File
	QMenu * fileMenu = menuBar()->addMenu(tr("&File"));
	fileMenu->addAction(tr("File &Load..."), this, SLOT(onFileLoad()), QKeySequence(Qt::ControlModifier + Qt::Key_O));
	fileMenu->addAction(tr("File &Tail..."), this, SLOT(onFileTail()), QKeySequence(Qt::ControlModifier + Qt::Key_T));
	fileMenu->addAction(tr("File &Save..."), this, SLOT(onFileSave()), QKeySequence(Qt::ControlModifier + Qt::Key_S));
	fileMenu->addAction(tr("File &Save As CSV format"), this, SLOT(onFileExportToCSV()), QKeySequence(Qt::ControlModifier + Qt::ShiftModifier + Qt::Key_S));
	fileMenu->addSeparator();
	fileMenu->addAction(tr("Quit program"), this, SLOT(onQuit()), QKeySequence::Quit);

	// Edit
	QMenu * editMenu = menuBar()->addMenu(tr("&Edit"));
	editMenu->addAction(tr("Find"), this, SLOT(onEditFind()), QKeySequence::Find);
	editMenu->addAction(tr("Find Next"), this, SLOT(onEditFindNext()), QKeySequence::FindNext);
	editMenu->addAction(tr("Find Prev"), this, SLOT(onEditFindPrev()), QKeySequence::FindPrevious);
	editMenu->addAction(tr("Find and Select All"), this, SLOT(onFindAllButton()));
	editMenu->addAction(tr("Goto Next Tag or Selection"), this, SLOT(onNextToView()));

	new QShortcut(QKeySequence(Qt::Key_Slash), this, SLOT(onEditFind()));
	editMenu->addSeparator();
	editMenu->addAction(tr("Copy"), m_server, SLOT(onCopyToClipboard()), QKeySequence::Copy);
	new QShortcut(QKeySequence(Qt::ControlModifier + Qt::Key_Insert), this, SLOT(onCopyToClipboard()));
	editMenu->addSeparator();
	editMenu->addAction(tr("Close Tab"), m_server, SLOT(onCloseCurrentTab()), QKeySequence(Qt::ControlModifier + Qt::Key_W));

	// Filter
	QMenu * filterMenu = menuBar()->addMenu(tr("Fi&lter"));
	filterMenu->addAction(tr("Hide previous rows"), m_server, SLOT(onHidePrevFromRow()), QKeySequence(Qt::Key_Delete));
	filterMenu->addAction(tr("Unhide previous rows"), m_server, SLOT(onUnhidePrevFromRow()), QKeySequence(Qt::ControlModifier + Qt::Key_Delete));
	filterMenu->addAction(tr("Set time reference row"), m_server, SLOT(onTimeRefFromRow()), QKeySequence(Qt::Key_Space));
	filterMenu->addAction(tr("Exclude file:line row"), m_server, SLOT(onExcludeFileLine()), QKeySequence(Qt::Key_X));
	filterMenu->addAction(tr("Goto file filter"), this, SLOT(onGotoFileFilter()), QKeySequence(Qt::ControlModifier + Qt::Key_F1));
	filterMenu->addAction(tr("Goto level filter"), this, SLOT(onGotoLevelFilter()), QKeySequence(Qt::ControlModifier + Qt::Key_F2));
	filterMenu->addAction(tr("Goto regex filter"), this, SLOT(onGotoRegexFilter()), QKeySequence(Qt::ControlModifier + Qt::Key_F3));
	filterMenu->addAction(tr("Goto color filter"), this, SLOT(onGotoColorFilter()), QKeySequence(Qt::ControlModifier + Qt::Key_F4));

	QMenu * tailMenu = menuBar()->addMenu(tr("&Tail"));
	tailMenu->addAction(tr("File &Tail..."), this, SLOT(onFileTail()), QKeySequence(Qt::ControlModifier + Qt::Key_T));
	tailMenu->addAction(tr("Trace Server Log"), this, SLOT(onLogTail()), QKeySequence(Qt::ControlModifier + Qt::AltModifier + Qt::Key_L));
		
	// Clear
	QMenu * clearMenu = menuBar()->addMenu(tr("&Clear"));
	clearMenu->addAction(tr("Clear current table view"), m_server, SLOT(onClearCurrentView()), QKeySequence(Qt::Key_C));
	new QShortcut(QKeySequence(Qt::ControlModifier + Qt::Key_L), this, SLOT(onClearCurrentView()));
	clearMenu->addAction(tr("Clear current file filter"), m_server, SLOT(onClearCurrentFileFilter()));
	clearMenu->addAction(tr("Clear current context filter"), m_server, SLOT(onClearCurrentCtxFilter()));
	clearMenu->addAction(tr("Clear current thread id filter"), m_server, SLOT(onClearCurrentCtxFilter()));
	clearMenu->addAction(tr("Clear current colorized regexp filter"), m_server, SLOT(onClearCurrentColorizedRegexFilter()));
	clearMenu->addAction(tr("Clear current regexp filter"), m_server, SLOT(onClearCurrentRegexFilter()));
	clearMenu->addAction(tr("Clear current collapsed scope filter"), m_server, SLOT(onClearCurrentScopeFilter()));
	clearMenu->addAction(tr("Clear current ref time"), m_server, SLOT(onClearCurrentRefTime()));

	// Tools
	QMenu * tools = menuBar()->addMenu(tr("&Settings"));
	tools->addAction(tr("&Options"), this, SLOT(onSetupAction()), QKeySequence(Qt::AltModifier + Qt::ShiftModifier + Qt::Key_O));
	//tools->addAction(tr("Save Current Filter As..."), this, SLOT(onSaveCurrentFileFilter()));
	tools->addSeparator();
	tools->addAction(tr("Save options now (this will NOT save presets)"), this, SLOT(storeState()), QKeySequence(Qt::AltModifier + Qt::ControlModifier + Qt::ShiftModifier + Qt::Key_O));

	// Help
	QMenu * helpMenu = menuBar()->addMenu(tr("&Help"));
	helpMenu->addAction(tr("Help"), this, SLOT(onShowHelp()));
	helpMenu->addAction(tr("Dump filters"), this, SLOT(onDumpFilters()));

	new QShortcut(QKeySequence(Qt::AltModifier + Qt::Key_Space), this, SLOT(onAutoScrollHotkey()));
}

void MainWindow::storeState ()
{
	qDebug("%s", __FUNCTION__);
	QSettings settings("MojoMir", "TraceServer");

	settings.setValue("trace_addr", m_config.m_trace_addr);
	settings.setValue("trace_port", m_config.m_trace_port);
	settings.setValue("profiler_addr", m_config.m_profiler_addr);
	settings.setValue("profiler_port", m_config.m_profiler_port);

	settings.setValue("geometry", saveGeometry());
	settings.setValue("windowState", saveState());
	//settings.setValue("splitter", ui->splitter->saveState());

	settings.setValue("tableSlider", ui->tableSlider->value());
	settings.setValue("plotSlider", ui->plotSlider->value());
	settings.setValue("ganttSlider", ui->ganttSlider->value());
	settings.setValue("buffCheckBox", ui->buffCheckBox->isChecked());
	//settings.setValue("filterModeComboBox", ui->filterModeComboBox->currentIndex());
	//settings.setValue("filterPaneComboBox", ui_settings->filterPaneComboBox->currentIndex());
	settings.setValue("levelSpinBox", ui->levelSpinBox->value());

	settings.setValue("trace_stats", ui_settings->traceStatsCheckBox->isChecked());
	settings.setValue("reuseTabCheckBox", ui_settings->reuseTabCheckBox->isChecked());
	settings.setValue("onTopCheckBox", ui_settings->onTopCheckBox->isChecked());
	settings.setValue("presetComboBox", ui->presetComboBox->currentText());

	write_list_of_strings(settings, "known-applications", "application", m_config.m_app_names);
	for (int i = 0, ie = m_config.m_app_names.size(); i < ie; ++i)
	{
		settings.beginGroup(tr("column_order_%1").arg(m_config.m_app_names[i]));
		{
			write_list_of_strings(settings, "orders", "column", m_config.m_columns_setup.at(i));
		}
		settings.endGroup();

		settings.beginGroup(tr("column_sizes_%1").arg(m_config.m_app_names[i]));
		{
			QList<columns_sizes_t>::const_iterator oi = m_config.m_columns_sizes.constBegin();
			while (oi != m_config.m_columns_sizes.constEnd())
			{
				settings.beginWriteArray("sizes");
				int const size = (*oi).size();
				for (int ai = 0; ai < size; ++ai) {
					settings.setArrayIndex(ai);
					settings.setValue("column", QString::number((*oi).at(ai)));
				}
				settings.endArray();
				++oi;
			}
		}
		settings.endGroup();

		settings.beginGroup(tr("column_align_%1").arg(m_config.m_app_names[i]));
		{
			write_list_of_strings(settings, "aligns", "column", m_config.m_columns_align.at(i));
		}
		settings.endGroup();

		settings.beginGroup(tr("column_elide_%1").arg(m_config.m_app_names[i]));
		{
			write_list_of_strings(settings, "elides", "column", m_config.m_columns_elide.at(i));
		}
		settings.endGroup();
	}

#ifdef WIN32
	settings.setValue("hotkeyCode", m_config.m_hotkey);
#endif
}

void MainWindow::loadState ()
{
	qDebug("%s", __FUNCTION__);
	m_config.m_app_names.clear();
	m_config.m_columns_setup.clear();
	m_config.m_columns_sizes.clear();
	m_config.loadHistory();
	//updateSearchHistory();

	QSettings settings("MojoMir", "TraceServer");
	restoreGeometry(settings.value("geometry").toByteArray());
	restoreState(settings.value("windowState").toByteArray());
	int const pane_val = settings.value("filterPaneComboBox", 0).toInt();
	/*ui_settings->filterPaneComboBox->setCurrentIndex(pane_val);
	if (settings.contains("splitter"))
	{
		//ui->splitter->restoreState(settings.value("splitter").toByteArray());
		//ui->splitter->setOrientation(pane_val ? Qt::Vertical : Qt::Horizontal);
	}*/

	ui_settings->traceStatsCheckBox->setChecked(settings.value("trace_stats", true).toBool());

	ui_settings->reuseTabCheckBox->setChecked(settings.value("reuseTabCheckBox", true).toBool());

	ui->tableSlider->setValue(settings.value("tableSlider", 0).toInt());
	ui->plotSlider->setValue(settings.value("plotSlider", 0).toInt());
	ui->ganttSlider->setValue(settings.value("ganttSlider", 0).toInt());
	ui->buffCheckBox->setChecked(settings.value("buffCheckBox", true).toBool());


	//@TODO: delete filterMode from registry if exists
	if (m_start_level == -1)
	{
		qDebug("reading saved level from cfg");
		ui->levelSpinBox->setValue(settings.value("levelSpinBox", 3).toInt());
	}
	else
	{
		qDebug("reading level from command line");
		ui->levelSpinBox->setValue(m_start_level);
	}


	// @TODO: old code
	read_list_of_strings(settings, "known-applications", "application", m_config.m_app_names);
	for (int i = 0, ie = m_config.m_app_names.size(); i < ie; ++i)
	{
		m_config.m_columns_setup.push_back(columns_setup_t());
		settings.beginGroup(tr("column_order_%1").arg(m_config.m_app_names[i]));
		{
			read_list_of_strings(settings, "orders", "column", m_config.m_columns_setup.back());
		}
		settings.endGroup();
		
		m_config.m_columns_sizes.push_back(columns_sizes_t());
		settings.beginGroup(tr("column_sizes_%1").arg(m_config.m_app_names[i]));
		{
			int const size = settings.beginReadArray("sizes");
			for (int i = 0; i < size; ++i) {
				settings.setArrayIndex(i);
				m_config.m_columns_sizes.back().push_back(settings.value("column").toInt());
			}
			settings.endArray();
		}
		settings.endGroup();

		m_config.m_columns_align.push_back(columns_align_t());
		settings.beginGroup(tr("column_align_%1").arg(m_config.m_app_names[i]));
		{
			read_list_of_strings(settings, "aligns", "column", m_config.m_columns_align.back());
		}
		settings.endGroup();

		if (m_config.m_columns_align.back().size() < m_config.m_columns_sizes.back().size())
			for (int i = 0, ie = m_config.m_columns_sizes.back().size(); i < ie; ++i)
				m_config.m_columns_align.back().push_back(QString("L"));

		m_config.m_columns_elide.push_back(columns_elide_t());
		settings.beginGroup(tr("column_elide_%1").arg(m_config.m_app_names[i]));
		{
			read_list_of_strings(settings, "elides", "column", m_config.m_columns_elide.back());
		}
		settings.endGroup();

		if (m_config.m_columns_elide.back().size() < m_config.m_columns_sizes.back().size())
			for (int i = 0, ie = m_config.m_columns_sizes.back().size(); i < ie; ++i)
				m_config.m_columns_elide.back().push_back(QString("R"));
	}

	if (m_config.m_thread_colors.empty())
	{
		for (size_t i = Qt::white; i < Qt::transparent; ++i)
			m_config.m_thread_colors.push_back(QColor(static_cast<Qt::GlobalColor>(i)));
	}

#ifdef WIN32
	unsigned const hotkeyCode = settings.value("hotkeyCode").toInt();
	m_config.m_hotkey = hotkeyCode ? hotkeyCode : VK_SCROLL;
	DWORD const hotkey = m_config.m_hotkey;
	int mod = 0;
	UnregisterHotKey(getHWNDForWidget(this), 0);
	RegisterHotKey(getHWNDForWidget(this), 0, mod, LOBYTE(hotkey));
#endif

	loadPresets();
	QString const pname = settings.value("presetComboBox").toString();
	ui->presetComboBox->setCurrentIndex(ui->presetComboBox->findText(pname));

	ui->dockedWidgetsToolButton->setChecked(m_dock_mgr.m_docked_widgets->isVisible());
	qApp->installEventFilter(this);
}


void MainWindow::iconActivated (QSystemTrayIcon::ActivationReason reason)
{
	switch (reason) {
		case QSystemTrayIcon::Trigger:     break;
		case QSystemTrayIcon::DoubleClick: onHotkeyShowOrHide(); break;
		case QSystemTrayIcon::MiddleClick: break;
		default: break;
	}
}

void MainWindow::closeEvent (QCloseEvent * event)
{
	storeState();

	m_config.m_hidden = true;
	hide();
	event->ignore();
}

void MainWindow::changeEvent (QEvent * e) { QMainWindow::changeEvent(e); }
bool MainWindow::eventFilter (QObject * target, QEvent * e)
{
	if (e->type() == QEvent::Shortcut)
	{
		QShortcutEvent * se = static_cast<QShortcutEvent *>(e);
		if (se->key() == QKeySequence(Qt::ControlModifier + Qt::Key_Insert))
		{














			//onCopyToClipboard();
			return true;
		}
	}
	return false;
}

void MainWindow::addNewApplication (QString const & appname)
{
	m_config.m_app_names.push_back(appname);
	/*m_config.m_columns_setup.push_back(columns_setup_t());
	m_config.m_columns_sizes.push_back(columns_sizes_t());
	m_config.m_columns_align.push_back(columns_align_t());
	m_config.m_columns_elide.push_back(columns_elide_t());*/
}

int MainWindow::createAppName (QString const & appname, E_SrcProtocol const proto)
{
	addNewApplication(appname);
	int const app_idx = static_cast<int>(m_config.m_app_names.size()) - 1;

	/*if (proto == e_Proto_TLV)
	{
		size_t const n = tlv::tag_bool;
		for (int i = tlv::tag_time; i < n; ++i)
		{
			char const * name = tlv::get_tag_name(i);
			if (name)
			{
				m_config.m_columns_setup.back().push_back(QString::fromLatin1(name));
				m_config.m_columns_sizes.back().push_back(default_sizes[i]);
				m_config.m_columns_align.back().push_back(QChar(alignToString(default_aligns[i])));
				m_config.m_columns_elide.back().push_back(QChar(elideToString(default_elides[i])));
			}
		}
		onSetup(proto, static_cast<int>(app_idx), true, true);
	}
	else if (proto == e_Proto_CSV)
	{
	}*/

	return app_idx;
}

