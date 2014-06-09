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
#include <basetablemodel.h>
#include <filterstate.h>
#include "findconfig.h"
#include <cmd.h>
#include <3rd/assocvector.h>

class Connection;
class FilterState;
class QAbstractProxyModel;

namespace logs { class LogWidget; struct LogConfig; }

class LogTableModel : public BaseTableModel
{
public:
	explicit LogTableModel (QObject * parent, logs::LogWidget & lw);
	~LogTableModel ();

	void appendCommand (DecodedCommand const & cmd);

	//bool checkExistence (QModelIndex const & index) const;
	//bool checkColumnExistence (tlv::tag_t tag, QModelIndex const & index) const;
	//bool checkTagExistence (tlv::tag_t tag, QModelIndex const & index) const;

	void resizeToCfg (logs::LogConfig const & config);

	FilterState const & filterState () const { return m_filter_state; }
    logs::LogWidget const & logWidget () const { return m_log_widget; }
	dcmds_t const & dcmds () { return m_dcmds; }
	LogTableModel * cloneToNewModel (logs::LogWidget * parent, FindConfig const & fc);
    void reloadModelAccordingTo (logs::LogConfig & config);

	int column2Tag (int col) const;
	int tag2Column (int tag) const;
	int storage2Column (int storage_col) const { return storage_col < m_storage2columns.size() ? m_storage2columns[storage_col] : -1; }
	int appendColumnAndTag (int tag); 

	virtual void clearModel ();

signals:
	
public slots:

protected:
	friend class logs::LogWidget;

	virtual void parseCommand (DecodedCommand const & cmd, E_ReceiveMode mode, BatchCmd & batch);
	virtual void commitBatchToModel (BatchCmd & batch); 
	void postProcessBatch (int src_from, int src_to, BatchCmd const & batch);

	logs::LogWidget & m_log_widget;
	std::vector<int> m_columns2storage;
	std::vector<int> m_columns2tag;
	typedef Loki::AssocVector<int, int> tags2column_t; // @TODO: reserve
	tags2column_t m_tags2column;
	std::vector<int> m_storage2columns;
	FilterState & m_filter_state;
};

