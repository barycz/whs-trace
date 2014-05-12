#include "tablemodel.h"
#include <QBrush>
#include <QColor>
#include <QAbstractProxyModel>
#include <trace_client/trace.h>
#include <sysfn/time_query.h>

TableModel::TableModel (QObject * parent, std::vector<QString> & hhdr, std::vector<int> & hsize)
	: BaseTableModel(parent, hhdr, hsize)
{
	qDebug("%s this=0x%08x", __FUNCTION__, this);
}

TableModel::~TableModel ()
{
	qDebug("%s this=0x%08x", __FUNCTION__, this);
}

void TableModel::parseCommand (DecodedCommand const & cmd, E_ReceiveMode mode, BatchCmd & batch)
{
	switch (cmd.m_hdr.cmd)
	{
		case tlv::cmd_table_xy:			handleTableXYCommand(cmd, mode); break;
		case tlv::cmd_table_setup:		handleTableSetupCommand(cmd, mode); break;
		case tlv::cmd_table_clear:		handleTableClearCommand(cmd, mode); break;
	}
}

bool TableModel::handleTableXYCommand (DecodedCommand const & cmd, E_ReceiveMode mode)
{
	QString tag;
	if (!cmd.getString(tlv::tag_msg, tag)) return true;
	QString ctime;
	if (!cmd.getString(tlv::tag_ctime, ctime)) return true;
	int x = 0;
	if (!cmd.get(tlv::tag_ix, x)) return true;
	int y = 0;
	if (!cmd.get(tlv::tag_iy, y)) return true;
	QString fgc;
	cmd.getString(tlv::tag_fgc, fgc);
	QString bgc;
	cmd.getString(tlv::tag_bgc, bgc);

	parseTableXY(x, y, ctime, fgc, bgc, tag, m_batch);
	return true;
}

bool TableModel::handleTableSetupCommand (DecodedCommand const & cmd, E_ReceiveMode mode)
{
	QString tag;
	if (!cmd.getString(tlv::tag_msg, tag)) return true;
	QString ctime;
	if (!cmd.getString(tlv::tag_ctime, ctime)) return true;
	int x = 0;
	if (!cmd.get(tlv::tag_ix, x)) return true;
	int y = 0;
	if (!cmd.get(tlv::tag_iy, y)) return true;
	QString fgc;
	cmd.getString(tlv::tag_fgc, fgc);
	QString bgc;
	cmd.getString(tlv::tag_bgc, bgc);
	QString hhdr;
	cmd.getString(tlv::tag_hhdr, bgc);

    //qDebug("table: setup hdr: x=%i y=%i hhdr=%s fg=%s bg=%s", x, y, hhdr.toStdString().c_str(), fgc.toStdString().c_str(), bgc.toStdString().c_str());
	parseTableSetup(x, y, ctime, fgc, bgc, hhdr, tag, m_batch);
	return true;
}

bool TableModel::handleTableClearCommand (DecodedCommand const & cmd, E_ReceiveMode mode)
{
	QString msg;
	if (!cmd.getString(tlv::tag_msg, msg)) return true;

	QString tag = msg;
	int const slash_pos = tag.lastIndexOf(QChar('/'));
	tag.chop(msg.size() - slash_pos);

	QString subtag = msg;
	subtag.remove(0, slash_pos + 1);

	//dataplots_t::iterator it = m_dataplots.find(tag);
	//if (it != m_dataplots.end())
	//	(*it)->clearCurveData(subtag);
	return true;
}

void TableModel::parseTableXY (int x, int y, QString const & time, QString const & fgc, QString const & bgc, QString const & cmd, BatchCmd & batch)
{
	unsigned long long t = time.toULongLong();
	QStringList const values = cmd.split("|");
	int const n_cols = values.size();

	bool new_rows = false;
	int rows_first = 0;
	int rows_last = 0;

	bool new_cols = false;
	int cols_first = 0;
	int cols_last = 0;


	if (y < 0)
	{
		size_t const curr_sz = m_rows.size();
		m_rows.push_back(columns_t());
		m_row_ctimes.push_back(t);
		sys::hptimer_t const now = sys::queryTime_us();
		m_row_stimes.push_back(now);
		if (x < 0)
		{
			m_rows.back().resize(n_cols);
			m_col_times.resize(n_cols);
			x = 0;
		}
		else
		{
			m_rows.back().resize(x + n_cols);
			m_col_times.resize(x + n_cols);
		}
		y = m_rows.size() - 1;

		new_rows = true;
		rows_first = curr_sz;
		rows_last = y;
	}
	else
	{
		if (y >= m_rows.size())
		{
			size_t const curr_sz = m_rows.size();
			m_rows.resize(y + 1);
			m_row_ctimes.resize(y + 1);
			m_row_stimes.resize(y + 1);
			for (size_t r = curr_sz; r < y + 1; ++r)
			{
				m_row_ctimes[r] = t + r * 10;
				sys::hptimer_t const now = sys::queryTime_us();
				m_row_stimes[r] = now;
			}

			new_rows = true;
			rows_first = curr_sz;
			rows_last = y;
		}

/*		if (x < 0)
		{
			++m_columnCount;
			size_t const curr_sz = m_rows[y].size();
			m_rows[y].resize(curr_sz + 1);
			x = curr_sz;
			// @TODO: updatnout size kontejneru s predchozim poctem elementu
			//
		}
		else*/
		{
			//qDebug("	append: x>=0 ", m_rows.back().size(), x + n_cols);
			if (x + n_cols >= m_rows.back().size())
			{
				//qDebug("	append: x>=0  resize %i -> %i", m_rows.back().size(), x + n_cols);
				m_rows.back().resize(x + n_cols);
				m_col_times.resize(x + n_cols);

			}
		}
	}

	if (m_column_count < x + n_cols)
	{
		new_cols = true;
		cols_first = m_column_count;
		cols_last = x + n_cols - 1;
	}
	for (int iy = 0, iye = m_rows.size(); iy < iye; ++iy)
	{
		int const new_size =  x + n_cols;
		if (m_rows[iy].size() < new_size)
		{
			m_rows[iy].resize(new_size);
		}
	}

	for (int ix = x, ixe = x + n_cols; ix < ixe; ++ix)
	{
		m_rows[y][ix].m_value = values.at(ix - x);
		m_rows[y][ix].m_fgc = fgc;
		m_rows[y][ix].m_bgc = bgc;
		m_col_times[ix] = t;
	}

	if (new_rows)
	{
		beginInsertRows(QModelIndex(), rows_first, rows_last);
		//qDebug("mod  -  beginInsertRows(%02i, %02i) ", rows_first, rows_last);
		endInsertRows();

	}

	if (new_cols)
	{
		beginInsertColumns(QModelIndex(), cols_first, cols_last);
		qDebug("mod  COL  beginInsertCols(%02i, %02i) ", cols_first, cols_last);
		insertColumns(cols_first, cols_last);
		if (m_column_count < cols_last + 1)
			m_column_count = cols_last + 1;
		endInsertColumns();
	}

	for (int ix = x, ixe = x + n_cols; ix < ixe; ++ix)
	{
		QModelIndex const idx = index(y, ix, QModelIndex());
		emit dataChanged(idx, idx);
		if (m_proxy)
			m_proxy->setData(idx, m_rows[y][ix].m_value, Qt::EditRole);
	}
}

void TableModel::parseTableSetup (int x, int y, QString const & time, QString const & fgc, QString const & bgc, QString const & hhdr, QString const & tag, BatchCmd & batch)
{
	unsigned long long t = time.toULongLong();
	if (!hhdr.isEmpty() && x > -1)
	{
		setHeaderData(x, Qt::Horizontal, hhdr, Qt::EditRole);
		qDebug("header h[x=%2i, y=%2i] = %s", x, y, hhdr.toStdString().c_str());
	}

	if (!fgc.isEmpty() && x >= 0 && y >= 0)
	{
		createRows(t, y, y, QModelIndex());
		createColumns(t, x, x, QModelIndex());
		QModelIndex const & idx = index(y, x, QModelIndex());
		setData(idx, QColor(fgc), Qt::ForegroundRole);
	}

	if (!bgc.isEmpty() && x >= 0 && y >= 0)
	{
		createRows(t, y, y, QModelIndex());
		createColumns(t, x, x, QModelIndex());
		QModelIndex const & idx = index(y, x, QModelIndex());
		setData(idx, QColor(bgc), Qt::BackgroundRole);
	}
}

void TableModel::commitBatchToModel (BatchCmd & batch)
{
}

void TableModel::postProcessBatch (int src_from, int src_to, BatchCmd const & batch)
{
}
