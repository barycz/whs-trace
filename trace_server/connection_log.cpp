#include "connection.h"
#include <QClipboard>
#include <QObject>
#include "logs/logtablemodel.h"
#include "tableview.h"
#include "constants.h"
#include "utils.h"

DataLog::DataLog (Connection * connection, config_t & config, QString const & fname)
	: DockedData<e_data_log>(connection, config, fname)
{
	qDebug("%s this=0x%08x name=%s", __FUNCTION__, this, fname.toStdString().c_str());

	QWidget * tab = connection->m_tab_widget;
	QHBoxLayout * horizontalLayout = new QHBoxLayout(tab);
	horizontalLayout->setSpacing(1);
	horizontalLayout->setContentsMargins(0, 0, 0, 0);
	horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
	logs::LogWidget * tableView = new logs::LogWidget(connection, tab, config, fname);
	tableView->setStyleSheet("QTableView::item{ selection-background-color:	#F5DEB3  } QTableView::item{ selection-color:	#000000 }");
	
	// to ignore 'resizeColumnToContents' when accidentaly double-clicked on header handle
	QObject::disconnect(tableView->horizontalHeader(), SIGNAL(sectionHandleDoubleClicked(int)), tableView, SLOT(resizeColumnToContents(int)));

	tableView->setObjectName(QString::fromUtf8("tableView"));
	LogTableModel * model = new LogTableModel(tableView, connection);
	QObject::disconnect(model, SIGNAL(rowsInserted(QModelIndex,int,int)), tableView->verticalHeader(), SLOT(sectionsInserted(QModelIndex,int,int)));
    tableView->verticalHeader()->setFont(config.m_font);
	tableView->verticalHeader()->setDefaultSectionSize(config.m_row_width);
	tableView->verticalHeader()->hide();	// @NOTE: users want that //@NOTE2: they can't have it because of performance
	tableView->setModel(model);
	horizontalLayout->addWidget(tableView);
	m_widget = tableView;

	connection->m_table_view_src = model;
	connection->m_table_view_widget = tableView;
	connection->sessionState().setupThreadColors(connection->getMainWindow()->getThreadColors());
	QObject::connect(tableView->horizontalHeader(), SIGNAL(sectionResized(int, int, int)), connection, SLOT(onSectionResized(int, int, int)));



}

bool Connection::handleLogCommand (DecodedCommand const & cmd)
{
	QString const tag("default"); // @FIXME
	//int const slash_pos = tag.lastIndexOf(QChar('/'));
	//tag.chop(msg_tag.size() - slash_pos);

	//QString subtag = msg_tag;
	//subtag.remove(0, slash_pos + 1);

	datalogs_t::iterator it = findOrCreateLog(tag);

	DataLog & dp = **it;

	if (cmd.hdr.cmd == tlv::cmd_scope_entry || (cmd.hdr.cmd == tlv::cmd_scope_exit))
	{
		if (m_main_window->scopesEnabled())
		{
			LogTableModel * model = static_cast<LogTableModel *>(m_table_view_proxy ? m_table_view_proxy->sourceModel() : m_table_view_widget->model());
			//dp.widget().model()->appendCommand(m_table_view_proxy, cmd);
			model->appendCommand(m_table_view_proxy, cmd);
		}
	}
	else if (cmd.hdr.cmd == tlv::cmd_log)
	{
		LogTableModel * model = static_cast<LogTableModel *>(m_table_view_proxy ? m_table_view_proxy->sourceModel() : m_table_view_widget->model());
		model->appendCommand(m_table_view_proxy, cmd);
	}

	m_main_window->getWidgetFile()->hideLinearParents();
	return true;
}


bool Connection::handleLogClearCommand (DecodedCommand const & cmd)
{
	return true;
}


void Connection::onSectionResized (int idx, int /*old_size*/, int new_size)
{
	if (sessionState().getColumnSizes() && idx < sessionState().getColumnSizes()->size())
		sessionState().getColumnSizes()->operator[](idx) = new_size;
}


void Connection::onShowLogs ()
{
	qDebug("%s", __FUNCTION__);
	for (datalogs_t::iterator it = m_data.get<e_data_log>().begin(), ite = m_data.get<e_data_log>().end(); it != ite; ++it)
	{
		(*it)->onShow();
		m_main_window->restoreDockWidget((*it)->m_wd);
	}
}

void Connection::onHideLogs ()
{
	qDebug("%s", __FUNCTION__);
	for (datalogs_t::iterator it = m_data.get<e_data_log>().begin(), ite = m_data.get<e_data_log>().end(); it != ite; ++it)
	{
		(*it)->onHide();
	}
}

void Connection::onShowLogContextMenu (QPoint const &)
{
	qDebug("%s", __FUNCTION__);
	for (datalogs_t::iterator it = m_data.get<e_data_log>().begin(), ite = m_data.get<e_data_log>().end(); it != ite; ++it)
	{
		(*it)->widget().onHideContextMenu();
	}
}

bool Connection::loadConfigForLogs (QString const & preset_name)
{
	qDebug("%s this=0x%08x", __FUNCTION__, this);
	for (datalogs_t::iterator it = m_data.get<e_data_log>().begin(), ite = m_data.get<e_data_log>().end(); it != ite; ++it)
	{
		DataLog * const tbl = *it;
		QString const fname = getDataTagFileName(getConfig().m_appdir, preset_name, g_presetLogTag, tbl->m_config.m_tag);
		loadConfig(tbl->m_config, fname);
		tbl->widget().applyConfig(tbl->m_config);
		if (tbl->m_config.m_show)
			tbl->onShow();
		else
			tbl->onHide();
	}
	return true;
}

bool Connection::saveConfigForLog (logs::LogConfig const & config, QString const & tag)
{
	QString const preset_name = m_curr_preset.isEmpty() ? m_main_window->getValidCurrentPresetName() : m_curr_preset;
	QString const fname = getDataTagFileName(getConfig().m_appdir, preset_name, g_presetLogTag, tag);
	qDebug("log save cfg file=%s", fname.toStdString().c_str());
	return saveConfig(config, fname);
}

bool Connection::saveConfigForLogs (QString const & preset_name)
{
	qDebug("%s this=0x%08x", __FUNCTION__, this);
	for (datalogs_t::iterator it = m_data.get<e_data_log>().begin(), ite = m_data.get<e_data_log>().end(); it != ite; ++it)
	{
		DataLog * const tbl = *it;
		QString const fname = getDataTagFileName(getConfig().m_appdir, preset_name, g_presetLogTag, tbl->m_config.m_tag);
		tbl->widget().onSaveButton();
	}
	return true;
}

	/*Connection * conn = server->findConnectionByName(app_name);
	if (conn)
	{
		qDebug("cmd setup: looking for app=%s: not found", app_name.toStdString().c_str());
		if (!m_main_window->clrFltEnabled())
		{
			m_file_model->beforeLoad();
			loadSessionState(conn->sessionState(), m_session_state);
		}

		QWidget * w = conn->m_tab_widget;
		server->onCloseTab(w);	// close old one
		// @TODO: delete persistent storage for the tab

		m_file_model->afterLoad();
	}
	else
	{
		qDebug("cmd setup: looking for app=%s: found", app_name.toStdString().c_str());
		m_file_model->beforeLoad();
		QString const pname = m_main_window->matchClosestPresetName(app_name);
		m_main_window->onPresetActivate(this, pname);
		m_file_model->afterLoad();
	}*/

datalogs_t::iterator Connection::findOrCreateLog (QString const & tag)
{
	datalogs_t::iterator it = dataWidgetFactory<e_data_log>(tag);
	if (it != m_data.get<e_data_log>().end())
	{
		(*it)->onShow();
	}
	else
		assert(false);
	return it;
}

void Connection::onTabTraceFocus ()
{
	setupModelFile();
	setupModelLvl();
	m_main_window->getWidgetCtx()->setModel(m_ctx_model);
	m_main_window->getWidgetTID()->setModel(m_tid_model);
	m_main_window->getWidgetColorRegex()->setModel(m_color_regex_model);
	m_main_window->getWidgetRegex()->setModel(m_regex_model);
	m_main_window->getWidgetString()->setModel(m_string_model);
	if (!m_curr_preset.isEmpty())
		m_main_window->setPresetNameIntoComboBox(m_curr_preset);
	m_main_window->setLastSearchIntoCombobox(m_last_search);
}

QString Connection::onCopyToClipboard ()
{
	QAbstractItemModel * model = m_table_view_widget->model();
	QItemSelectionModel * selection = m_table_view_widget->selectionModel();
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
		selected_text.append(model->data(current).toString());
		
		if (i + 1 < indexes.size() && current.row() != indexes.at(i + 1).row())
			selected_text.append('\n');	// switching rows
		else
			selected_text.append('\t');
	}
	return selected_text;
}

bool Connection::isModelProxy () const
{
	if (0 == m_table_view_widget->model())
		return false;
	return m_table_view_widget->model() == m_table_view_proxy;
}

void Connection::findTableIndexInFilters (QModelIndex const & src_idx, bool scroll_to_item, bool expand)
{
	{
		QString const file = findString4Tag(tlv::tag_file, src_idx);
		QString const line = findString4Tag(tlv::tag_line, src_idx);
		QString const combined = file + "/" + line;
		qDebug("findTableIndexInFilters %s in tree", combined.toStdString().c_str());
		m_file_model->selectItem(m_main_window->getWidgetFile(), combined, scroll_to_item);
		if (expand)
			m_file_model->expandItem(m_main_window->getWidgetFile(), combined);
	}
	{
		QString tid = findString4Tag(tlv::tag_tid, src_idx);
		QModelIndexList indexList = m_tid_model->match(m_tid_model->index(0, 0), Qt::DisplayRole, tid);
		if (!indexList.empty())
		{
			QModelIndex const selectedIndex(indexList.first());
			m_main_window->getWidgetTID()->setCurrentIndex(selectedIndex);
		}
	}
	{
		QString lvl = findString4Tag(tlv::tag_lvl, src_idx);
		QModelIndexList indexList = m_lvl_model->match(m_lvl_model->index(0, 0), Qt::DisplayRole, lvl);
		if (!indexList.empty())
		{
			QModelIndex const selectedIndex(indexList.first());
			m_main_window->getWidgetLvl()->setCurrentIndex(selectedIndex);
		}
	}
	{
		QString ctx = findString4Tag(tlv::tag_ctx, src_idx);
		QModelIndexList indexList = m_ctx_model->match(m_ctx_model->index(0, 0), Qt::DisplayRole, ctx);
		if (!indexList.empty())
		{
			QModelIndex const selectedIndex(indexList.first());
			m_main_window->getWidgetCtx()->setCurrentIndex(selectedIndex);
		}
	}
}

void Connection::onTableClicked (QModelIndex const & row_index)
{
	m_last_clicked = row_index;
	if (isModelProxy())
		m_last_clicked = m_table_view_proxy->mapToSource(row_index);

	m_last_search_row = m_last_clicked.row(); // set search from this line
	m_last_search_col = m_last_clicked.column();

	bool const scroll_to_item = false;
	bool const expand = false;
	findTableIndexInFilters(m_last_clicked, scroll_to_item, expand);
}

void Connection::onTableDoubleClicked (QModelIndex const & row_index)
{
	if (m_table_view_proxy)
	{
		QModelIndex const curr = m_table_view_proxy->mapToSource(row_index);
		LogTableModel * model = static_cast<LogTableModel *>(m_table_view_proxy ? m_table_view_proxy->sourceModel() : m_table_view_widget->model());
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
	}
}

void Connection::onApplyColumnSetup ()
{
	qDebug("%s", __FUNCTION__);
	for (int i = 0; i < m_table_view_widget->horizontalHeader()->count(); ++i)
	{
		//qDebug("column: %s", m_table_view_widget->horizontalHeader()->text());
	}

	QMap<int, int> order;

	if (sessionState().m_app_idx == -1)
		sessionState().m_app_idx = m_main_window->m_config.m_columns_setup.size() - 1;
	
	columns_setup_t const & new_cs = m_main_window->getColumnSetup(sessionState().m_app_idx);

	for (int i = 0, ie = new_cs.size(); i < ie; ++i)
	{
		tlv::tag_t const tag = tlv::tag_for_name(new_cs.at(i).toStdString().c_str());
		if (tag != tlv::tag_invalid)
		{
			order[tag] = i;
		}
	}

	if (0 == sessionState().m_columns_setup_current)
	{
	}
	else
	{
		columns_setup_t const & old_cs = *sessionState().m_columns_setup_current;
	}

	static_cast<TableView *>(m_table_view_widget)->setColumnOrder(order, m_session_state);
}

void Connection::onExcludeFileLine ()
{
	QModelIndex const current = m_last_clicked;
	if (current.isValid())
	{
		onExcludeFileLine(current);
		onInvalidateFilter();
	}
}

void Connection::onToggleRefFromRow ()
{
	QModelIndex const current = m_last_clicked;
	if (current.isValid())
	{
		qDebug("Toggle Ref from row=%i", current.row());
		m_session_state.setTimeRefFromRow(current.row());

		//LogTableModel * model = static_cast<LogTableModel *>(m_table_view_proxy ? m_table_view_proxy->sourceModel() : m_table_view_widget->model());
		QString const & strtime = findString4Tag(tlv::tag_time, current);
		m_session_state.setTimeRefValue(strtime.toULongLong());
		onInvalidateFilter();
	}
}

void Connection::onColorTagRow (int)
{
	QModelIndex current = m_table_view_widget->currentIndex();
	if (isModelProxy())
	{
		current = m_table_view_proxy->mapToSource(current);
	}

	int const row = current.row(); // set search from this line
	if (current.isValid())
	{
		qDebug("Color tag on row=%i", current.row());
		m_session_state.addColorTagRow(current.row());
		onInvalidateFilter();
	}
}

void Connection::onClearCurrentView ()
{
	//QModelIndex const curr = m_table_view_proxy->mapToSource(row_index);
	LogTableModel * model = static_cast<LogTableModel *>(m_table_view_proxy ? m_table_view_proxy->sourceModel() : m_table_view_widget->model());
	m_session_state.excludeContentToRow(model->rowCount());
	onInvalidateFilter();
}

void Connection::onHidePrevFromRow ()
{
	QModelIndex const current = m_last_clicked;
	if (current.isValid())
	{
		qDebug("Hide prev from row=%i", current.row());
		m_session_state.excludeContentToRow(current.row());
		onInvalidateFilter();
	}
}

void Connection::onUnhidePrevFromRow ()
{
	m_session_state.excludeContentToRow(0);
	onInvalidateFilter();
}

void Connection::onShowContextMenu (QPoint const & pos)
{
    QPoint globalPos = m_table_view_widget->mapToGlobal(pos);
    QAction * selectedItem = m_ctx_menu.exec(globalPos); // @TODO: rather async

	//poas = ui->tableView->viewport()->mapFromGlobal(e->globalPos());
	QModelIndex const idx = m_table_view_widget->indexAt(pos);
	qDebug("left click at r=%2i,c=%2i", idx.row(), idx.column());

	onTableClicked(idx);

    if (selectedItem == m_actions[e_action_HidePrev])
    {
		onHidePrevFromRow();
    }
    else if (selectedItem == m_actions[e_action_ToggleRef])
    {
		onToggleRefFromRow();
    }
    else if (selectedItem == m_actions[e_action_ExcludeFileLine])
	{
		onExcludeFileLine(m_last_clicked);
	}
    else if (selectedItem == m_actions[e_action_Find])
	{
		onFindFileLine(m_last_clicked);
	}
    else if (selectedItem == m_actions[e_action_Copy])
	{
		QString const & selection = onCopyToClipboard();
		qApp->clipboard()->setText(selection);
	}
    else if (selectedItem == m_actions[e_action_ColorTag])
	{
		onColorTagRow(m_last_clicked.row());
	}
    else if (selectedItem == m_actions[e_action_Setup])
	{
	}
    else
    { }
}

