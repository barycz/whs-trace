#include "logtableview.h"
#include "logwidget.h"
#include <connection.h>

namespace logs {

	LogTableView::LogTableView (Connection * conn, LogWidget & logwidget, LogConfig & config)
		: TableView(0)
		, m_connection(conn)
		, m_log_widget(logwidget)
		, m_config(config)
	{
		//setAutoScroll(false); @NOTE: this does not work the way i want
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
		/*
		//m_table_view_widget->horizontalHeader()->setStretchLastSection(false);
		//////////////// PERF!!!!! //////////////////
		// m_table_view_widget->horizontalHeader()->setStretchLastSection(true);
		//////////////// PERF!!!!! //////////////////
		*/
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
		if (hint == QAbstractItemView::EnsureVisible)
			QTableView::scrollTo(index, hint); // @TODO: modify qt's default behaviour
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
			QModelIndex mod_idx = m_log_widget.currentSourceIndex();

			unsigned long long const t = m_log_widget.m_src_model->row_stime(mod_idx.row());

			m_log_widget.emitRequestSynchronization(e_SyncServerTime, m_config.m_sync_group, t, this); // this is ignored in the call
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

}


