#include "logwidget.h"
//#include <QScrollBar>
#include <QClipboard>
#include <connection.h>
#include <utils.h>
#include <utils_qstandarditem.h>
#include "logdelegate.h"
#include <syncwidgets.h>
#include "logtablemodel.h"
#include "filterproxymodel.h"
#include "findproxymodel.h"
#include "warnimage.h"
#include <serialize.h>
#include <QInputDialog>
#include <QFontDialog>
#include <QShortcut>
#include <controlbarlog.h>
#include <ui_controlbarlog.h>


namespace logs {

	LogTableView::LogTableView (Connection * conn, LogWidget & logwidget, LogConfig & config)
		: TableView(0)
		, m_connection(conn)
		, m_log_widget(logwidget)
		, m_config(config)
	{
		setAutoScroll(false);
		setHorizontalScrollMode(ScrollPerPixel);
		horizontalHeader()->setSectionsMovable(true);

		setStyleSheet("QTableView::item{ selection-background-color: #F5DEB3  } QTableView::item{ selection-color: #000000 }");

		// to ignore 'resizeColumnToContents' when accidentaly double-clicked on header handle
		QObject::disconnect(horizontalHeader(), SIGNAL(sectionHandleDoubleClicked(int)), this, SLOT(resizeColumnToContents(int)));

		setObjectName(QString::fromUtf8("LogTableView"));

		verticalHeader()->setFont(m_config.m_font);
		verticalHeader()->setDefaultSectionSize(m_config.m_row_width);
		verticalHeader()->hide(); // @NOTE: users want that //@NOTE2: they can't have it because of performance
		horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
	}

	LogTableView::~LogTableView ()
	{
	}

void LogTableView::keyPressEvent (QKeyEvent * e)
{
	if (e->type() == QKeyEvent::KeyPress)
	{
		bool const ctrl_ins = (e->modifiers() & Qt::ControlModifier) == Qt::ControlModifier && e->key() == Qt::Key_Insert;
		if (e->matches(QKeySequence::Copy) || ctrl_ins)
		{
			m_log_widget.onCopyToClipboard();
			e->accept();
			return;
		}

		bool const ctrl = (e->modifiers() & Qt::ControlModifier) == Qt::ControlModifier;
		bool const shift = (e->modifiers() & Qt::ShiftModifier) == Qt::ShiftModifier;
		bool const alt = (e->modifiers() & Qt::AltModifier) == Qt::AltModifier;
		bool const x = e->key() == Qt::Key_X;
		bool const h = e->key() == Qt::Key_H;
		if (!ctrl && !shift && !alt && x)
		{
			m_log_widget.onExcludeFileLine();
		}
		if (!ctrl && !shift && !alt && h)
		{
			m_log_widget.onExcludeFile();
		}

		if (e->key() == Qt::Key_Escape)
		{
			if (m_log_widget.m_find_widget && m_log_widget.m_find_widget->isVisible())
			{
				m_log_widget.m_find_widget->onCancel();
				e->accept();
			}
		}
		if (!ctrl && shift && !alt && e->key() == Qt::Key_Delete)
		{
			m_log_widget.onClearAllDataButton();
			e->accept();
		}

		if (e->matches(QKeySequence::Find))
		{
			m_log_widget.onFind();
			e->accept();
		}
		if (!ctrl && !shift && !alt && e->key() == Qt::Key_Slash)
		{
			m_log_widget.onFind();
			e->accept();
		}
		if (ctrl && shift && e->key() == Qt::Key_F)
		{
			m_log_widget.onFindAllRefs();
			e->accept();
		}

		if (e->matches(QKeySequence::FindNext))
		{
			m_log_widget.onFindNext();
			e->accept();
		}
		if (e->matches(QKeySequence::FindPrevious))
		{
			m_log_widget.onFindPrev();
			e->accept();
		}
		if (e->key() == Qt::Key_Tab && m_log_widget.m_find_widget && m_log_widget.m_find_widget->isVisible())
		{
			m_log_widget.m_find_widget->focusNext();
			e->ignore();
			return;
		}

		if (e->key() == Qt::Key_Backtab && m_log_widget.m_find_widget && m_log_widget.m_find_widget->isVisible())
		{
			m_log_widget.m_find_widget->focusPrev();
			e->ignore();
			return;
		}
	}
	QTableView::keyPressEvent(e);
}

void LogTableView::scrollTo (QModelIndex const & index, ScrollHint hint)
{
	QTableView::scrollTo(index, hint);
}

void LogTableView::wheelEvent (QWheelEvent * event)
{
	bool const mod = event->modifiers() & Qt::CTRL;

	if (mod)
	{
		CursorAction const a = event->delta() < 0 ? MoveDown : MoveUp;
		moveCursor(a, Qt::ControlModifier);
		event->accept();
	}
	else
	{
		QTableView::wheelEvent(event);
	}
}


QModelIndex LogTableView::moveCursor (CursorAction cursor_action, Qt::KeyboardModifiers modifiers)
{
	m_log_widget.autoScrollOff();
	if (modifiers & Qt::ControlModifier)
	{
		if (cursor_action == MoveHome)
		{
			scrollToTop();
			return QModelIndex(); // @FIXME: should return valid value
		}
		else if (cursor_action == MoveEnd)
		{
			scrollToBottom();
			m_log_widget.autoScrollOn();
			return QModelIndex(); // @FIXME too
		}
		else
		{
			QModelIndex const idx = QTableView::moveCursor(cursor_action, modifiers);
			return idx;
		}
	}
	else if (modifiers & Qt::AltModifier)
	{
		QModelIndex const curr_idx = QTableView::moveCursor(cursor_action, modifiers);
		if (curr_idx.isValid())
			setCurrentIndex(curr_idx);
		QModelIndex mod_idx = curr_idx;
		if (m_log_widget.isModelProxy())
			mod_idx = m_log_widget.m_proxy_model->mapToSource(curr_idx);

		unsigned long long const t = m_log_widget.m_src_model->row_stime(mod_idx.row());

		emit requestSynchronization(e_SyncServerTime, m_config.m_sync_group, t, this);
		scrollTo(curr_idx, QAbstractItemView::PositionAtCenter);
		//qDebug("table: pxy findNearestTime curr_idx=(%i, %i)  mod_idx=(%i, %i)", curr_idx.column(), curr_idx.row(), mod_idx.column(), mod_idx.row());
		return curr_idx;
	}
	else
	{
		//int const value = horizontalScrollBar()->value();
		QModelIndex const idx = QTableView::moveCursor(cursor_action, modifiers);
		scrollTo(idx);
		//horizontalScrollBar()->setValue(value);
		return idx;
	}
}

	LogWidget::LogWidget (Connection * conn, QString const & fname, QStringList const & path)
		: DockedWidgetBase(path)
		, m_connection(conn)
		, m_tableview(0)
		, m_cacheLayout(0)
		, m_gotoPrevErrButton(0) , m_gotoNextErrButton(0) , m_gotoPrevWarnButton(0) , m_gotoNextWarnButton(0)
		, m_excludeFileButton(0) , m_excludeFileLineButton(0) , m_excludeRowButton(0) , m_locateRowButton(0)
		, m_clrDataButton(0) , m_setRefTimeButton(0) , m_hidePrevButton(0) , m_hideNextButton(0)
		, m_colorRowButton(0) , m_colorFileLineButton(0) , m_uncolorRowButton(0) , m_gotoPrevColorButton(0) , m_gotoNextColorButton(0)
		, m_control_bar(0)
		, m_config()
		, m_config_ui(*this, this)
		, m_fname(fname)
		, m_warnimage(0)
		, m_filter_state()
		, m_tagconfig()
		, m_tags2columns()
		, m_tls()
		, m_time_ref_value(0)
		, m_proxy_model(0)
		, m_find_proxy_model(0)
		, m_src_model(0)
		, m_selection(0)
		, m_ksrc_selection(0)
		, m_kproxy_selection(0)
		, m_src_selection(0)
		, m_proxy_selection(0)
		, m_find_proxy_selection(0)
		, m_kfind_proxy_selection(0)
		, m_color_regex_model(0)
		, m_find_widget(0)
		, m_linked_parent(0)
		, m_csv_separator()
		, m_file_csv_stream(0)
		//, m_file_tlv_stream(0)


	{
		m_queue.reserve(256);
		m_tableview = new LogTableView(conn, *this, m_config);

		QVBoxLayout * vLayout = new QVBoxLayout();
		vLayout->setSpacing(1);
		vLayout->setContentsMargins(0, 0, 0, 0);
		vLayout->setObjectName(QString::fromUtf8("verticalLayout"));
		setLayout(vLayout);
		QWidget * cacheWidget = new QFrame();
		ButtonCache * cacheLayout = new ButtonCache();
		cacheLayout->setSpacing(1);
		cacheLayout->setContentsMargins(0, 0, 0, 0);
		cacheLayout->setObjectName(QString::fromUtf8("cacheLayout"));
		cacheWidget->setLayout(cacheLayout);
		vLayout->addWidget(cacheWidget);
		vLayout->addWidget(m_tableview);
		fillButtonCache();

		m_find_widget->setActionAbleWidget(this);

		setContextMenuPolicy(Qt::CustomContextMenu);
		connect(m_tableview, SIGNAL(customContextMenuRequested(QPoint const &)), this, SLOT(onShowContextMenu(QPoint const &)));

		m_control_bar = new ControlBarLog(0); // @TODO: delete

		setConfigValuesToUI(m_config);

		connect(&getSyncWidgets(), SIGNAL( requestSynchronization(E_SyncMode, int, unsigned long long, void *) ),
							 this, SLOT( performSynchronization(E_SyncMode, int, unsigned long long, void *) ));
		connect(this, SIGNAL( requestSynchronization(E_SyncMode, int, unsigned long long, void *) ),
							 &getSyncWidgets(), SLOT( performSynchronization(E_SyncMode, int, unsigned long long, void *) ));


		setObjectName(QString::fromUtf8("LogWidget"));
		//setupThreadColors(connection->getMainWindow()->getThreadColors());

		QStyle const * const style = QApplication::style();
		m_config_ui.ui()->findWidget->setMainWindow(m_connection->getMainWindow());
		connect(m_config_ui.ui()->gotoNextButton, SIGNAL(clicked()), this, SLOT(onNextToView()));
		m_config_ui.ui()->gotoNextButton->setIcon(style->standardIcon(QStyle::SP_ArrowDown));
		connect(m_config_ui.ui()->saveButton, SIGNAL(clicked()), this, SLOT(onSaveButton()));
		//connect(ui->autoScrollCheckBox, SIGNAL(stateChanged(int)), this, SLOT(onAutoScrollStateChanged(int)));
		//connect(ui->inViewCheckBox, SIGNAL(stateChanged(int)), this, SLOT(onInViewStateChanged(int)));
		//connect(m_config_ui.ui()->filterFileCheckBox, SIGNAL(stateChanged(int)), this, SLOT(onFilterFile(int)));
		connect(m_tableview, SIGNAL(clicked(QModelIndex const &)), this, SLOT(onTableClicked(QModelIndex const &)));
		connect(m_tableview, SIGNAL(doubleClicked(QModelIndex const &)), this, SLOT(onTableDoubleClicked(QModelIndex const &)));

		QObject::connect(m_tableview->horizontalHeader(), SIGNAL(sectionResized(int, int, int)), this, SLOT(onSectionResized(int, int, int)));
		QObject::connect(m_tableview->horizontalHeader(), SIGNAL(sectionMoved(int, int, int)), this, SLOT(onSectionMoved(int, int, int)));
		m_tableview->setItemDelegate(new LogDelegate(*this, m_connection->appData(), this));
		
		m_warnimage = new WarnImage(this);
		m_find_widget = new FindWidget(m_connection->getMainWindow(), this);
		m_find_widget->setParent(this);
	}

	void LogWidget::fillButtonCache ()
	{
		QWidget * parent_widget = this;

		QFrame * line = 0;
		QFrame * line_3 = 0;
		QFrame * line_2 = 0;
		QSpacerItem * horizontalSpacer_3 = 0;
		ButtonCache * cacheLayout = m_cacheLayout;
 
		m_gotoPrevErrButton = new QToolButton(parent_widget);
		m_gotoPrevErrButton->setObjectName(QStringLiteral("gotoPrevErrButton"));
		m_gotoPrevErrButton->setMaximumSize(QSize(16777215, 16));
		m_gotoPrevErrButton->setCheckable(false);
		m_gotoPrevErrButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
		m_gotoPrevErrButton->setArrowType(Qt::UpArrow);
		cacheLayout->addWidget(m_gotoPrevErrButton);
		m_gotoNextErrButton = new QToolButton(parent_widget);
		m_gotoNextErrButton->setObjectName(QStringLiteral("gotoNextErrButton"));
		m_gotoNextErrButton->setMaximumSize(QSize(16777215, 16));
		m_gotoNextErrButton->setCheckable(false);
		m_gotoNextErrButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
		m_gotoNextErrButton->setArrowType(Qt::DownArrow);
		cacheLayout->addWidget(m_gotoNextErrButton);

		m_gotoPrevWarnButton = new QToolButton(parent_widget);
		m_gotoPrevWarnButton->setObjectName(QStringLiteral("gotoPrevWarnButton"));
		m_gotoPrevWarnButton->setMaximumSize(QSize(16777215, 16));
		m_gotoPrevWarnButton->setCheckable(false);
		m_gotoPrevWarnButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
		m_gotoPrevWarnButton->setArrowType(Qt::UpArrow);
		cacheLayout->addWidget(m_gotoPrevWarnButton);
		m_gotoNextWarnButton = new QToolButton(parent_widget);
		m_gotoNextWarnButton->setObjectName(QStringLiteral("gotoNextWarnButton"));
		m_gotoNextWarnButton->setMaximumSize(QSize(16777215, 16));
		m_gotoNextWarnButton->setCheckable(false);
		m_gotoNextWarnButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
		m_gotoNextWarnButton->setArrowType(Qt::DownArrow);
		cacheLayout->addWidget(m_gotoNextWarnButton);

		QFrame * line4 = new QFrame(parent_widget);
		line4->setObjectName(QStringLiteral("line4"));
		line4->setMinimumSize(QSize(3, 0));
		line4->setFrameShape(QFrame::VLine);
		//line4->setFrameShadow(QFrame::Sunken);
		cacheLayout->addWidget(line4);

		m_excludeFileButton = new QToolButton(parent_widget);
		m_excludeFileButton->setObjectName(QStringLiteral("excludeFileButton"));
		m_excludeFileButton->setMinimumSize(QSize(16, 0));
		m_excludeFileButton->setMaximumSize(QSize(16777215, 16));
		cacheLayout->addWidget(m_excludeFileButton);

		m_excludeFileLineButton = new QToolButton(parent_widget);
		m_excludeFileLineButton->setObjectName(QStringLiteral("excludeFileLineButton"));
		m_excludeFileLineButton->setMinimumSize(QSize(16, 0));
		m_excludeFileLineButton->setMaximumSize(QSize(16777215, 16));
		cacheLayout->addWidget(m_excludeFileLineButton);

		m_excludeRowButton = new QToolButton(parent_widget);
		m_excludeRowButton->setObjectName(QStringLiteral("excludeRowButton"));
		QSizePolicy sizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
		sizePolicy.setHorizontalStretch(0);
		sizePolicy.setVerticalStretch(0);
		sizePolicy.setHeightForWidth(m_excludeRowButton->sizePolicy().hasHeightForWidth());
		m_excludeRowButton->setSizePolicy(sizePolicy);
		m_excludeRowButton->setMinimumSize(QSize(0, 0));
		m_excludeRowButton->setMaximumSize(QSize(16777215, 16));
		cacheLayout->addWidget(m_excludeRowButton);

		m_locateRowButton = new QToolButton(parent_widget);
		m_locateRowButton->setObjectName(QStringLiteral("locateRowButton"));
		m_locateRowButton->setMaximumSize(QSize(16777215, 16));
		cacheLayout->addWidget(m_locateRowButton);

		m_timeComboBox = new TimeComboBox(parent_widget);
		m_timeComboBox->setObjectName(QStringLiteral("timeComboBox"));
		m_timeComboBox->setMaximumSize(QSize(16777215, 16));
		cacheLayout->addWidget(m_timeComboBox);

		line = new QFrame(parent_widget);
		line->setObjectName(QStringLiteral("line"));
		line->setMinimumSize(QSize(5, 0));
		line->setMaximumSize(QSize(10, 0));
		line->setFrameShape(QFrame::VLine);
		//line->setFrameShadow(QFrame::Sunken);
		cacheLayout->addWidget(line);

		m_clrDataButton = new QToolButton(parent_widget);
		m_clrDataButton->setObjectName(QStringLiteral("clrData"));
		m_clrDataButton->setMaximumSize(QSize(16777215, 16));
		cacheLayout->addWidget(m_clrDataButton);


		line = new QFrame(parent_widget);
		line->setObjectName(QStringLiteral("line"));
		line->setMinimumSize(QSize(5, 0));
		line->setFrameShape(QFrame::VLine);
		//line->setFrameShadow(QFrame::Sunken);
		cacheLayout->addWidget(line);

		m_setRefTimeButton = new QToolButton(parent_widget);
		m_setRefTimeButton->setObjectName(QStringLiteral("setRefTimeButton"));
		m_setRefTimeButton->setMaximumSize(QSize(16777215, 16));
		m_setRefTimeButton->setCheckable(true);
		cacheLayout->addWidget(m_setRefTimeButton);

		m_hidePrevButton = new QToolButton(parent_widget);
		m_hidePrevButton->setObjectName(QStringLiteral("hidePrevButton"));
		m_hidePrevButton->setMaximumSize(QSize(16777215, 16));
		m_hidePrevButton->setCheckable(true);
		m_hidePrevButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
		m_hidePrevButton->setArrowType(Qt::UpArrow);
		cacheLayout->addWidget(m_hidePrevButton);

		m_hideNextButton = new QToolButton(parent_widget);
		m_hideNextButton->setObjectName(QStringLiteral("hideNextButton"));
		m_hideNextButton->setMaximumSize(QSize(16777215, 16));
		m_hideNextButton->setCheckable(true);
		m_hideNextButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
		m_hideNextButton->setArrowType(Qt::DownArrow);
		cacheLayout->addWidget(m_hideNextButton);

		//line_2 = new QLine(parent_widget);
		//line_2->setObjectName(QStringLiteral("line_2"));
		//line_2->setMinimumSize(QSize(3, 0));
		//line_2->setFrameShape(QFrame::VLine);
		//line_2->setFrameShadow(QFrame::Sunken);
		//cacheLayout->addWidget(line_2);

		m_colorRowButton = new QToolButton(parent_widget);
		m_colorRowButton->setObjectName(QStringLiteral("colorRowButton"));
		m_colorRowButton->setMaximumSize(QSize(16777215, 16));
		cacheLayout->addWidget(m_colorRowButton);

		m_gotoPrevColorButton = new QToolButton(parent_widget);
		m_gotoPrevColorButton->setObjectName(QStringLiteral("gotoPrevColorButton"));
		m_gotoPrevColorButton->setMaximumSize(QSize(16777215, 16));
		m_gotoPrevColorButton->setCheckable(false);
		m_gotoPrevColorButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
		m_gotoPrevColorButton->setArrowType(Qt::UpArrow);
		cacheLayout->addWidget(m_gotoPrevColorButton);
		m_gotoNextColorButton = new QToolButton(parent_widget);
		m_gotoNextColorButton->setObjectName(QStringLiteral("gotoNextColorButton"));
		m_gotoNextColorButton->setMaximumSize(QSize(16777215, 16));
		m_gotoNextColorButton->setCheckable(false);
		m_gotoNextColorButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
		m_gotoNextColorButton->setArrowType(Qt::DownArrow);
		cacheLayout->addWidget(m_gotoNextColorButton);

		//m_colorFileLineButton = new QToolButton(parent_widget);
		//m_colorFileLineButton->setObjectName(QStringLiteral("colorFileLineButton"));
		//m_colorFileLineButton->setMaximumSize(QSize(16777215, 16));
		//cacheLayout->addWidget(m_colorFileLineButton);

		m_uncolorRowButton = new QToolButton(parent_widget);
		m_uncolorRowButton->setObjectName(QStringLiteral("uncolorRowButton"));
		m_uncolorRowButton->setMaximumSize(QSize(16777215, 16));
		cacheLayout->addWidget(m_uncolorRowButton);

		horizontalSpacer_3 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

		m_gotoPrevErrButton->setToolTip(QApplication::translate("SettingsLog", "<html><head/><body><p>Goto prev error</p><p><br/></p><p>Hotkey = <span style=\" font-weight:600;\">Shift + e</span></p></body></html>", 0));
		m_gotoPrevErrButton->setText(QApplication::translate("SettingsLog", "E", 0));
		m_gotoNextErrButton->setToolTip(QApplication::translate("SettingsLog", "<html><head/><body><p>Goto next error</p><p><br/></p><p>Hotkey = <span style=\" font-weight:600;\">e</span></p></body></html>", 0));
		m_gotoNextErrButton->setText(QApplication::translate("SettingsLog", "E", 0));

		m_gotoPrevWarnButton->setToolTip(QApplication::translate("SettingsLog", "<html><head/><body><p>Goto prev warning</p><p><br/></p><p>Hotkey = <span style=\" font-weight:600;\">Shift + w</span></p></body></html>", 0));
		m_gotoPrevWarnButton->setText(QApplication::translate("SettingsLog", "W", 0));
		m_gotoNextWarnButton->setToolTip(QApplication::translate("SettingsLog", "<html><head/><body><p>Goto next warning</p><p><br/></p><p>Hotkey = <span style=\" font-weight:600;\">x</span></p></body></html>", 0));
		m_gotoNextWarnButton->setText(QApplication::translate("SettingsLog", "W", 0));

		m_excludeFileButton->setToolTip(QApplication::translate("SettingsLog", "<html><head/><body><p>Excludes File from current selection from table. This is shortcut for going into Filter/File:Line and click on File tree item</p><p><br/></p><p>Hotkey = <span style=\" font-weight:600;\">h</span></p></body></html>", 0));
		m_excludeFileButton->setText(QApplication::translate("SettingsLog", "x F", 0));

		m_excludeFileLineButton->setToolTip(QApplication::translate("SettingsLog", "<html><head/><body><p>Excludes File:Line combination from current selection from table. This is shortcut for going into Filter/File:Line and click on item</p><p><br/></p><p>Hotkey = <span style=\" font-weight:600;\">x</span></p></body></html>", 0));
		m_excludeFileLineButton->setText(QApplication::translate("SettingsLog", "x F:L", 0));

		m_excludeRowButton->setToolTip(QApplication::translate("SettingsLog", "<html><head/><body><p>Excludes selected row via Filter/Row. This one does not use File:Line information, so it can be used to exclude specific lines while keeping the rest.</p><p>Hotkey = <span style=\" font-weight:600;\">r</span></p></body></html>", 0));
		m_excludeRowButton->setText(QApplication::translate("SettingsLog", "x ==", 0));

		m_timeComboBox->setToolTip(QApplication::translate("SettingsLog", "<html><head/><body><p>Change time units</p><p>Hotkey = <span style=\" font-weight:600;\">?</span></p></body></html>", 0));

		m_clrDataButton->setToolTip(QApplication::translate("SettingsLog", "<html><head/><body><p>Delete table data.</p><p>Hotkey = <span style=\" font-weight:600;\">Shift+DEL</span></p></body></html>", 0));
		m_clrDataButton->setText(QApplication::translate("SettingsLog", "DEL", 0));

		m_locateRowButton->setToolTip(QApplication::translate("SettingsLog", "<html><head/><body><p>Locates currently selected row in Filters/File:Line</p><p>Hotkey = <span style=\" font-weight:600;\">?</span></p></body></html>", 0));
		m_locateRowButton->setText(QApplication::translate("SettingsLog", "? ==", 0));

		m_setRefTimeButton->setToolTip(QApplication::translate("SettingsLog", "<html><head/><body><p>Set/Unset reference time (= 0) to currently selected line</p></body></html>", 0));
		m_setRefTimeButton->setText(QApplication::translate("SettingsLog", "Ref T", 0));

		m_hidePrevButton->setToolTip(QApplication::translate("SettingsLog", "<html><head/><body><p>Hide rows preceeding current selection</p></body></html>", 0));
		m_hidePrevButton->setText(QApplication::translate("SettingsLog", "Hide T", 0));
		m_hideNextButton->setToolTip(QApplication::translate("SettingsLog", "Hide lines following current selection", 0));
		m_hideNextButton->setText(QApplication::translate("SettingsLog", "Hide T", 0));
		m_colorRowButton->setText(QApplication::translate("SettingsLog", "Color ==", 0));
		//m_colorFileLineButton->setText(QApplication::translate("SettingsLog", "Color F:L", 0));
		m_uncolorRowButton->setText(QApplication::translate("SettingsLog", "Uncolor", 0));

		m_gotoPrevColorButton->setToolTip(QApplication::translate("SettingsLog", "<html><head/><body><p>Goto prev color tag</p><p><br/></p><p>Hotkey = <span style=\" font-weight:600;\"></span></p></body></html>", 0));
		m_gotoPrevColorButton->setText(QApplication::translate("SettingsLog", "", 0));
		m_gotoNextColorButton->setToolTip(QApplication::translate("SettingsLog", "<html><head/><body><p>Goto next color tag</p><p><br/></p><p>Hotkey = <span style=\" font-weight:600;\"></span></p></body></html>", 0));
		m_gotoNextColorButton->setText(QApplication::translate("SettingsLog", "", 0));

		//cacheLayout->addItem(horizontalSpacer_3);

		connect(m_gotoPrevErrButton, SIGNAL(clicked()), this, SLOT(onGotoPrevErr()));
		connect(m_gotoNextErrButton, SIGNAL(clicked()), this, SLOT(onGotoNextErr()));
		connect(m_gotoPrevWarnButton, SIGNAL(clicked()), this, SLOT(onGotoPrevWarn()));
		connect(m_gotoNextWarnButton, SIGNAL(clicked()), this, SLOT(onGotoNextWarn()));
		connect(m_excludeFileButton, SIGNAL(clicked()), this, SLOT(onExcludeFile()));
		connect(m_excludeFileLineButton, SIGNAL(clicked()), this, SLOT(onExcludeFileLine()));
		connect(m_excludeRowButton, SIGNAL(clicked()), this, SLOT(onExcludeRow()));
		connect(m_locateRowButton, SIGNAL(clicked()), this, SLOT(onLocateRow()));
		connect(m_clrDataButton, SIGNAL(clicked()), this, SLOT(onClearAllDataButton()));
		connect(m_timeComboBox->comboBox(), SIGNAL(activated(int)), this, SLOT(onChangeTimeUnits(int)));
		//connect(m_colorFileLineButton, SIGNAL(clicked()), this, SLOT(onColorFileLine()));
		connect(m_colorRowButton, SIGNAL(clicked()), this, SLOT(onColorRow()));
		connect(m_uncolorRowButton, SIGNAL(clicked()), this, SLOT(onUncolorRow()));
		connect(m_setRefTimeButton, SIGNAL(clicked()), this, SLOT(onSetRefTime()));
		connect(m_hidePrevButton, SIGNAL(clicked()), this, SLOT(onHidePrev()));
		connect(m_hideNextButton, SIGNAL(clicked()), this, SLOT(onHideNext()));
		connect(m_gotoPrevColorButton, SIGNAL(clicked()), this, SLOT(onGotoPrevColor()));
		connect(m_gotoNextColorButton, SIGNAL(clicked()), this, SLOT(onGotoNextColor()));
	}


	QModelIndex LogWidget::currentSourceIndex () const
	{
		QModelIndex current = m_tableview->currentIndex();
		if (isModelProxy())
		{
			current = m_proxy_model->mapToSource(current);
		}
		return current;
	}

	void LogWidget::setupNewLogModel ()
	{
		setupLogModel(0);
	}

	void LogWidget::setupLogModel (LogTableModel * linked_model)
	{
		if (linked_model)
			m_src_model = linked_model;
		else
		{
			m_src_model = new LogTableModel(this, *this);
			QObject::disconnect(m_src_model, SIGNAL(rowsInserted(QModelIndex,int,int)), m_tableview->verticalHeader(), SLOT(sectionsInserted(QModelIndex,int,int)));
		}

		m_proxy_model = new FilterProxyModel(this, *this);
		m_proxy_model->setSourceModel(m_src_model);

		m_find_proxy_model = new FindProxyModel(this, *this);
		m_find_proxy_model->setSourceModel(m_src_model);

		//setupLogSelectionProxy();
		if (linked_model)
		{
			QItemSelectionModel * src_selection = linked_model->m_log_widget.m_tableview->selectionModel();
			QItemSelectionModel * pxy_selection = linked_model->m_log_widget.m_proxy_selection;
			//m_proxy_selection = new QItemSelectionModel(m_proxy_model);
			//m_ksrc_selection = new KLinkItemSelectionModel(m_src_model, src_selection);
			//m_kproxy_selection = new KLinkItemSelectionModel(m_proxy_model, m_proxy_selection);
			//m_src_model = linked_model;
		}
		else
		{
			setupLogSelectionProxy();
		}
	}

	void LogWidget::setupLogSelectionProxy ()
	{
		m_src_selection = new QItemSelectionModel(m_src_model);
		m_proxy_selection = new QItemSelectionModel(m_proxy_model);
		//m_ksrc_selection = new KLinkItemSelectionModel(m_src_model, m_src_selection);
		m_kproxy_selection = new KLinkItemSelectionModel(m_proxy_model, m_proxy_selection);
		//setSelectionModel(m_src_selection);
		//m_selection = new LogSelectionProxyModel(m_src_model, m_src_selection);
	}

	LogWidget::~LogWidget ()
	{
		m_tableview->setItemDelegate(0);
		qDebug("%s this=0x%08x tag=%s", __FUNCTION__, this, m_config.m_tag.toStdString().c_str());
		disconnect(this, SIGNAL(customContextMenuRequested(QPoint const &)), this, SLOT(onShowContextMenu(QPoint const &)));

		if (m_linked_parent)
		{
			LogWidget * parent = qobject_cast<LogWidget *>(m_linked_parent);
			if (parent)
			{
				parent->unregisterLinkedWidget(this);
			}
		}

		for (linked_widgets_t::iterator it = m_linked_widgets.begin(), ite = m_linked_widgets.end(); it != ite; ++it)
		{
			DockedWidgetBase * child = *it;
			m_connection->destroyDockedWidget(child);
		}
		m_linked_widgets.clear();


	/*	if (m_file_csv_stream)
		{
			QIODevice * const f = m_file_csv_stream->device();
			f->close();
			delete m_file_csv_stream;
			m_file_csv_stream = 0;
			delete f;
		}*/
	}

	void LogWidget::onShow ()
	{
		show();
	}

	void LogWidget::onHide ()
	{
		hide();
	}

	void LogWidget::onHideContextMenu ()
	{
		Ui::SettingsLog * ui = m_config_ui.ui();
		disconnect(ui->saveButton, SIGNAL(clicked()), this, SLOT(onSaveButton()));
		m_config_ui.onHideContextMenu();
	}

	void LogWidget::onShowContextMenu (QPoint const & pos)
	{
		//qDebug("%s this=0x%08x", __FUNCTION__, this);
		m_config_ui.onShowContextMenu(QCursor::pos());
		Ui::SettingsLog * ui = m_config_ui.ui();

		setConfigValuesToUI(m_config);
		//connect(ui->logViewComboBox, SIGNAL(activated(int)), this, SLOT(onLogViewActivate(int)));
	}

	void LogWidget::swapSectionsAccordingTo (logs::LogConfig const & cfg)
	{
		QStringList src;
		int const hn = m_tableview->horizontalHeader()->count();
		for (int hi = 0; hi < hn; ++hi)
		{
			int const li = m_tableview->horizontalHeader()->logicalIndex(hi);
			QString const li_str = m_config.m_columns_setup[li]; // @note: from m_config
			src << li_str;
		}
	
		QStringList tgt;
		int const nn = cfg.m_columns_setup.size();
		for (int nj = 0; nj < nn; ++nj)
		{
			tgt << cfg.m_columns_setup[nj];
		}

		for (int nj = 0; nj < nn; ++nj)
			for (int hi = 0; hi < hn; ++hi)
				if (src[hi] == tgt[nj])
				{
					if (hi != nj)
					{
						std::swap(src[hi], src[nj]);
						m_tableview->horizontalHeader()->swapSections(hi, nj);
						break;
					}
				}

		//void LogWidget::resizeSections ()
		{
			bool const old = blockSignals(true);
			for (int c = 0, ce = cfg.m_columns_sizes.size(); c < ce; ++c)
			{
				int const li = m_tableview->horizontalHeader()->logicalIndex(c);
				m_tableview->horizontalHeader()->resizeSection(li, cfg.m_columns_sizes.at(c));
			}
			blockSignals(old);
		}
	}

	void LogWidget::applyConfig ()
	{
		filterMgr()->disconnectFiltersTo(this);
		colorizerMgr()->disconnectFiltersTo(this);

		resizeModelToConfig(m_config);
		swapSectionsAccordingTo(m_config);
		filterMgr()->applyConfig();
		colorizerMgr()->applyConfig();

		filterMgr()->connectFiltersTo(this);
		colorizerMgr()->connectFiltersTo(this);

		connect(filterMgr(), SIGNAL(filterEnabledChanged()), this, SLOT(onFilterEnabledChanged()));
		connect(filterMgr(), SIGNAL(filterChangedSignal()), this, SLOT(onInvalidateFilter()));
		// @TODO: nesel by mensi brutus nez je invalidate filter?
		connect(colorizerMgr(), SIGNAL(filterEnabledChanged()), this, SLOT(onFilterEnabledChanged()));
		connect(colorizerMgr(), SIGNAL(filterChangedSignal()), this, SLOT(onInvalidateFilter()));

		if (filterMgr()->getFilterCtx())
			filterMgr()->getFilterCtx()->setAppData(&m_connection->appData());

		if (colorizerMgr()->getColorizerRegex())
			colorizerMgr()->getColorizerRegex()->setSrcModel(m_src_model);

		if (colorizerMgr()->getColorizerRow())
			colorizerMgr()->getColorizerRow()->setSrcModel(m_src_model);

		//if (colorizerMgr()->getFilterCtx())
		//	colorizerMgr()->getFilterCtx()->setAppData(&m_connection->appData());
	}

	void LogWidget::resizeSections ()
	{
		bool const old = blockSignals(true);
		for (int c = 0, ce = m_config.m_columns_sizes.size(); c < ce; ++c)
			m_tableview->horizontalHeader()->resizeSection(c, m_config.m_columns_sizes.at(c));
		blockSignals(old);
	}

	void LogWidget::resizeModelToConfig (LogConfig & cfg)
	{
		//qDebug("%s this=0x%08x", __FUNCTION__, this);
		//Ui::SettingsLog * ui = m_config_ui.ui();
		//horizontalHeader()->resizeSections(QHeaderView::Fixed);
		//horizontalHeader()->resizeSectionItem(c, 32, );
		
		int const new_sz = cfg.m_columns_setup.size();
		int const cur_sz = m_config.m_columns_setup.size();
		if (new_sz < cur_sz)
		{
			// @TODO
		}
		else if (new_sz > cur_sz)
		{
			// @TODO
		}

		if (m_src_model)
			m_src_model->resizeToCfg(cfg);
		if (m_proxy_model)
			m_proxy_model->resizeToCfg(cfg);
		if (m_find_proxy_model)
			m_find_proxy_model->resizeToCfg(cfg);

		if (!isLinkedWidget())
			setFilteringProxy(filterMgr()->enabled());

		resizeSections();
	}

	int LogWidget::sizeHintForColumn (int column) const
	{
		int const idx = !isModelProxy() ? column : m_proxy_model->colToSource(column);
		//qDebug("table: on rsz hdr[%i -> src=%02i ]	%i->%i\t\t%s", c, idx, old_size, new_size, m_config.m_hhdr.at(idx).toStdSt
		if (idx < 0) return 64;
		int const curr_sz = m_config.m_columns_sizes.size();
		if (idx < curr_sz)
		{
			//qDebug("%s this=0x%08x hsize[%i]=%i", __FUNCTION__, this, idx, new_size);
			return m_config.m_columns_sizes[idx];
		}
		return 32;
	}

	void LogWidget::setConfigValuesToUI (LogConfig const & cfg)
	{
		//qDebug("%s this=0x%08x", __FUNCTION__, this);
		Ui::SettingsLog * ui = m_config_ui.ui();
		ui->dtCheckBox->setCheckState(cfg.m_dt_enabled ? Qt::Checked : Qt::Unchecked);
		ui->autoScrollCheckBox->setCheckState(cfg.m_auto_scroll ? Qt::Checked : Qt::Unchecked);
		
		//ui->globalShowCheckBox->blockSignals(true);
		//ui->globalShowCheckBox->setCheckState(cfg.m_show ? Qt::Checked : Qt::Unchecked);
		//ui->globalShowCheckBox->blockSignals(false);

		//ui->logViewComboBox->clear();
		/*for (size_t i = 0, ie = cfg.m_gvcfg.size(); i < ie; ++i)
		{
			LogViewConfig const & gvcfg = cfg.m_gvcfg[i];
			ui->logViewComboBox->addItem(gvcfg.m_tag);
		}
		if (cfg.m_gvcfg.size())
			setViewConfigValuesToUI(cfg.m_gvcfg[0]);*/
	}

	void LogWidget::setUIValuesToConfig (LogConfig & cfg)
	{
		//qDebug("%s this=0x%08x", __FUNCTION__, this);
		Ui::SettingsLog * ui = m_config_ui.ui();
		//m_config.m_show = ui->globalShowCheckBox->checkState() == Qt::Checked;
	}

	void LogWidget::onApplyButton ()
	{
		applyConfig();
		//setUIValuesToConfig(m_config2);
		//applyConfig();
	}

	QString LogWidget::getCurrentWidgetPath () const
	{
		QString const appdir = m_connection->getMainWindow()->getAppDir();
		QString const logpath = appdir + "/" + m_connection->getCurrPreset() + "/" + g_LogTag + "/" + m_config.m_tag;
		return logpath;
	}

	void addTagToConfig (logs::LogConfig & config, TagDesc const & td)
	{
		config.m_columns_setup.push_back(tlv::get_tag_name(td.m_tag));
		config.m_columns_sizes.push_back(td.m_size);
		config.m_columns_align.push_back(td.m_align_str);
		config.m_columns_elide.push_back(td.m_elide_str);
	}
	void LogWidget::reconfigureConfig (logs::LogConfig & config)
	{
		fillDefaultConfig(config);
		tlv::tag_t const tags[] = {
				tlv::tag_stime
			, tlv::tag_ctime
			, tlv::tag_tid
			, tlv::tag_lvl
			, tlv::tag_ctx
			, tlv::tag_file
			, tlv::tag_line
			, tlv::tag_func
			, tlv::tag_msg
		};
		for (size_t i = 0, ie = sizeof(tags)/sizeof(*tags); i < ie; ++i)
		{
			TagDesc const & td = m_tagconfig.findOrCreateTag(tags[i]);
			addTagToConfig(config, td);
		}
	}
	void LogWidget::defaultConfigFor (logs::LogConfig & config)
	{
		QString const & appname = m_connection->getAppName();
		if (!validateConfig(config))
			reconfigureConfig(config);
	}

	void LogWidget::loadConfig (QString const & preset_dir)
	{
		QString const tag_backup = m_config.m_tag;
		QString const logpath = preset_dir + "/" + g_LogTag + "/" + m_config.m_tag + "/";
		m_config.clear();
		bool const loaded = loadConfigTemplate(m_config, logpath + g_LogFile);
		if (!loaded)
		{
			defaultConfigFor(m_config);
			m_config.m_tag = tag_backup; // defaultConfigFor destroys tag
		}

		loadAuxConfigs();
	}
	void LogWidget::loadAuxConfigs ()
	{
		QString const logpath = getCurrentWidgetPath();
		m_config.m_find_config.clear();
		loadConfigTemplate(m_config.m_find_config, logpath + "/" + g_findTag);
		filterMgr()->loadConfig(logpath);
		colorizerMgr()->loadConfig(logpath);
	}
	void LogWidget::saveAuxConfigs ()
	{
		QString const logpath = getCurrentWidgetPath();
		saveConfigTemplate(m_config.m_find_config, logpath + "/" + g_findTag);
		filterMgr()->saveConfig(logpath);
		colorizerMgr()->saveConfig(logpath);
	}
	void LogWidget::saveFindConfig ()
	{
		QString const logpath = getCurrentWidgetPath();
		saveConfigTemplate(m_config.m_find_config, logpath + "/" + g_findTag);
	}

	void LogWidget::normalizeConfig (logs::LogConfig & normalized)
	{
		QMap<int, int> perms;
		int const n = m_tableview->horizontalHeader()->count();
		for (int i = 0; i < n; ++i)
		{
			int const currentVisualIndex = m_tableview->horizontalHeader()->visualIndex(i);
			if (currentVisualIndex != i)
			{
				perms.insert(i, currentVisualIndex);
			}
		}

		normalized.m_columns_setup.resize(n);
		normalized.m_columns_sizes.resize(n);
		normalized.m_columns_align.resize(n);
		normalized.m_columns_elide.resize(n);

		QMapIterator<int, int> iter(perms);
		while (iter.hasNext())
		{
			iter.next();
			int const logical = iter.key();
			int const visual = iter.value();

			normalized.m_columns_setup[visual] = m_config.m_columns_setup[logical];
			normalized.m_columns_sizes[visual] = m_config.m_columns_sizes[logical];
			normalized.m_columns_align[visual] = m_config.m_columns_align[logical];
			normalized.m_columns_elide[visual] = m_config.m_columns_elide[logical];
		}
	}

	void LogWidget::saveConfig (QString const & path)
	{
		QString const logpath = getCurrentWidgetPath();
		mkDir(logpath);

		logs::LogConfig tmp = m_config;
		normalizeConfig(tmp);
		logs::saveConfig(tmp, logpath + "/" + g_LogFile);
		saveAuxConfigs();
	}

	void LogWidget::onSaveButton ()
	{
		/*m_config.m_hsize.clear();
		m_config.m_hsize.resize(m_modelView->columnCount());
		for (int i = 0, ie = m_modelView->columnCount(); i < ie; ++i)
			m_config.m_hsize[i] = horizontalHeader()->sectionSize(i);*/
		//saveConfig();
		//m_pers_filter.saveConfig(
	}
	void LogWidget::onResetButton () { setConfigValuesToUI(m_config); }
	void LogWidget::onDefaultButton ()
	{
		LogConfig defaults;
		//defaults.partialLoadFrom(m_config);
		setConfigValuesToUI(defaults);
	}

	void LogWidget::onClearAllDataButton ()
	{
		m_proxy_model->clearModelData();
		m_src_model->clearModelData();
	}

	void LogWidget::performSynchronization (E_SyncMode mode, int sync_group, unsigned long long time, void * source)
	{
		qDebug("%s syncgrp=%i time=%i", __FUNCTION__, sync_group, time);

		if (this == source)
			return;

		switch (mode)
		{
			case e_SyncClientTime:
				findNearestRow4Time(true, time);
				break;
			case e_SyncServerTime:
				findNearestRow4Time(false, time);
				break;
			case e_SyncFrame:
			case e_SyncSourceRow:
			case e_SyncProxyRow:
			default:
				break;
		}
	}

	/*void BaseLog::performFrameSynchronization (int sync_group, unsigned long long frame, void * source)
	{
		qDebug("%s syncgrp=%i frame=%i", __FUNCTION__, sync_group, frame);
	}*/

	void LogWidget::onFilterEnabledChanged ()
	{
		qDebug("%s", __FUNCTION__);
		resizeModelToConfig(m_config);
		//setupUi
		//applyConfig();
	}

void LogWidget::onDumpFilters ()
{
	/*
	QDialog dialog(this);
	dialog.setWindowFlags(Qt::Sheet);
	m_help->setupUi(&dialog);
	m_help->helpTextEdit->clear();

	QString text(tr("Dumping current filters:\n"));
	QString session_string;

	if (Connection * conn = m_server->findCurrentConnection())
	{
		SessionState const & ss = conn->sessionState();
		QString ff;
		ss.m_file_filters.dump_filter(ff);

		QString cols;
		for (int i = 0, ie = ss.m_colorized_texts.size(); i < ie; ++i)
		{
			ColorizedText const & x = ss.m_colorized_texts.at(i);
			cols += QString("%1 %2 %3\n")
						.arg(x.m_regex_str)
						.arg(x.m_qcolor.name())
						.arg(x.m_is_enabled ? " on" : "off");
		}
		QString regs;
		for (int i = 0, ie = ss.m_filtered_regexps.size(); i < ie; ++i)
		{
			FilteredRegex const & x = ss.m_filtered_regexps.at(i);
			regs += QString("%1 %2 %3\n")
						.arg(x.m_regex_str)
						.arg(x.m_state ? "incl" : "excl")
						.arg(x.m_is_enabled ? " on" : "off");
		}
		QString lvls;
		for (int i = 0, ie = ss.m_lvl_filters.size(); i < ie; ++i)
		{
			FilteredLevel const & x = ss.m_lvl_filters.at(i);
			lvls += QString("%1 %2 %3\n")
						.arg(x.m_level_str)
						.arg(x.m_is_enabled ? " on" : "off")
						.arg(x.m_state ? "incl" : "excl");
		}
		QString ctxs;
		for (int i = 0, ie = ss.m_ctx_filters.size(); i < ie; ++i)
		{
			FilteredContext const & x = ss.m_ctx_filters.at(i);
			ctxs += QString("%1 %2 %3\n")
						.arg(x.m_ctx_str)
						.arg(x.m_state)
						.arg(x.m_is_enabled ? " on" : "off");
		}

		session_string = QString("Files:\n%1\n\nColors:\n%2\n\nRegExps:\n%3\n\nLevels:\n%4\n\nContexts:\n%5\n\n")
				.arg(ff)
				.arg(cols)
				.arg(regs)
				.arg(lvls)
				.arg(ctxs);
	}

	m_help->helpTextEdit->setPlainText(text + session_string);
	m_help->helpTextEdit->setReadOnly(true);
	dialog.exec();
	*/
}

DecodedCommand const * LogWidget::getDecodedCommand (QModelIndex const & row_index)
{
	return getDecodedCommand(row_index.row());
}

DecodedCommand const * LogWidget::getDecodedCommand (int row)
{
	if (row >= 0 && row < m_src_model->dcmds().size())
		return &m_src_model->dcmds()[row];
	return 0;
}

void LogWidget::commitCommands (E_ReceiveMode mode)
{
	for (int i = 0, ie = m_queue.size(); i < ie; ++i)
	{
		DecodedCommand & cmd = m_queue[i];
		m_src_model->handleCommand(cmd, mode);
		// warning: qmodelindex in source does not exist yet here... 
		appendToFilters(cmd); // this does not bother filters
		//appendToColorizers(cmd); // but colorizer needs qmodelindex
	}
	if (m_queue.size())
	{
		m_src_model->commitCommands(mode);
		if (m_config.m_auto_scroll)
			m_tableview->scrollToBottom();
		m_queue.clear();
	}
}

void LogWidget::handleCommand (DecodedCommand const & cmd, E_ReceiveMode mode)
{
	if (mode == e_RecvSync)
		m_src_model->handleCommand(cmd, mode);
	else
		m_queue.append(cmd);

/*	if (cmd.hdr.cmd == tlv::cmd_scope_entry || (cmd.hdr.cmd == tlv::cmd_scope_exit))
	{
		if (m_config.m_scopes)
		{
			LogTableModel * m = static_cast<LogTableModel *>(m_proxy_model ? m_proxy_model->sourceModel() : model());
			//dp.widget().model()->appendCommand(m_proxy_model, cmd);
			m->appendCommand(m_proxy_model, cmd);
		}
	}
	else if (cmd.hdr.cmd == tlv::cmd_log)
	{
		m_src_model->handleCommand(cmd);
		//m_src_model->appendCommand(m_proxy_model, cmd);
	}*/
}

void LogWidget::reloadModelAccordingTo (LogConfig & config)
{
	m_tableview->horizontalHeader()->reset();
	//setItemDelegate(new LogDelegate(*this, m_connection->appData(), this));
	//setItemDelegate(0); // @FIXME TMP
	m_config.m_columns_setup = config.m_columns_setup;
	m_config.m_columns_sizes = config.m_columns_sizes;
	m_config.m_columns_align = config.m_columns_align;
	m_config.m_columns_elide = config.m_columns_elide;
	m_src_model->clearModel();
	m_proxy_model->clearModel();
	m_tags2columns.clear();
	m_src_model->reloadModelAccordingTo(config);
	resizeSections();
}


void LogWidget::commitBatchToLinkedWidgets (int src_from, int src_to, BatchCmd const & batch)
{
  for (linked_widgets_t::iterator it = m_linked_widgets.begin(), ite = m_linked_widgets.end(); it != ite; ++it)
  {
    DockedWidgetBase * child = *it;
	if (child->type() == e_data_log)
	{
		LogWidget * lw = qobject_cast<LogWidget *>(child->dockedWidget());
		lw->commitBatchToLinkedModel(src_from, src_to, batch);  
	}
  }
}

void LogWidget::commitBatchToLinkedModel (int src_from, int src_to, BatchCmd const & batch)
{
	FilterProxyModel * flt_pxy = m_proxy_model;
	if (m_tableview->model() == flt_pxy)
		flt_pxy->commitBatchToModel(src_from, src_to, batch);

	FindProxyModel * fnd_pxy = m_find_proxy_model;
	if (m_tableview->model() == fnd_pxy)
		fnd_pxy->commitBatchToModel(src_from, src_to, batch);
}

LogTableModel * LogWidget::cloneToNewModel (FindConfig const & fc)
{
	if (m_tableview->model() == m_src_model)
	{
		return m_src_model->cloneToNewModel(fc);
	}
	else if (m_tableview->model() == m_proxy_model)
	{
		Q_ASSERT(m_src_model);
		LogTableModel * new_model = new LogTableModel(this, *this);
		for (size_t r = 0, re = m_src_model->dcmds().size(); r < re; ++r)
		{
			DecodedCommand const & dcmd = m_src_model->dcmds()[r];
			if (m_proxy_model->filterAcceptsRow(r, QModelIndex()))
			{
				bool row_match = false;
				for (size_t i = 0, ie = dcmd.m_tvs.size(); i < ie; ++i)
				{
					QString const & val = dcmd.m_tvs[i].m_val;

					if (matchToFindConfig(val, fc))
					{
						row_match = true;
						break;
					}
				}

				if (row_match)
				{
					new_model->handleCommand(dcmd, e_RecvBatched);
				}
			}

		}
		new_model->commitCommands(e_RecvSync);
		return new_model;
	}
}



/*void LogWidget::applyConfig ()
{
	settings.setValue("autoScrollCheckBox", ui->autoScrollCheckBox->isChecked());
	settings.setValue("inViewCheckBox", ui->inViewCheckBox->isChecked());
	settings.setValue("filterFileCheckBox", ui->filterFileCheckBox->isChecked());
	settings.setValue("clrFiltersCheckBox", ui_settings->clrFiltersCheckBox->isChecked());
	//settings.setValue("filterModeComboBox", ui->filterModeComboBox->currentIndex());
	//settings.setValue("filterPaneComboBox", ui_settings->filterPaneComboBox->currentIndex());

	settings.setValue("scopesCheckBox1", ui_settings->scopesCheckBox->isChecked());
	settings.setValue("indentCheckBox", ui_settings->indentCheckBox->isChecked());
	settings.setValue("cutPathCheckBox", ui_settings->cutPathCheckBox->isChecked());
	settings.setValue("cutNamespaceCheckBox", ui_settings->cutNamespaceCheckBox->isChecked());
	settings.setValue("indentSpinBox", ui_settings->indentSpinBox->value());
	settings.setValue("tableRowSizeSpinBox", ui_settings->tableRowSizeSpinBox->value());
	settings.setValue("tableFontComboBox", ui_settings->tableFontComboBox->currentText());
	settings.setValue("cutPathSpinBox", ui_settings->cutPathSpinBox->value());
	settings.setValue("cutNamespaceSpinBox", ui_settings->cutNamespaceSpinBox->value());
}

*/

int LogWidget::findColumn4TagCst (tlv::tag_t tag) const
{
	QMap<tlv::tag_t, int>::const_iterator it = m_tags2columns.find(tag);
	if (it != m_tags2columns.end())
		return it.value();
	return -1;
}

int LogWidget::findColumn4Tag (tlv::tag_t tag)
{
	QMap<tlv::tag_t, int>::const_iterator it = m_tags2columns.find(tag);
	if (it != m_tags2columns.end())
		return it.value();

	char const * name = tlv::get_tag_name(tag);
	QString const qname = QString(name);
	for (int i = 0, ie = m_config.m_columns_setup.size(); i < ie; ++i)
		if (m_config.m_columns_setup[i] == qname)
		{
			m_tags2columns.insert(tag, i);
			return i;
		}
	return -1;
}

int LogWidget::appendColumn (tlv::tag_t tag)
{
	TagDesc const & desc = m_tagconfig.findOrCreateTag(tag);

	m_config.m_columns_setup.push_back(desc.m_tag_str);
	m_config.m_columns_align.push_back(desc.m_align_str);
	m_config.m_columns_elide.push_back(desc.m_elide_str);
	m_config.m_columns_sizes.push_back(desc.m_size);

	//qDebug("inserting column and size. tmpl_sz=%u curr_sz=%u sizes_sz=%u", m_columns_setup_template->size(), m_columns_setup_current->size(), m_columns_sizes->size());

	int const column_index = m_config.m_columns_setup.size() - 1;
	m_tags2columns.insert(tag, column_index);

	return column_index;
}

inline void simplify_keep_indent (QString const & src, QString & indent, QString & dst)
{
	const int n = src.size();
	for (int i = 0; i < n; ++i)
	{
		if (!src.at(i).isSpace())
		{
			dst = src.right(n - i);
			indent = src.left(i);
			return;
		}
	}
}

QString LogWidget::exportSelection ()
{
	QAbstractItemModel * m = m_tableview->model();
	QItemSelectionModel * selection = m_tableview->selectionModel();
	if (!selection)
		return QString();
	QModelIndexList indexes = selection->selectedIndexes();

	if (indexes.size() < 1)
		return QString();

	std::sort(indexes.begin(), indexes.end());

	QString selected_text;
	selected_text.reserve(4096);
	for (int i = 0; i < indexes.size(); ++i)
	{
		QModelIndex const current = indexes.at(i);
		QString const str = m->data(current).toString();
		QString skipped_indent, to_simplify;
		simplify_keep_indent(str, skipped_indent, to_simplify);
		QString const simplified = to_simplify.simplified();
		selected_text.append(skipped_indent);
		selected_text.append(simplified);
		
		if (i + 1 < indexes.size() && current.row() != indexes.at(i + 1).row())
			selected_text.append('\n'); // switching rows
		else
			selected_text.append('\t');
	}
	return selected_text;
}

void LogWidget::onCopyToClipboard ()
{
	QString const text = exportSelection();
	QClipboard * clipboard = QApplication::clipboard();
	clipboard->setText(text);
}

void LogWidget::autoScrollOff ()
{
	m_config.m_auto_scroll = false;
}

void LogWidget::autoScrollOn ()
{
	m_config.m_auto_scroll = true;
}

//@TODO: should be in model probably
void LogWidget::findNearestRow4Time (bool ctime, unsigned long long t)
{
	//qDebug("%s this=0x%08x", __FUNCTION__, this);
	bool const is_proxy = isModelProxy();
	int closest_i = 0;
	unsigned long long closest_dist = std::numeric_limits<unsigned long long>::max();
	for (int i = 0; i < m_src_model->rowCount(); ++i)
	{
		unsigned long long t0 = ctime ? m_src_model->row_ctime(i) : m_src_model->row_stime(i);
		long long const diff = t0 - t;
		long long const d = abs(diff);
		bool const row_exists = is_proxy ? m_proxy_model->rowInProxy(i) : true;
		if (row_exists && d < closest_dist)
		{
			closest_i = i;
			closest_dist = d;
			//qDebug("table: nearest index= %i/%i %llu", closest_i, m_src_model->rowCount(), d);
		}
	}

	if (is_proxy)
	{
		qDebug("table: pxy nearest index= %i/%i", closest_i, m_src_model->rowCount());
		QModelIndex const curr = m_tableview->currentIndex();
		QModelIndex const idx = m_src_model->index(closest_i, curr.column() < 0 ? 0 : curr.column(), QModelIndex());
		//qDebug("table: pxy findNearestTime curr=(%i, %i)  new=(%i, %i)", curr.column(), curr.row(), idx.column(), idx.row());

		QModelIndex const pxy_idx = m_proxy_model->mapFromSource(idx);
		QModelIndex valid_pxy_idx = pxy_idx;
		if (!pxy_idx.isValid())
		{
			valid_pxy_idx = m_proxy_model->mapNearestFromSource(idx);
		}
		//qDebug("table: pxy findNearestTime pxy_new=(%i, %i) valid_pxy_new=(%i, %i)", pxy_idx.column(), pxy_idx.row(), valid_pxy_idx.column(), valid_pxy_idx.row());
		m_tableview->scrollTo(valid_pxy_idx, QAbstractItemView::PositionAtCenter);
		m_tableview->selectionModel()->select(valid_pxy_idx, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
	}
	else
	{
		qDebug("table: nearest index= %i/%i", closest_i, m_src_model->rowCount());
		QModelIndex const curr = m_tableview->currentIndex();
		QModelIndex const idx = m_src_model->index(closest_i, curr.column() < 0 ? 0 : curr.column(), QModelIndex());
		//qDebug("table: findNearestTime curr=(%i, %i)  new=(%i, %i)", curr.column(), curr.row(), idx.column(), idx.row());

		m_tableview->scrollTo(idx, QAbstractItemView::PositionAtCenter);
		m_tableview->selectionModel()->select(idx, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
	}
}

bool LogWidget::isModelProxy () const
{
	if (0 == m_tableview->model())
		return false;
	return m_tableview->model() == m_proxy_model;
}

void LogWidget::onFilterChanged ()
{
	onInvalidateFilter();
}

void LogWidget::onSectionMoved (int logical, int old_visual, int new_visual)
{
	qDebug("log: section moved logical=%i old_visual=%i new_visual=%i", logical, old_visual, new_visual);
}

void LogWidget::onSectionResized (int logical, int old_size, int new_size)
{
	qDebug("log: section resized logical=%i old_sz=%i new_sz=%i", logical, old_size, new_size);
	int const idx = !isModelProxy() ? logical : m_proxy_model->colToSource(logical);
	//qDebug("table: on rsz hdr[%i -> src=%02i ]	%i->%i\t\t%s", c, idx, old_size, new_size, m_config.m_hhdr.at(idx).toStdString().c_str());
	if (idx < 0) return;
	int const curr_sz = m_config.m_columns_sizes.size();
	if (idx < curr_sz)
	{
		//qDebug("%s this=0x%08x hsize[%i]=%i", __FUNCTION__, this, idx, new_size);
	}
	else
	{
		m_config.m_columns_sizes.resize(idx + 1);
		for (int i = curr_sz; i < idx + 1; ++i)
			m_config.m_columns_sizes[i] = 32;
	}
	m_config.m_columns_sizes[idx] = new_size;
}

void LogWidget::exportStorageToCSV (QString const & filename)
{
	// " --> ""
	QRegExp regex("\"");
	QString to_string("\"\"");
	QFile csv(filename);
	csv.open(QIODevice::WriteOnly);
	QTextStream str(&csv);

	for (int c = 0, ce = m_config.m_columns_setup.size(); c < ce; ++c)
	{
		str << "\"" << m_config.m_columns_setup.at(c) << "\"";
		if (c < ce - 1)
			str << ",\t";
	}
	str << "\n";

	for (int r = 0, re = m_tableview->model()->rowCount(); r < re; ++r)
	{
		for (int c = 0, ce = m_tableview->model()->columnCount(); c < ce; ++c)
		{
			QModelIndex current = m_tableview->model()->index(r, c, QModelIndex());
			// csv nedumpovat pres proxy
			QString txt = m_tableview->model()->data(current).toString();
			QString simplified = txt.simplified();
			QString const quoted_txt = simplified.replace(regex, to_string);
			str << "\"" << quoted_txt << "\"";
			if (c < ce - 1)
				str << ",\t";
		}
		str << "\n";
	}
	csv.close();
}


}


