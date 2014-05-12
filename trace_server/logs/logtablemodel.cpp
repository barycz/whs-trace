#include "logtablemodel.h"
#include <QBrush>
#include <QColor>
#include <QAbstractProxyModel>
#include <connection.h>
#include <trace_client/trace.h>
#include "filterproxymodel.h"
#include "findproxymodel.h"
#include <sysfn/time_query.h>

LogTableModel::LogTableModel (QObject * parent, logs::LogWidget & lw)
	: BaseTableModel(parent, lw.m_config.m_columns_setup, lw.m_config.m_columns_sizes)
	, m_log_widget(lw)
	, m_filter_state(lw.m_filter_state)
{
	qDebug("%s this=0x%08x", __FUNCTION__, this);
	size_t const prealloc_size = 128 * 1024;
	m_rows.reserve(prealloc_size); // @TODO: magic const!
}

LogTableModel::~LogTableModel ()
{
	qDebug("%s this=0x%08x", __FUNCTION__, this);
}

void LogTableModel::resizeToCfg (logs::LogConfig const & config)
{
	//beginResetModel();
	//emit layoutAboutToBeChanged();

	//@TODO: dedup: logtablemodel, findproxy, filterproxy
	if (config.m_columns_setup.size() > m_column_count)
	{
		int const last = static_cast<int>(config.m_columns_setup.size()) - 1;
		beginInsertColumns(QModelIndex(), m_column_count, last);
		insertColumns(m_column_count, last);
		m_column_count = last + 1;
		endInsertColumns();
	}
	else if (config.m_columns_setup.size() < m_column_count)
	{
		int const last = static_cast<int>(config.m_columns_setup.size()) + 1;
		beginRemoveColumns(QModelIndex(), last, m_column_count);
		removeColumns(last, m_column_count);
		m_column_count = last - 1;
		endInsertColumns();
	}

	//endResetModel();
}

void LogTableModel::postProcessBatch (int from, int to, BatchCmd const & batch)
{
	size_t const rows = batch.m_rows.size();
	for (size_t r = 0, re = rows; r < re; ++r)
	{
		DecodedCommand const & cmd = m_dcmds[from + r];
		m_log_widget.appendToColorizers(cmd);
	}

	FilterProxyModel * flt_pxy = m_log_widget.m_proxy_model;
	if (m_proxy && m_proxy == flt_pxy)
		flt_pxy->commitBatchToModel(static_cast<int>(from), static_cast<int>(to), batch);

	FindProxyModel * fnd_pxy = m_log_widget.m_find_proxy_model;
	if (m_proxy && m_proxy == fnd_pxy)
		fnd_pxy->commitBatchToModel(from, to, batch);

	m_log_widget.commitBatchToLinkedWidgets(from, to, batch);  
}

void LogTableModel::reloadModelAccordingTo (logs::LogConfig & config)
{
	resizeToCfg(config);
	for (size_t r = 0, re = m_dcmds.size(); r < re; ++r)
	{
		DecodedCommand const & dcmd = m_dcmds[r];
		handleCommand(dcmd, e_RecvBatched);
	}
	commitCommands(e_RecvSync);
  //emit headerDataChanged(Qt::Horizontal, 0, 10);// TODO TODO TODO TST
}

/*LogTableModel * LogTableModel::cloneToNewModel ()
{
	LogTableModel * new_model = new LogTableModel(this, m_log_widget);
	new_model->m_rows = m_rows;
	new_model->m_dcmds = m_dcmds;

	new_model->m_row_ctimes = m_row_ctimes;
	new_model->m_row_stimes = m_row_stimes;
	new_model->m_col_times = m_col_times;
	new_model->m_column_count = m_column_count;
	return new_model;
}*/

LogTableModel * LogTableModel::cloneToNewModel (logs::LogWidget * parent, FindConfig const & fc)
{
	LogTableModel * new_model = new LogTableModel(parent, *parent);
	for (size_t r = 0, re = m_rows.size(); r < re; ++r)
	{
		DecodedCommand const & dcmd = m_dcmds[r];
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
			new_model->m_rows.push_back(m_rows[r]);
			new_model->m_dcmds.push_back(m_dcmds[r]);
			new_model->m_row_ctimes.push_back(m_row_ctimes[r]);
			new_model->m_row_stimes.push_back(m_row_stimes[r]);
		}
	}
	new_model->m_col_times = m_col_times;
	new_model->m_column_count = m_column_count;
	return new_model;
}


void LogTableModel::parseCommand (DecodedCommand const & cmd, E_ReceiveMode mode, BatchCmd & batch)
{
	int column_index = -1;
	int thread_idx = -1;
	for (size_t i=0, ie=cmd.m_tvs.size(); i < ie; ++i)
		if (cmd.m_tvs[i].m_tag == tlv::tag_tid)
		{
			ThreadSpecific & tls = m_log_widget.getTLS();
			thread_idx = tls.findThreadId(cmd.m_tvs[i].m_val);
		}

	int indent = 0;
	QString qindent;
	if (m_log_widget.m_config.m_indent)
	{
		if (thread_idx >= 0)
			indent = m_log_widget.getTLS().m_indents[thread_idx];

		if (indent > 0)
		{
			if (cmd.m_hdr.cmd == tlv::cmd_scope_exit)
				--indent; // indent is decreased after this call, that's why

			for(int j = 0; j < indent; ++j)
				qindent.append("  ");	// @TODO: ugh
		}
	}

	batch.m_rows.push_back(columns_t(cmd.m_tvs.size()));
	batch.m_dcmds.push_back(cmd);
	batch.m_dcmds.back().m_indent = indent;
	batch.m_dcmds.back().m_row_type = cmd.m_hdr.cmd;
	columns_t & columns = batch.m_rows.back();
	columns.reserve(cmd.m_tvs.size());

	size_t n = cmd.m_tvs.size();
	if (cmd.m_hdr.cmd == tlv::cmd_scope_entry || (cmd.m_hdr.cmd == tlv::cmd_scope_exit))
		n = n + 1;

	QString file;
	QString line;
	QString func;
	QString time;
	for (size_t i = 0, ie = cmd.m_tvs.size(); i < ie; ++i)
	{
		tlv::tag_t const tag = cmd.m_tvs[i].m_tag;
		QString const & val = cmd.m_tvs[i].m_val;
		if (tag == tlv::tag_file)
			file = val;
		if (tag == tlv::tag_line)
			line = val;
		if (tag == tlv::tag_func)
			func = val;
		if (tag == tlv::tag_ctime)
			time = val;

		QString qval;
		if (tag == tlv::tag_msg)
		{
			if (cmd.m_hdr.cmd == tlv::cmd_scope_entry)
			{
				qval = qindent + QString("{ ");
			}
			else if (cmd.m_hdr.cmd == tlv::cmd_scope_exit)
			{
				qval = qindent + QString("} ");
			}
			else
			{
				qval.append(qindent);
			}
		}
		qval.append(val);

		column_index = m_log_widget.findColumn4Tag(tag);
		if (column_index >= 0)
		{
			if (columns.size() <= column_index + 1)
				columns.resize(column_index + 1);
			columns[column_index].m_value = qval;
		}
	}

	sys::hptimer_t const now = sys::queryTime_us();
	unsigned long long const last_t = m_log_widget.getTLS().lastTime(thread_idx);
	unsigned long long const t = time.toULongLong();
	long long const dt = t - last_t;
	// dt + stime
	{
		int ci = m_log_widget.findColumn4Tag(tlv::tag_dt);
		if (ci >= 0)
		{
			//if (ci < 0)
				//ci = m_log_widget.appendColumn(tlv::tag_dt);

			if (columns.size() <= ci + 1)
				columns.resize(ci + 1);

			columns[ci].m_value = tr("%1").arg(dt);
		}

		m_log_widget.getTLS().setLastTime(thread_idx, t);

		batch.m_row_ctimes.push_back(t);
		batch.m_row_stimes.push_back(now);

		// stime
		int sti = m_log_widget.findColumn4Tag(tlv::tag_stime);
		if (sti >= 0)
		{
			if (sti < 0)
				sti = m_log_widget.appendColumn(tlv::tag_stime);
			columns[sti].m_value = tr("%1").arg(now);
		}
	}
}

void LogTableModel::commitBatchToModel (BatchCmd & batch)
{
	size_t const rows = batch.m_rows.size();
	size_t const from = m_rows.size();
	m_dcmds.resize(from + rows);
	m_rows.resize(from + rows);
	m_row_ctimes.resize(from + rows);
	m_row_stimes.resize(from + rows);
	int const to = static_cast<int>(from) + static_cast<int>(rows) - 1;
	beginInsertRows(QModelIndex(), static_cast<int>(from), to);
	for (size_t r = 0, re = batch.m_rows.size(); r < re; ++r)
	{
		m_rows[from + r] = batch.m_rows[r];
		m_dcmds[from + r] = batch.m_dcmds[r];
		m_dcmds[from + r].m_src_row = from + r;
		m_row_ctimes[from + r] = batch.m_row_ctimes[r];
		m_row_stimes[from + r] = batch.m_row_stimes[r];
	}
	endInsertRows();

	postProcessBatch(from, to + 1, batch); // hook to linked models

	batch.clear();
}



/*

QVariant LogTableModel::data (const QModelIndex &index, int role) const
{
	if (!index.isValid())
		return QVariant();
	if (role == Qt::DisplayRole || role == Qt::ToolTipRole)
	{
		QString str("");
		if (checkExistence(index))
		{
			return m_rows[index.row()][index.column()];
		}
		return str;
	}

	bool const color_set = (role == Qt::BackgroundRole || role == Qt::ForegroundRole);

	if (role == Qt::BackgroundRole)
	{
		if (checkColumnExistence(tlv::tag_msg, index))
		{
			int const column_idx = m_log_widget.findColumn4Tag(tlv::tag_tid);
			if (column_idx != -1)
			{
				QString const & tid = m_rows[index.row()][column_idx];

				bool const is_collapsed = m_filter_state.isBlockCollapsedIncl(tid, index.row());
				if (is_collapsed)
					return QBrush(Qt::lightGray);
			}
		}

		if (checkColumnExistence(tlv::tag_tid, index))
		{
			QString const & tid = m_rows[index.row()][index.column()];
			int const idx = m_log_widget.getTLS().findThreadId(tid);
			if (idx >= 0)
				return QBrush(m_log_widget.m_config.m_thread_colors[idx]);
		}
		if (checkColumnExistence(tlv::tag_lvl, index))
		{
			QString const & lvl = m_rows[index.row()][index.column()];
            if (lvl.toInt() == trace::e_Fatal)
				return QBrush(Qt::black);
			if (lvl.toInt() == trace::e_Error)
				return QBrush(Qt::red);
			if (lvl.toInt() == trace::e_Warning)
				return QBrush(Qt::yellow);
		}

	}
	if (role == Qt::ForegroundRole)
	{
		if (checkColumnExistence(tlv::tag_lvl, index))
		{
			QString const & lvl = m_rows[index.row()][index.column()];
			if (lvl.toInt() == trace::e_Fatal)
				return QBrush(Qt::white);
		}
	}

	return QVariant();
}

QVariant LogTableModel::headerData (int section, Qt::Orientation orientation, int role) const
{
	if (role == Qt::DisplayRole) {
		if (orientation == Qt::Horizontal) {
			if (0 <= section && section < (int)m_log_widget.m_config.m_columns_setup.size())
				return m_log_widget.m_config.m_columns_setup[section];
		}
	}
	return QVariant();
}

void LogTableModel::appendCommandCSV (QAbstractProxyModel * filter, tlv::StringCommand const & cmd)
{
	m_rows.push_back(columns_t(cmd.tvs.size()));

	QString msg;
	for (size_t i = 0, ie = cmd.tvs.size(); i < ie; ++i)
	{
		tlv::tag_t const tag = cmd.tvs[i].m_tag;
		QString const & val = cmd.tvs[i].m_val;
		if (tag == tlv::tag_msg)
			msg = val;
	}

	//QStringList list = msg.split(QRegExp(separator), QString::SkipEmptyParts);
	QStringList const list = msg.split(m_log_widget.m_csv_separator);
	columns_t & columns = m_rows.back();
	columns.resize(list.size());
	for (int i = 0, ie = list.size(); i < ie; ++i)
	{
		QString const & col_value = list.at(i);
		int column_index = m_log_widget.findColumn4Tag(i);
		if (column_index < 0)
		{
			column_index = m_log_widget.appendColumn(i);
			beginInsertColumns(QModelIndex(), column_index, column_index + 3);
			insertColumns(column_index, 3);
			endInsertColumns();

			if (filter)
			{
				filter->insertColumn(column_index);
			}
		}

		//QModelIndex const idx = createIndex(m_rows.size() - 1, column_index, 0);
		QModelIndex const idx = index(m_rows.size() - 1, column_index, QModelIndex());
		setData(idx, col_value, Qt::EditRole);
	}

	//m_table_view_widget->horizontalHeader()->resizeSections(QHeaderView::Fixed);
	//m_table_view_widget->horizontalHeader()->setResizeMode(QHeaderView::Fixed);
	//m_table_view_widget->verticalHeader()->setResizeMode(QHeaderView::Fixed);
	//m_table_view_widget->setVisible(true);


	if (filter)
	{
		int const row = filter->rowCount();
		filter->insertRow(row);
	}
	else
	{
		int const row = rowCount();
		insertRow(row);
	}
}
*/
