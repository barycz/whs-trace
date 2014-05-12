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
#include <basetablemodel.h>
#include <QAbstractProxyModel>
#include <QString>
#include <QColor>
#include <vector>
#include <tlv_parser/tlv_parser.h>

class TableModel : public BaseTableModel
{
	//Q_OBJECT
public:
	explicit TableModel (QObject * parent, std::vector<QString> & hhdr, std::vector<int> & hsize);
	~TableModel ();

protected:

	virtual void parseCommand (DecodedCommand const & cmd, E_ReceiveMode mode, BatchCmd & batch);
	virtual void commitBatchToModel (BatchCmd & batch);
	void postProcessBatch (int src_from, int src_to, BatchCmd const & batch);

	bool handleTableXYCommand (DecodedCommand const & cmd, E_ReceiveMode mode);
	bool handleTableSetupCommand (DecodedCommand const & cmd, E_ReceiveMode mode);
	bool handleTableClearCommand (DecodedCommand const & cmd, E_ReceiveMode mode);

	void parseTableXY (int x, int y, QString const &, QString const & fgc, QString const & bgc, QString const & msg_tag, BatchCmd & batch);
	void parseTableSetup (int x, int y, QString const & time, QString const & fgc, QString const & bgc, QString const & hhdr, QString const & tag, BatchCmd & batch);
};

