#include "logwidget.h"
#include "filterproxymodel.h"
#include "connection.h"
#include "mainwindow.h"
#include "utils_openfilelinewith.h"

namespace logs {

bool LogWidget::handleAction (Action * a, E_ActionHandleType sync)
{
	switch (a->type())
	{
		case e_Close:
		{
			m_connection->destroyDockedWidget(this);
			setParent(0);
			delete this;
			return true;
		}

		case e_Visibility:
		{
			Q_ASSERT(a->m_args.size() > 0);
			bool const on = a->m_args.at(0).toBool();
			setVisible(on);
			m_connection->getMainWindow()->onDockRestoreButton();
			return true;
		}

		case e_Find:
		{
			if (a->m_args.size() > 0)
			{
				if (a->m_args.at(0).canConvert<FindConfig>())
				{
					FindConfig const fc = a->m_args.at(0).value<FindConfig>();
					handleFindAction(fc);
					m_config.m_find_config = fc;
					// m_config.save
				}
				return true;
			}
		}
		case e_Colorize:
		{
			if (a->m_args.size() > 0)
			{
				if (a->m_args.at(0).canConvert<ColorizeConfig>())
				{
					ColorizeConfig const cc = a->m_args.at(0).value<ColorizeConfig>();
					handleColorizeAction(cc);
					m_config.m_colorize_config = cc;
					// m_config.save
				}
				return true;
			}
		}

		default:
			return false;
	}
	return false;
}

void LogWidget::onTableClicked (QModelIndex const & row_index)
{
	enableAutoScroll(false);

	QModelIndex const curr_src_idx = currentSourceIndex();
	if (!curr_src_idx.isValid())
		return;

	bool const scroll_to_item = false;
	bool const expand = false;
	findTableIndexInFilters(curr_src_idx, scroll_to_item, expand);

	m_lastSelectedColumn = row_index.column();
}

void LogWidget::onTableFontToolButton ()
{
	/*
	bool ok = false;
	QFont curr_font;
	Connection * conn = m_server->findCurrentConnection();
	if (conn)
	{
		curr_font = conn->getTableViewWidget()->font();
		ui_settings->tableFontComboBox->addItem(curr_font.toString());
	}

	QFont f = QFontDialog::getFont(&ok, curr_font);
	if (ok)
	{
		ui_settings->tableFontComboBox->insertItem(0, curr_font.toString());
		verticalHeader()->setFont(f);
	}
	*/
}


void LogWidget::onNextToView ()
{
	nextToView();
}

void LogWidget::findTableIndexInFilters (QModelIndex const & src_idx, bool scroll_to_item, bool expand)
{
	DecodedCommand const * dcmd = getDecodedCommand(src_idx);
	if (dcmd)
	{
		QString file, line;
		if (dcmd->getString(tlv::tag_file, file) && dcmd->getString(tlv::tag_line, line))
		{
			QString const combined = file + "/" + line;
			if (filterMgr()->getFilterFileLine())
				filterMgr()->getFilterFileLine()->locateItem(combined, scroll_to_item, expand);
		}
		QString tid;
		if (dcmd->getString(tlv::tag_tid, tid))
		{
			if (filterMgr()->getFilterTid())
				filterMgr()->getFilterTid()->locateItem(tid, scroll_to_item, expand);;
		}
		QString lvl;
		if (dcmd->getString(tlv::tag_lvl, lvl))
		{
			if (filterMgr()->getFilterLvl())
				filterMgr()->getFilterLvl()->locateItem(lvl, scroll_to_item, expand);;
		}
		QString ctx;
		if (dcmd->getString(tlv::tag_ctx, ctx))
		{
			if (filterMgr()->getFilterCtx())
				filterMgr()->getFilterCtx()->locateItem(ctx, scroll_to_item, expand);;
		}
	}
}

void LogWidget::colorRow (int)
{
	QModelIndex curr_src_idx = currentSourceIndex();

	int const row = curr_src_idx.row(); // set search from this line
	if (curr_src_idx.isValid())
	{
		qDebug("Color tag on row=%i", curr_src_idx.row());
		addColorTagRow(curr_src_idx.row());
		onInvalidateFilter();
	}
}

/*void LogWidget::onClearCurrentView ()
{
	LogTableModel * model = static_cast<LogTableModel *>(m_proxy_model ? m_proxy_model->sourceModel() : m_src_model);
	//excludeContentToRow(model->rowCount());
	//onInvalidateFilter();
}*/

void LogWidget::excludeFile (QModelIndex const & src_index)
{
	if (filterMgr()->getFilterFileLine())
	{
		if (!src_index.isValid())
			return;

		DecodedCommand const * dcmd = getDecodedCommand(src_index);
		if (dcmd)
		{

			QString file;
			if (dcmd->getString(tlv::tag_file, file))
			{
				qDebug("excluding: %s", file.toStdString().c_str());
				QString const fileline = file;
				QModelIndex const result = filterMgr()->getFilterFileLine()->fileModel()->stateToItem(fileline, Qt::Unchecked);
				if (!result.isValid())
				{
					qWarning("%s: nonexistent index!", __FUNCTION__);
				}
			}
			onInvalidateFilter();
		}
	}
}

void LogWidget::onExcludeFile ()
{
	QModelIndexList l;
	QModelIndexList src_list;
	currSelection(l);
	foreach(QModelIndex index, l)
	{
		if (isModelProxy())
		{
			index = m_proxy_model->mapToSource(index);
		}

		if (index.isValid())
			src_list << index;
	}

	foreach(QModelIndex index, src_list)
	{
		if (index.isValid())
			excludeFile(index);
	}
}

void LogWidget::excludeFileLine (QModelIndex const & src_index)
{
	if (filterMgr()->getFilterFileLine())
	{
		if (!src_index.isValid())
			return;

		DecodedCommand const * dcmd = getDecodedCommand(src_index);
		if (dcmd)
		{

			QString file, line;
			if (dcmd->getString(tlv::tag_file, file) && dcmd->getString(tlv::tag_line, line))
			{
				qDebug("excluding: %s:%s", file.toStdString().c_str(), line.toStdString().c_str());
				QString const fileline = file + "/" + line;
				QModelIndex const result = filterMgr()->getFilterFileLine()->fileModel()->stateToItem(fileline, Qt::Unchecked);
				if (!result.isValid())
				{
					qWarning("%s: nonexistent index!", __FUNCTION__);
				}
			}
			onInvalidateFilter();
		}
	}
}

void LogWidget::onExcludeFileLine ()
{
	QModelIndexList l;
	QModelIndexList src_list;
	currSelection(l);
	foreach(QModelIndex index, l)
	{
		if (isModelProxy())
		{
			index = m_proxy_model->mapToSource(index);
		}

		if (index.isValid())
			src_list << index;
	}

	foreach(QModelIndex index, src_list)
	{
		if (index.isValid())
			excludeFileLine(index);
	}
}

void LogWidget::excludeRow (QModelIndex const & src_index)
{
	if (!filterMgr()->getFilterRow())
	{
		// request to create filter
	}

	if (filterMgr()->getFilterRow())
	{
		if (!src_index.isValid())
			return;

		DecodedCommand const * dcmd = getDecodedCommand(src_index);
		if (dcmd)
		{
			qDebug("excluding: %i", dcmd->m_src_row);
			filterMgr()->getFilterRow()->appendRowFilter(dcmd->m_src_row);
			onInvalidateFilter(); //@FIXME: performance if selection!
		}
	}
}

void LogWidget::onExcludeRow ()
{
	QModelIndexList l;
	QModelIndexList src_list;
	currSelection(l);
	foreach(QModelIndex index, l)
	{
		if (isModelProxy())
		{
			index = m_proxy_model->mapToSource(index);
		}

		if (index.isValid())
			src_list << index;
	}

	foreach(QModelIndex index, src_list)
	{
		if (index.isValid())
			excludeRow(index);
	}
}
void LogWidget::onLocateRow ()
{
	QModelIndex curr_src_idx = currentSourceIndex();

	if (!curr_src_idx.isValid())
		return;

	bool const scroll_to_item = true;
	bool const expand = true;
	findTableIndexInFilters(curr_src_idx, scroll_to_item, expand);
	filterMgr()->focusToFilter(e_Filter_FileLine);
	m_config_ui.ui()->groupingWidget->setCurrentWidget(m_config_ui.ui()->filtersPage);
	// @TODO: locate in colorizer row
}
void LogWidget::onColorFileLine ()
{
}
void LogWidget::onColorRow ()
{
	colorRow(0);
}
void LogWidget::onUncolorRow ()
{
	QModelIndex curr_src_idx = currentSourceIndex();

	int const row = curr_src_idx.row(); // set search from this line
	if (curr_src_idx.isValid())
	{
		qDebug("uncolor tag on row=%i", curr_src_idx.row());
		removeColorTagRow(curr_src_idx.row());
		onInvalidateFilter();
	}
}

void LogWidget::onChangeTimeUnits (int)
{
	QString const & curr = m_timeComboBox->comboBox()->currentText();
	float const t = stringToUnitsValue(curr);
	m_config.m_time_units_str = curr;
	m_config.m_time_units = t;
	onInvalidateFilter(); // TODO: invalidate only column?
}

void LogWidget::onSetRefTime ()
{
	if (m_setRefTimeButton->isChecked())
	{
		QModelIndex const curr_src_idx = currentSourceIndex();
		if (curr_src_idx.isValid())
		{
			QString const & strtime = findString4Tag(tlv::tag_ctime, curr_src_idx);
			setTimeRefValue(strtime.toULongLong());
			onInvalidateFilter();
		}
	}
	else
	{
		clearRefTime();
		onInvalidateFilter(); // TODO: invalidate only column?
	}
}

void LogWidget::onGotoPrevErr ()
{
	FindConfig fc;
	fc.m_next = 0;
	fc.m_prev = 1;
	fc.m_select = 1;
	fc.m_refs = 0;
	fc.m_clone = 0;
	fc.m_case_sensitive = 0;
	fc.m_whole_word = 0;
	fc.m_regexp = 0;
	fc.m_str = "Error";
	findAndSelectPrevFromCurrent(fc);
}
void LogWidget::onGotoNextErr ()
{
	FindConfig fc;
	fc.m_next = 1;
	fc.m_prev = 0;
	fc.m_select = 1;
	fc.m_refs = 0;
	fc.m_clone = 0;
	fc.m_case_sensitive = 0;
	fc.m_whole_word = 0;
	fc.m_regexp = 0;
	fc.m_str = "Error";
	findAndSelectNextFromCurrent(fc);
}
void LogWidget::onGotoPrevWarn ()
{
	FindConfig fc;
	fc.m_next = 0;
	fc.m_prev = 1;
	fc.m_select = 1;
	fc.m_refs = 0;
	fc.m_clone = 0;
	fc.m_case_sensitive = 0;
	fc.m_whole_word = 0;
	fc.m_regexp = 0;
	fc.m_str = "Warning";
	findAndSelectPrevFromCurrent(fc);
}
void LogWidget::onGotoNextWarn ()
{
	FindConfig fc;
	fc.m_next = 1;
	fc.m_prev = 0;
	fc.m_select = 1;
	fc.m_refs = 0;
	fc.m_clone = 0;
	fc.m_case_sensitive = 0;
	fc.m_whole_word = 0;
	fc.m_regexp = 0;
	fc.m_str = "Warning";
	findAndSelectNextFromCurrent(fc);
}

void LogWidget::onHidePrev ()
{
	bool const checked = m_hidePrevButton->isChecked();

	QModelIndex curr_src_idx = currentSourceIndex();

	if (!curr_src_idx.isValid())
		return;

	QString const & strtime = findString4Tag(tlv::tag_ctime, curr_src_idx);

	filterMgr()->mkFilter(e_Filter_Time);

	if (checked)
		filterMgr()->getFilterTime()->onAdd(cmpModToString(e_CmpGE), strtime, m_config.m_time_units_str);
	else
		filterMgr()->getFilterTime()->remove(cmpModToString(e_CmpGE), strtime, m_config.m_time_units_str);

	onInvalidateFilter(); //@TODO: should be done by filter?
}
void LogWidget::onHideNext () //@TODO: dedup
{
	bool const checked = m_hidePrevButton->isChecked();

	QModelIndex curr_src_idx = currentSourceIndex();

	if (!curr_src_idx.isValid())
		return;

	QString const & strtime = findString4Tag(tlv::tag_ctime, curr_src_idx);

	filterMgr()->mkFilter(e_Filter_Time);

	if (checked)
		filterMgr()->getFilterTime()->onAdd(cmpModToString(e_CmpLE), strtime, m_config.m_time_units_str);
	else
		filterMgr()->getFilterTime()->remove(cmpModToString(e_CmpLE), strtime, m_config.m_time_units_str);

	onInvalidateFilter(); //@TODO: should be done by filter?
}


void LogWidget::onTableDoubleClicked (QModelIndex const & row_index)
{
	qDebug("%s this=0x%08x", __FUNCTION__, this);
	if (m_config.m_sync_group == 0)
		return;	// do not sync groups with zero

	E_SyncMode const mode = e_SyncServerTime;
	//@TODO: mode from UI

	QModelIndex curr = row_index;
	if (isModelProxy())
	{
		curr = m_proxy_model->mapToSource(row_index);
	}

	unsigned long long time = 0;
	if (mode == e_SyncServerTime)
		time = m_src_model->row_stime(curr.row());
	else
		time = m_src_model->row_ctime(curr.row());

	qDebug("table: dblClick curr=(%i, %i)  time=%llu", curr.row(), curr.column(), time);
	emit requestSynchronization(mode, m_config.m_sync_group, time, this);

/*	if (m_proxy_model)
	{
		QModelIndex const curr = m_proxy_model->mapToSource(row_index);
		LogTableModel * model = static_cast<LogTableModel *>(m_proxy_model ? m_proxy_model->sourceModel() : model());
		qDebug("2c curr: (%i,col) -> (%i,col)", row_index.row(), curr.row());

		int row_bgn = curr.row();
		int row_end = curr.row();
		int layer = model->layers()[row_bgn];

		if (model->rowTypes()[row_bgn] == tlv::cmd_scope_exit)
		{
			layer += 1;
			// test na range
			--row_bgn;
		}

		QString tid = findString4Tag(tlv::tag_tid, curr);
		QString file = findString4Tag(tlv::tag_file, curr);
		QString line = findString4Tag(tlv::tag_line, curr);
		int from = row_bgn;

		if (model->rowTypes()[from] != tlv::cmd_scope_entry)
		{
			while (row_bgn > 0)
			{
				QModelIndex const curr_idx = model->index(row_bgn, row_index.column(), QModelIndex());
				QString curr_tid = findString4Tag(tlv::tag_tid, curr_idx);
				if (curr_tid == tid)
				{
					if (static_cast<int>(model->layers()[row_bgn]) >= layer)
					{
						from = row_bgn;
					}
					else
					{	
						break;
					}
				}
				--row_bgn;
			}
		}

		int to = row_end;
		if (model->rowTypes()[to] != tlv::cmd_scope_exit)
		{
			while (row_end < static_cast<int>(model->layers().size()))
			{
				QModelIndex const curr_idx = model->index(row_end, row_index.column(), QModelIndex());
				QString next_tid = findString4Tag(tlv::tag_tid, curr_idx);
				if (next_tid == tid)
				{
					if (model->layers()[row_end] >= layer)
						to = row_end;
					else if ((model->layers()[row_end] == layer - 1) && (model->rowTypes()[row_end] == tlv::cmd_scope_exit))
					{
						to = row_end;
						break;
					}
					else
						break;
				}
				++row_end;
			}
		}

		qDebug("row=%u / curr=%u layer=%u, from,to=(%u, %u)", row_index.row(), curr.row(), layer, from, to);
		if (m_session_state.findCollapsedBlock(tid, from, to))
			m_session_state.eraseCollapsedBlock(tid, from, to);
		else
			m_session_state.appendCollapsedBlock(tid, from, to, file, line);
		onInvalidateFilter();
	}*/
}

void LogWidget::onOpenFileLine ()
{
	QModelIndex const curr_src_idx = currentSourceIndex();
	if (!curr_src_idx.isValid())
		return;

	DecodedCommand const * dcmd = getDecodedCommand(curr_src_idx);
	if (dcmd)
	{
		QString file, line;
		if (dcmd->getString(tlv::tag_file, file) && dcmd->getString(tlv::tag_line, line))
		{
			openFileLineWith(m_config_ui.ui()->openFileWithComboBox->currentText(), file, line);
		}
	}
}

}

