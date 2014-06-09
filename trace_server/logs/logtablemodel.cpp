#include "logtablemodel.h"
#include <QBrush>
#include <QColor>
#include <QAbstractProxyModel>
#include <connection.h>
#include <trace_client/trace.h>
#include "filterproxymodel.h"
#include "findproxymodel.h"
#include "utils.h"
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


int LogTableModel::column2Tag (int col) const
{
	if (col >= 0 && col < m_columns2tag.size())
		return m_columns2tag[col];
	return -1;
}
int LogTableModel::tag2Column (int tag) const
{
	tags2column_t::const_iterator it = m_tags2column.find(tag);
	if (it != m_tags2column.end())
		return it->second;

	return -1;
}

/*
int LogTableModel::appendColumnAndTag (int tag)
{
	TagDesc const & desc = m_log_widget.m_tagconfig.findOrCreateTag(tag);

	m_log_widget.m_config.m_columns_setup.push_back(desc.m_tag_str);
	m_log_widget.m_config.m_columns_align.push_back(desc.m_align_str);
	m_log_widget.m_config.m_columns_elide.push_back(desc.m_elide_str);
	m_log_widget.m_config.m_columns_sizes.push_back(desc.m_size);

	//qDebug("inserting column and size. tmpl_sz=%u curr_sz=%u sizes_sz=%u", m_columns_setup_template->size(), m_columns_setup_current->size(), m_columns_sizes->size());
	int const column_index = static_cast<int>(m_log_widget.m_config.m_columns_setup.size()) - 1;
	m_tags2columns.insert(std::make_pair(tag, column_index));
	m_columns2tag.resize(column_index);
	m_columns2tag[column_index] = tag;
	//m_columns2storage ?
	//rest?
	return column_index;
}
*/


/*
int LogTableModel::findColumn4Tag (int tag)
{
	QMap<int, int>::const_iterator it = m_tags2columns.find(tag);
	if (it != m_tags2columns.end())
		return it.value();

	QString qname;
	char const * name = tlv::get_tag_name(tag);
	if (name)
	{
		qname = QString(name);
	}
	else
	{
		qname = tr("Col%1").arg(tag - tlv::tag_max_value);
	}
	
	for (size_t i = 0, ie = m_log_widget.m_config.m_columns_setup.size(); i < ie; ++i)
		if (m_log_widget.m_config.m_columns_setup[i] == qname)
		{
			m_tags2columns.insert(std::make_pair(tag, static_cast<int>(i)));
			return static_cast<int>(i);
		}
	return -1;
}*/

/*int LogTableModel::appendColumn (int tag)
{
	TagDesc const & desc = m_log_widget.m_tagconfig.findOrCreateTag(tag);

	m_log_widget.m_config.m_columns_setup.push_back(desc.m_tag_str);
	m_log_widget.m_config.m_columns_align.push_back(desc.m_align_str);
	m_log_widget.m_config.m_columns_elide.push_back(desc.m_elide_str);
	m_log_widget.m_config.m_columns_sizes.push_back(desc.m_size);

	//qDebug("inserting column and size. tmpl_sz=%u curr_sz=%u sizes_sz=%u", m_columns_setup_template->size(), m_columns_setup_current->size(), m_columns_sizes->size());
	int const column_index = static_cast<int>(m_log_widget.m_config.m_columns_setup.size()) - 1;
	//m_tags2columns.insert(std::make_pair(tag, column_index));
	m_columns2tag.resize(column_index);
	m_columns2tag[column_index] = tag;
	return column_index;
}*/

namespace tlv {
	inline size_t name_to_tag (QString const & str)
	{
		for (size_t i = 0; i < tlv::tag_max_value; ++i)
			if (str == QString(tag_names[i]))
				return i;

		if (str.size() >= 4)
		{
			QStringRef num = str.rightRef(4); // skipping the "Col" part
			bool ok = false;
			int col_n = num.toInt(&ok);
			if (ok)
				return tlv::tag_max_value + col_n;
		}

		return tag_invalid;
	}
}

void LogTableModel::parseCommand (DecodedCommand const & cmd, E_ReceiveMode mode, BatchCmd & batch)
{
	if (m_log_widget.protocol() == e_Proto_CSV) // @TODO: convert to inheritance
	{
		// parse the raw message
		QString msg = QString(QLatin1String(&cmd.m_orig_message[0]));

		QStringList l;
		if (m_log_widget.m_config.m_csv_separator.isEmpty())
		{
			l << msg;
		}
		else
		{
			l = msg.split(m_log_widget.m_config.m_csv_separator);
		}

		batch.m_rows.push_back(columns_t(l.size()));
		batch.m_dcmds.push_back(cmd);
		batch.m_dcmds.back().m_indent = 0;
		batch.m_dcmds.back().m_row_type = cmd.m_hdr.cmd;
		columns_t & columns = batch.m_rows.back();

		for (int i = 0, ie = l.size(); i < ie; ++i)
		{
			tlv::TV tv;
			tv.m_val = l.at(i);
			// @TODO: if tag is known tag such as STime, CTime, ... then make it that one
			tlv::tag_t const tag = tlv::tag_max_value + i;
			tv.m_tag = tag;
			batch.m_dcmds.back().m_tvs.push_back(tv);
		}


		bool const has_no_setup = m_log_widget.m_config.m_columns_setup.size() == 0;

		if (has_no_setup)
		{
			m_columns2storage.clear();
			m_columns2tag.clear();
			m_storage2columns.clear();
			m_tags2column.clear();
			m_columns2storage.resize(m_log_widget.m_config.m_storage_order.size());
			m_columns2tag.resize(m_log_widget.m_config.m_storage_order.size());
			m_storage2columns.resize(m_log_widget.m_config.m_storage_order.size());
			for (int c = 0, ce = m_log_widget.m_config.m_storage_order.size(); c < ce; ++c)
			{
				tlv::tag_t const tag = tlv::name_to_tag(m_log_widget.m_config.m_storage_order[c]);
				m_columns2storage[c] = c;
				m_columns2tag[c] = tag;
				m_storage2columns[c] = c;
				m_tags2column.insert(std::make_pair(tag, c));
			}
			resizeToCfg(m_log_widget.m_config);
		}
		else
		{
			if (m_columns2storage.size() == 0)
			{
				m_columns2storage.resize(m_log_widget.m_config.m_columns_setup.size());
				m_columns2tag.resize(m_log_widget.m_config.m_columns_setup.size());
				m_storage2columns.resize(m_log_widget.m_config.m_storage_order.size());
				for (size_t i = 0, ie = m_log_widget.m_config.m_columns_setup.size(); i < ie; ++i)
					for (int c = 0, ce = m_log_widget.m_config.m_storage_order.size(); c < ce; ++c)
					{
						if (m_log_widget.m_config.m_columns_setup[i] == m_log_widget.m_config.m_storage_order[c])
						{
							tlv::tag_t const tag = tlv::name_to_tag(m_log_widget.m_config.m_storage_order[c]);
							m_columns2storage[i] = c;
							m_columns2tag[i] = tag;
							m_storage2columns[c] = i;
							m_tags2column.insert(std::make_pair(tag, i));
							break;
						}
					}

				resizeToCfg(m_log_widget.m_config);

				//@TODO: ask on forum what is the correct way
				//for (size_t i = 0, ie = m_log_widget.m_config.m_columns_setup.size(); i < ie; ++i)
				//	m_log_widget.m_tableview->horizontalHeader()->resizeSection(i, m_log_widget.m_config.m_columns_sizes[i]);
			}
		}

		for (int i = 0, ie = m_columns2storage.size(); i < ie; ++i)
		{
			int const src = m_columns2storage[i];
			if (src < l.size())
			{
				QString const str =  unquoteString(l.at(src), m_log_widget.m_config.m_unquote_strings, m_log_widget.m_config.m_simplify_strings);
				columns[i].m_value = str;
			}
			else
			{
				qWarning("CSV parsing error at line: %s", qPrintable(msg));
			}
		}


		sys::hptimer_t const now = sys::queryTime_us();
		batch.m_row_ctimes.push_back(now); //@TODO: unless there is a time tag assigned to column
		batch.m_row_stimes.push_back(now);
	}
	else
	{
		size_t const generated_cols = 2; // dt + stime
		int column_index = -1;
		int thread_idx = -1;

		bool const has_no_setup = m_log_widget.m_config.m_columns_setup.size() == 0;
		bool const has_no_storage_order=  m_log_widget.m_config.m_storage_order.size() == 0;

		if (has_no_storage_order)
		{
			m_log_widget.m_config.m_storage_order.reserve(cmd.m_tvs.size() + generated_cols);
			for (size_t i = 0, ie = cmd.m_tvs.size(); i < ie; ++i)
			{
				tlv::tag_t const tag = cmd.m_tvs[i].m_tag;
				QString const name = tlv::get_tag_name(tag);
				m_log_widget.m_config.m_storage_order.push_back(name);
			}

			m_log_widget.m_config.m_storage_order.push_back(tlv::get_tag_name(tlv::tag_dt));
			m_log_widget.m_config.m_storage_order.push_back(tlv::get_tag_name(tlv::tag_stime));
		}

		if (has_no_setup)
		{
			m_columns2storage.clear();
			m_columns2tag.clear();
			m_storage2columns.clear();
			m_tags2column.clear();
			m_columns2storage.resize(m_log_widget.m_config.m_storage_order.size());
			m_columns2tag.resize(m_log_widget.m_config.m_storage_order.size());
			m_storage2columns.resize(m_log_widget.m_config.m_storage_order.size());
			for (int c = 0, ce = m_log_widget.m_config.m_storage_order.size(); c < ce; ++c)
			{
				tlv::tag_t const tag = tlv::name_to_tag(m_log_widget.m_config.m_storage_order[c]);
				m_columns2storage[c] = c;
				m_columns2tag[c] = tag;
				m_storage2columns[c] = c;
				m_tags2column.insert(std::make_pair(tag, c));
			}
			resizeToCfg(m_log_widget.m_config);
		}
		else
		{
			if (m_columns2storage.size() == 0)
			{
				m_columns2storage.resize(m_log_widget.m_config.m_columns_setup.size());
				m_columns2tag.resize(m_log_widget.m_config.m_columns_setup.size());
				m_storage2columns.resize(m_log_widget.m_config.m_storage_order.size());
				for (size_t i = 0, ie = m_log_widget.m_config.m_columns_setup.size(); i < ie; ++i)
					for (int c = 0, ce = m_log_widget.m_config.m_storage_order.size(); c < ce; ++c)
					{
						if (m_log_widget.m_config.m_columns_setup[i] == m_log_widget.m_config.m_storage_order[c])
						{
							tlv::tag_t const tag = tlv::name_to_tag(m_log_widget.m_config.m_storage_order[c]);
							m_columns2storage[i] = c;
							m_columns2tag[i] = tag;
							m_storage2columns[c] = i;
							m_tags2column.insert(std::make_pair(tag, i));
							break;
						}
					}

				resizeToCfg(m_log_widget.m_config);

				//@TODO: ask on forum what is the correct way
				//for (size_t i = 0, ie = m_log_widget.m_config.m_columns_setup.size(); i < ie; ++i)
				//	m_log_widget.m_tableview->horizontalHeader()->resizeSection(i, m_log_widget.m_config.m_columns_sizes[i]);
			}
		}

		// prepare indent for message w respect to thread of execution
		QString tid;
		if (cmd.getString(tlv::tag_tid, tid))
		{
			ThreadSpecific & tls = m_log_widget.getTLS();
			thread_idx = tls.findThreadId(tid);
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

		batch.m_rows.push_back(columns_t());
		batch.m_dcmds.push_back(cmd);
		batch.m_dcmds.back().m_indent = indent;
		batch.m_dcmds.back().m_row_type = cmd.m_hdr.cmd;
		columns_t & columns = batch.m_rows.back();
		columns.reserve(cmd.m_tvs.size() + generated_cols);

		DecodedCommand & cmd_copy = batch.m_dcmds.back();

		{ // generated columns
			QString time;
			cmd_copy.getString(tlv::tag_ctime, time);
			sys::hptimer_t const now = sys::queryTime_us();
			unsigned long long const last_t = m_log_widget.getTLS().lastTime(thread_idx);
			unsigned long long const t = time.toULongLong();
			long long const dt = t - last_t;

			m_log_widget.getTLS().setLastTime(thread_idx, t);
			cmd_copy.m_tvs.push_back(tlv::TV(tlv::tag_dt, tr("%1").arg(dt))); // dt
			cmd_copy.m_tvs.push_back(tlv::TV(tlv::tag_stime, tr("%1").arg(now))); // STime
			batch.m_row_ctimes.push_back(t);
			batch.m_row_stimes.push_back(now);
		}

		for (size_t c = 0, ce = m_columns2storage.size(); c < ce; ++c)
		{
			int const i = m_columns2storage[c];
			tlv::tag_t const tag = cmd_copy.m_tvs[i].m_tag;
			QString const & val = cmd_copy.m_tvs[i].m_val;

			QString indented_val;
			if (tag == tlv::tag_msg) // indent message w respect to thread of execution
			{
				if (cmd_copy.m_hdr.cmd == tlv::cmd_scope_entry)
				{
					indented_val = qindent + QString("{ ");
				}
				else if (cmd_copy.m_hdr.cmd == tlv::cmd_scope_exit)
				{
					indented_val = qindent + QString("} ");
				}
				else
				{
					indented_val.append(qindent);
				}
			}
			indented_val.append(val);

			columns.push_back(Cell());
			columns.back().m_value = indented_val;
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
	if (from == -1 || to == -1)
		return;
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

	if (from == 0)
		m_log_widget.resizeSections(); // @NOTE: QHeaderView::resizeSection does not work if there are no physical section
}

void LogTableModel::clearModel ()
{
	BaseTableModel::clearModel();
	m_columns2storage.clear();
	m_columns2tag.clear();
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
			int const column_idx = findColumn4Tag(tlv::tag_tid);
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


*/
