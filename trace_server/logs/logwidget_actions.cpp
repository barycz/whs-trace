#include "logwidget.h"
#include "filterproxymodel.h"

namespace logs {

bool LogWidget::handleAction (Action * a, E_ActionHandleType sync)
{
	switch (a->type())
	{
		case e_Visibility:
		{
			Q_ASSERT(m_args.size() > 0);
			bool const on = a->m_args.at(0).toBool();
			setVisible(on);
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
		default:
			return false;
	}
	return false;
}

void LogWidget::onTableClicked (QModelIndex const & row_index)
{
/*	m_last_clicked = row_index;
	if (isModelProxy())
		m_last_clicked = m_proxy_model->mapToSource(row_index);

	m_last_search_row = m_last_clicked.row(); // set search from this line
	m_last_search_col = m_last_clicked.column();*/

	bool const scroll_to_item = false;
	bool const expand = false;
	findTableIndexInFilters(m_last_clicked, scroll_to_item, expand);
}

void LogWidget::onTableFontToolButton ()
{
    /*bool ok = false;
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
    }*/
}


void LogWidget::onNextToView ()
{
	nextToView();
}

void LogWidget::turnOffAutoScroll ()
{
	//ui->autoScrollCheckBox->setCheckState(Qt::Unchecked);
}

void LogWidget::onAutoScrollHotkey ()
{
	/*if (ui->autoScrollCheckBox->checkState() == Qt::Checked)
		turnOffAutoScroll();
	else
		ui->autoScrollCheckBox->setCheckState(Qt::Checked);*/
}

void LogWidget::findTableIndexInFilters (QModelIndex const & src_idx, bool scroll_to_item, bool expand)
{
	{
		QString const file = findString4Tag(tlv::tag_file, src_idx);
		QString const line = findString4Tag(tlv::tag_line, src_idx);
		QString const combined = file + "/" + line;
		qDebug("findTableIndexInFilters %s in tree", combined.toStdString().c_str());
		filterMgr()->getFilterFileLine()->locateItem(combined, scroll_to_item, expand);
	}
	{
		QString const & item = findString4Tag(tlv::tag_tid, src_idx);
		filterMgr()->getFilterTid()->locateItem(item, scroll_to_item, expand);;
	}
	{
		QString const & item = findString4Tag(tlv::tag_lvl, src_idx);
		filterMgr()->getFilterLvl()->locateItem(item, scroll_to_item, expand);;
	}
	{
		QString const & item = findString4Tag(tlv::tag_ctx, src_idx);
		filterMgr()->getFilterCtx()->locateItem(item, scroll_to_item, expand);;
	}
}

void LogWidget::onExcludeFileLine ()
{
	QModelIndex const current = m_last_clicked;
	if (current.isValid())
	{
		onExcludeFileLine(current);
		onInvalidateFilter();
	}
}

void LogWidget::onToggleRefFromRow ()
{
	QModelIndex const current = m_last_clicked;
	if (current.isValid())
	{
		qDebug("Toggle Ref from row=%i", current.row());
		setTimeRefFromRow(current.row());

		//LogTableModel * model = static_cast<LogTableModel *>(m_proxy_model ? m_proxy_model->sourceModel() : model());
		QString const & strtime = findString4Tag(tlv::tag_time, current);
		setTimeRefValue(strtime.toULongLong());
		onInvalidateFilter();
	}
}

void LogWidget::onColorTagRow (int)
{
	QModelIndex current = currentIndex();
	if (isModelProxy())
	{
		current = m_proxy_model->mapToSource(current);
	}

	int const row = current.row(); // set search from this line
	if (current.isValid())
	{
		qDebug("Color tag on row=%i", current.row());
		addColorTagRow(current.row());
		onInvalidateFilter();
	}
}

void LogWidget::onClearCurrentView ()
{
	LogTableModel * model = static_cast<LogTableModel *>(m_proxy_model ? m_proxy_model->sourceModel() : m_src_model);
	excludeContentToRow(model->rowCount());
	onInvalidateFilter();
}

void LogWidget::onHidePrevFromRow ()
{
	QModelIndex const current = m_last_clicked;
	if (current.isValid())
	{
		qDebug("Hide prev from row=%i", current.row());
		excludeContentToRow(current.row());
		onInvalidateFilter();
	}
}

void LogWidget::onUnhidePrevFromRow ()
{
	//m_session_state.excludeContentToRow(0);
	//onInvalidateFilter();
}

void LogWidget::onExcludeFileLine (QModelIndex const & row_index)
{
	if (filterMgr()->getFilterFileLine())
	{
		DecodedCommand const * dcmd = getDecodedCommand(row_index);
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
					Q_ASSERT("nonexistent index");
					qFatal("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
				}
			}
			onInvalidateFilter();
		}
	}
}


void LogWidget::onTableDoubleClicked (QModelIndex const & row_index)
{
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

}

