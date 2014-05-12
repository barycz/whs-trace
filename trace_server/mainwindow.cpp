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
#include <QClipboard>
#include <QTimer>
#include <tlv_parser/tlv_parser.h>
#include "help.h"
#include "version.h"
#include "constants.h"
#include "dockwidget.h"
#include <ui_controlbarcommon.h>
#include "utils.h"
#include "utils_qstandarditem.h"
#include "utils_qsettings.h"
#include "utils_history.h"
#include "qt_plugins.h"

MainWindow::MainWindow (QWidget * parent, bool quit_delay, bool dump_mode, QString const & log_name, int level)
	: QMainWindow(parent)
	, m_time_units(0.001f)
	, ui(new Ui::MainWindow)
	, ui_settings(0)
	, m_help(new Ui::HelpDialog)
	, m_timer(new QTimer(this))
	, m_server(0)
	, m_windows_menu(0)
	, m_minimize_action(0)
	, m_maximize_action(0)
	, m_restore_action(0)
	, m_quit_action(0)
	, m_tray_menu(0)
	, m_tray_icon(0)
	, m_settings_dialog(0)
	, m_dock_mgr(this, QStringList(QString(g_traceServerName)))
	, m_docked_name(g_traceServerName)
	, m_log_name(log_name)
	, m_appdir(QDir::homePath() + "/" + g_traceServerDirName)
	, m_start_level(level)
{
	qDebug("================================================================================");
	qDebug("%s this=0x%08x", __FUNCTION__, this);
	ui->setupUi(this);

    setStyleSheet( "QMainWindow::separator { background: rgb(150, 150, 150); width: 1px; height: 1px; }");

	m_settings_dialog = new QDialog(this);
	m_settings_dialog->setWindowFlags(Qt::Sheet);
	ui_settings = new Ui::SettingsDialog();
	ui_settings->setupUi(m_settings_dialog);

	m_config.m_appdir = m_appdir;
	m_config.m_dump_mode = dump_mode;
	loadConfig();
	setConfigValuesToUI(m_config);

	// tray stuff
	createTrayActions();
	createTrayIcon();
	QIcon icon(":images/Icon1.ico");
	setWindowIcon(icon);
	m_tray_icon->setVisible(true);
	m_tray_icon->show();

	setAcceptDrops(true);
	setDockNestingEnabled(true);
	setAnimated(false);

	bool const on_top = m_config.m_on_top;
	if (on_top)
	{
		onOnTop(on_top);
	}

	m_server = new Server(m_config.m_trace_addr, m_config.m_trace_port, this, quit_delay);
	connect(m_server, SIGNAL(newConnection(Connection *)), this, SLOT(newConnection(Connection *)));
	connect(m_server, SIGNAL(statusChanged(QString const & status)), this, SLOT(onStatusChanged(QString const & status)));
	showServerStatus();

	m_timer->setInterval(5000);
	connect(m_timer, SIGNAL(timeout()) , this, SLOT(onTimerHit()));
	m_timer->start();
	setupMenuBar();

	connect(ui->dockManagerButton, SIGNAL(clicked()), this, SLOT(onDockManagerButton()));
	connect(ui->saveButton, SIGNAL(clicked()), this, SLOT(onSave()));
	connect(ui->saveAsButton, SIGNAL(clicked()), this, SLOT(onSaveAs()));

	connect(m_dock_mgr.controlUI()->levelSpinBox, SIGNAL(valueChanged(int)), this, SLOT(onLevelValueChanged(int)));
	connect(m_dock_mgr.controlUI()->buffCheckBox, SIGNAL(stateChanged(int)), this, SLOT(onBufferingStateChanged(int)));
	connect(m_dock_mgr.controlUI()->presetComboBox, SIGNAL(activated(int)), this, SLOT(onPresetChanged(int)));
	connect(m_dock_mgr.controlUI()->presetComboBox->lineEdit(), SIGNAL(returnPressed()), this, SLOT(onPresetAdd()));
	connect(m_dock_mgr.controlUI()->activatePresetButton, SIGNAL(clicked()), this, SLOT(onPresetApply()));
	connect(m_dock_mgr.controlUI()->presetSaveButton, SIGNAL(clicked()), this, SLOT(onPresetSave()));
	connect(m_dock_mgr.controlUI()->presetAddButton, SIGNAL(clicked()), this, SLOT(onPresetAdd()));
	connect(m_dock_mgr.controlUI()->presetRmButton, SIGNAL(clicked()), this, SLOT(onPresetRm()));
	connect(m_dock_mgr.controlUI()->presetResetButton, SIGNAL(clicked()), this, SLOT(onPresetReset()));
	connect(m_dock_mgr.controlUI()->logSlider, SIGNAL(valueChanged(int)), this, SLOT(onLogsStateChanged(int)));
	connect(m_dock_mgr.controlUI()->plotSlider, SIGNAL(valueChanged(int)), this, SLOT(onPlotsStateChanged(int)));
	connect(m_dock_mgr.controlUI()->tableSlider, SIGNAL(valueChanged(int)), this, SLOT(onTablesStateChanged(int)));
	connect(m_dock_mgr.controlUI()->ganttSlider, SIGNAL(valueChanged(int)), this, SLOT(onGanttsStateChanged(int)));

	/// status bar
	m_status_label = new QLabel(m_server->getStatus());
	QString human_version(g_Version);
	human_version.chop(human_version.size() - human_version.lastIndexOf(QChar('-')));
	QLabel * version_label = new QLabel(tr("Ver: %1").arg(human_version));
	statusBar()->addPermanentWidget(version_label);
	statusBar()->addWidget(m_status_label);

	QTimer::singleShot(0, this, SLOT(loadState()));	// trigger lazy load of settings
	setWindowTitle(g_traceServerName);
	setObjectName(g_traceServerName);
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

void MainWindow::createTrayActions ()
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

void MainWindow::registerHotKey ()
{
#ifdef WIN32
	DWORD const hotkey = m_config.m_hotkey;
	int mod = 0;
	UnregisterHotKey(getHWNDForWidget(this), 0);
	RegisterHotKey(getHWNDForWidget(this), 0, mod, LOBYTE(hotkey));
#endif
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

void MainWindow::onStatusChanged (QString const & status)
{
	statusBar()->showMessage(status);
	m_timer->start(5000);
}

void MainWindow::showServerStatus ()
{
	statusBar()->showMessage(m_server->getStatus());
}

void MainWindow::onTimerHit ()
{
	showServerStatus();
}

void MainWindow::onQuit ()
{
	qDebug("onQuit: hide systray, store state, qApp->quit");

	m_server->stop();

	m_tray_icon->setVisible(false);
	m_tray_icon->hide();
	storeState();

	while (!m_connections.empty())
	{
		Connection * c = m_connections.back();
		m_connections.pop_back();
		delete c;
	}

	QTimer::singleShot(0, this, SLOT(onQuitReally()));	// trigger lazy quit
}

void MainWindow::onQuitReally ()
{
	qDebug("%s", __FUNCTION__);
	qApp->quit();
}

void MainWindow::onDockManagerButton ()
{
	if (ui->dockManagerButton->isChecked())
	{
		m_dock_mgr.m_dockwidget->show();
		m_dock_mgr.m_config.m_show = true;
		restoreDockWidget(m_dock_mgr.m_dockwidget);
	}
	else
	{
		m_dock_mgr.m_dockwidget->hide();
		m_dock_mgr.m_config.m_show = false;
	}
}

void MainWindow::onDockManagerClosed ()
{
	ui->dockManagerButton->setChecked(false);
}

void MainWindow::tailFiles (QStringList const & files)
{
	for (int i = 0, ie = files.size(); i < ie; ++i)
	{
		QString const fname = files.at(i);
		if (!fname.isEmpty())
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
	createTailLogStream(m_log_name, "|");
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
	QString fname = QFileDialog::getOpenFileName(this, tr("Open File"), "", tr("Trace File(s) (*.%1)").arg(g_traceFileExtTLV));
	QStringList files;
	files << fname;
	openFiles(files);
}

void MainWindow::onSaveData ()
{
	QString const dir = QFileDialog::getExistingDirectory(this, tr("Save data to directory"), "", QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
	if (dir.isEmpty())
		return;

	if (mkPath(dir))
	{
		for (connections_t::iterator it = m_connections.begin(), ite = m_connections.end(); it != ite; ++it)
			(*it)->onSaveData(dir);
	}
}

void MainWindow::onExportDataToCSV ()
{
	QString const dir = QFileDialog::getExistingDirectory(this, tr("Export csv to directory"), "", QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
	if (dir.isEmpty())
		return;

	if (mkPath(dir))
	{
		for (connections_t::iterator it = m_connections.begin(), ite = m_connections.end(); it != ite; ++it)
			(*it)->onExportDataToCSV(dir);
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
	fileMenu->addAction(tr("Traceserver log"), this, SLOT(onLogTail()), QKeySequence(Qt::ControlModifier + Qt::AltModifier + Qt::Key_L));
	fileMenu->addAction(tr("&Save data..."), this, SLOT(onSaveData()), QKeySequence(Qt::ControlModifier + Qt::Key_S));
	fileMenu->addAction(tr("&Export data ss CSV format"), this, SLOT(onExportDataToCSV()), QKeySequence(Qt::ControlModifier + Qt::ShiftModifier + Qt::Key_S));
	fileMenu->addSeparator();
	fileMenu->addAction(tr("Quit program"), this, SLOT(onQuit()), QKeySequence::Quit);

	// View
	m_windows_menu = menuBar()->addMenu(tr("&Windows"));
	//fileMenu->addAction(tr("Show &Tool Widget"), this, SLOT(onShowToolWidget()), QKeySequence(Qt::ControlModifier + Qt::Key_O));

	// Panic!
	QMenu * panicMenu = menuBar()->addMenu(tr("&Panic"));
	panicMenu->addAction(tr("&Remove configuration files"), this, SLOT(onRemoveConfigurationFiles()));

	// Edit
	//QMenu * editMenu = menuBar()->addMenu(tr("&Edit"));
/*
	editMenu->addAction(tr("Find"), this, SLOT(onFind()), QKeySequence::Find);
	//editMenu->addAction(tr("Find"), this, SLOT(onFind()),	Qt::ControlModifier + Qt::Key_F);
	editMenu->addAction(tr("Find"), this, SLOT(onFindAllRefs()), Qt::ControlModifier + Qt::ShiftModifier + Qt::Key_F);
	editMenu->addAction(tr("Find Next"), this, SLOT(onFindNext()), QKeySequence::FindNext);
	editMenu->addAction(tr("Find Prev"), this, SLOT(onFindPrev()), QKeySequence::FindPrevious);
	new QShortcut(QKeySequence(Qt::Key_Slash), this, SLOT(onFind()));
*/
	//editMenu->addAction(tr("Find and Select All"), this, SLOT(onFindAllButton()));
	//editMenu->addAction(tr("Goto Next Tag or Selection"), this, SLOT(onNextToView()));

	//editMenu->addSeparator();
	//editMenu->addAction(tr("Close Tab"), this, SLOT(onCloseCurrentTab()), QKeySequence(Qt::ControlModifier + Qt::Key_W));

	// Filter
	//QMenu * filterMenu = menuBar()->addMenu(tr("Fi&lter"));
	//filterMenu->addAction(tr("Goto file filter"), this, SLOT(onGotoFileFilter()), QKeySequence(Qt::ControlModifier + Qt::Key_F1));
	//filterMenu->addAction(tr("Goto level filter"), this, SLOT(onGotoLevelFilter()), QKeySequence(Qt::ControlModifier + Qt::Key_F2));
	//filterMenu->addAction(tr("Goto regex filter"), this, SLOT(onGotoRegexFilter()), QKeySequence(Qt::ControlModifier + Qt::Key_F3));
	//filterMenu->addAction(tr("Goto color filter"), this, SLOT(onGotoColorFilter()), QKeySequence(Qt::ControlModifier + Qt::Key_F4));

	// Tools
	//QMenu * tools = menuBar()->addMenu(tr("&Settings"));
	//tools->addAction(tr("&Options"), this, SLOT(onSetupAction()), QKeySequence(Qt::AltModifier + Qt::ShiftModifier + Qt::Key_O));
	//tools->addAction(tr("Save Current Filter As..."), this, SLOT(onSaveCurrentFileFilter()));
	//tools->addSeparator();
	//tools->addAction(tr("Save options now (this will NOT save presets)"), this, SLOT(storeState()), QKeySequence(Qt::AltModifier + Qt::ControlModifier + Qt::ShiftModifier + Qt::Key_O));

	// Help
	//QMenu * helpMenu = menuBar()->addMenu(tr("&Help"));
	//helpMenu->addAction(tr("Help"), this, SLOT(onShowHelp()));
	//helpMenu->addAction(tr("Dump filters"), this, SLOT(onDumpFilters()));

	//new QShortcut(QKeySequence(Qt::AltModifier + Qt::Key_Space), this, SLOT(onAutoScrollHotkey()));
}

void MainWindow::addWindowAction (QAction * action)
{
	m_windows_menu->addAction(action);
}
void MainWindow::rmWindowAction (QAction * action)
{
	m_windows_menu->removeAction(action);
}

void MainWindow::iconActivated (QSystemTrayIcon::ActivationReason reason)
{
	switch (reason) {
		case QSystemTrayIcon::Trigger:	   break;
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
	/*if (e->type() == QEvent::Shortcut) {
		QShortcutEvent * se = static_cast<QShortcutEvent *>(e);
		if (se->key() == QKeySequence(Qt::ControlModifier + Qt::Key_Insert)) {
			//onCopyToClipboard();
			return true;
		}
	}*/
	return false;
}
void MainWindow::keyPressEvent (QKeyEvent * e)
{
	if (e->type() == QKeyEvent::KeyPress)
	{
		/*if (e->matches(QKeySequence::Copy))
			e->accept();
		else if (e->key() == Qt::Key_Escape)
			if (m_find_widget && m_find_widget->isVisible()) {
				m_find_widget->onCancel();
				e->accept();
			}*/
	}
	QMainWindow::keyPressEvent(e);
}

void MainWindow::onRemoveConfigurationFiles ()
{
	//QString const path = m_appdir; // @NOTE: rather not.. will not risk deletion of other files than mine
	QString const path = QDir::homePath() + "/" + g_traceServerDirName;
	if (path.isEmpty())
		return;
	QMessageBox msg_box;
	QPushButton * b_del = msg_box.addButton(tr("Yes, Delete"), QMessageBox::ActionRole);
	QPushButton * b_abort = msg_box.addButton(QMessageBox::Abort);
	msg_box.exec();
	if (msg_box.clickedButton() == b_abort)
		return;

	QDir d_out(path);
	d_out.removeRecursively();

	QDir d_new;
	d_new.mkpath(path);
}


