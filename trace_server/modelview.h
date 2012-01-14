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
#ifndef MODELVIEW_H
#define MODELVIEW_H

#include <QAbstractTableModel>
#include <QString>
#include <vector>
#include "../tlv_parser/tlv_parser.h"

class Connection;

class ModelView : public QAbstractTableModel
{
	Q_OBJECT
public:
	explicit ModelView (QObject * parent = 0, Connection * c = 0);
	int rowCount (const QModelIndex & parent = QModelIndex()) const;
	int columnCount (const QModelIndex & parent = QModelIndex()) const;
	QVariant data (const QModelIndex & index, int role = Qt::DisplayRole) const;
	QVariant headerData (int section, Qt::Orientation orientation, int role) const;

	void transactionStart ();
	void appendCommand (tlv::StringCommand const & cmd, bool & excluded);
	void transactionCommit ();

	bool checkExistence (QModelIndex const & index) const;
	bool checkColumnExistence (tlv::tag_t tag, QModelIndex const & index) const;

signals:
	
public slots:

private:

	Connection * m_connection;
	typedef std::vector<QString> columns_t;
	typedef std::vector<columns_t> rows_t;
	rows_t m_rows;
};

#endif // MODELVIEW_H