/**
 * Copyright (C) 2011 Mojmir Svoboda
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is furnished to do
 * so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 **/
#pragma once
#include <QAbstractTableModel>
#include <QAbstractProxyModel>
#include "cmd.h"
#include "types.h"
#include <QString>
#include <QColor>
#include <vector>
#include <tlv_parser/tlv_parser.h>

struct Cell
{
	Cell () : m_value(), m_fgc(), m_bgc() { }
	Cell (QVariant const & v) : m_value(v), m_fgc(), m_bgc() { }
	Cell (QVariant const & v, QVariant const & fgc) : m_value(v), m_fgc(fgc), m_bgc() { }
	Cell (QVariant const & v, QVariant const & fgc, QVariant const & bgc) : m_value(v), m_fgc(fgc), m_bgc(bgc) { }
	QVariant m_value;
	QVariant m_fgc;
	QVariant m_bgc;
};

typedef std::vector<Cell> columns_t;
typedef std::vector<columns_t> rows_t;

typedef std::vector<DecodedCommand> dcmds_t;

typedef std::vector<unsigned long long> times_t;
struct BatchCmd {
	dcmds_t m_dcmds;
	rows_t m_rows;
	times_t m_row_ctimes;
	times_t m_row_stimes;

	void clear ()
	{
		m_dcmds.clear();
		m_rows.clear();
		m_row_ctimes.clear();
		m_row_stimes.clear();
	}
};

class BaseTableModel : public QAbstractTableModel
{
	//Q_OBJECT
public:
	explicit BaseTableModel (QObject * parent, std::vector<QString> & hhdr, std::vector<int> & hsize);
	~BaseTableModel ();
	int rowCount (const QModelIndex & parent = QModelIndex()) const;
	int columnCount (const QModelIndex & parent = QModelIndex()) const;

	QVariant data (const QModelIndex & index, int role = Qt::DisplayRole) const;
	bool setData (QModelIndex const & index, QVariant const & value, int role = Qt::EditRole);

	QVariant headerData (int section, Qt::Orientation orientation, int role) const;
	bool  setHeaderData (int section, Qt::Orientation orientation, QVariant const & value, int role = Qt::EditRole);

	virtual void handleCommand (DecodedCommand const & cmd, E_ReceiveMode mode);
	virtual void commitCommands (E_ReceiveMode mode);

	void createCell (unsigned long long time, int x, int y);
	void createRows (unsigned long long time, int first, int last, QModelIndex const &);
	void createColumns (unsigned long long time, int first, int last, QModelIndex const & parent = QModelIndex());

	void emitLayoutChanged ();

	bool checkExistence (QModelIndex const & index) const;

	void setProxy (QAbstractProxyModel * pxy) { m_proxy = pxy; }
	unsigned long long row_ctime (int const row) const { return m_row_ctimes[row]; }
	unsigned long long row_stime (int const row) const { return m_row_stimes[row]; }
	unsigned long long col_time (int const col) const { return m_col_times[col]; }

	void clearModel ();
	void clearModelData ();

protected:

	virtual void commitBatchToModel (BatchCmd & batch) = 0; 
	virtual void parseCommand (DecodedCommand const & cmd, E_ReceiveMode mode, BatchCmd & batch) = 0;

	typedef std::vector<unsigned long long> times_t;
	times_t m_row_ctimes;
	times_t m_row_stimes;
	times_t m_col_times;
	rows_t m_rows;
	int m_column_count;
	std::vector<QString> & m_hhdr;
	std::vector<int> & m_hsize;
	QAbstractProxyModel * m_proxy;

	BatchCmd m_batch;
	dcmds_t m_dcmds;
};

