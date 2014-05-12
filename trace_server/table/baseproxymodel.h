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
#include <QAbstractProxyModel>
#include <vector>
#include <3rd/assocvector.h>

class BaseProxyModel : public QAbstractProxyModel
{
	Q_OBJECT

public:
	explicit BaseProxyModel (QObject * parent);

	virtual QModelIndex index (int row, int column, QModelIndex const & parent = QModelIndex()) const;
	virtual QModelIndex parent (QModelIndex const & child) const { return QModelIndex(); }

	virtual int rowCount (QModelIndex const & parent = QModelIndex()) const;
	virtual int columnCount (QModelIndex const & parent = QModelIndex()) const;

	virtual QModelIndex mapToSource (QModelIndex const & proxyIndex) const;

	QModelIndex mapNearestFromSource (QModelIndex const & sourceIndex) const;
	virtual QModelIndex mapFromSource (QModelIndex const & sourceIndex) const;
	bool rowInProxy (int row) const;
	int rowToSource (int row) const;
	bool colInProxy (int col) const;
	int colToSource (int col) const;
	int colFromSource (int col) const;

	//QVariant headerData (int section, Qt::Orientation orientation, int role) const;
	//bool  setHeaderData (int section, Qt::Orientation orientation, QVariant const & value, int role = Qt::EditRole);

	virtual bool insertRows (int first, int last, QModelIndex const &);
	virtual bool insertColumns (int first, int last, QModelIndex const & parent = QModelIndex());

	void insertAllowedColumn (int src_col);
	void removeAllowedColumn (int src_col);

	QVariant data (QModelIndex const & index, int role) const;
	bool setData (QModelIndex const & index, QVariant const & value, int role);

	Qt::ItemFlags flags (QModelIndex const & index) const;

	virtual bool filterAcceptsColumn (int sourceColumn, QModelIndex const & source_parent) const = 0;
	virtual bool filterAcceptsRow (int sourceRow, QModelIndex const & sourceParent) const = 0;

public slots:
	void force_update();

protected:

	typedef Loki::AssocVector<int, int> map_t; // @TODO: reserve

	void insertCol (int c);
	void insertRow (int c);

	map_t m_cmap_from_src;
	std::vector<int> m_cmap_from_tgt;
	map_t m_map_from_src;
	std::vector<int> m_map_from_tgt;
	std::vector<int> m_allowed_src_cols;
};


