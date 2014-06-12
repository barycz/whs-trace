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
#include <QStandardItem>
#include <QString>
#include <QDir>
#include <QFile>
#include <QSettings>
#include "constants.h"
#include "types.h"

inline QStandardItem * findChildByText (QStandardItem * parent, QString const & txt)
{
	for (int i = 0, ie = parent->rowCount(); i < ie; ++i)
	{
		if (parent->child(i)->text() == txt)
			return parent->child(i);
	}
	return 0;
}

inline QList<QStandardItem *> listChildren (QStandardItem * item)
{
	QStandardItemModel * model = item->model();
	QList<QStandardItem *> children;
	QModelIndex const idx = item->index();
	//if (!idx.isValid()) @FIXME: BUT not root
	//	return children;

	if (item->rowCount() == 0)
	{
		return children;
	}

	for (int r = 0; r < item->rowCount(); ++r)
	{
		QModelIndex const si = model->index(r, 0, idx);
		QStandardItem * child = model->itemFromIndex(si);
		if (child)
			children.push_back(child);
	}
	return children;
}

inline QList<QStandardItem *> addRow (QString const & str, bool checked )
{
	QList<QStandardItem *> row_items;
	QStandardItem * const name_item = new QStandardItem(str);
	name_item->setCheckable(true);
	name_item->setCheckState(checked ? Qt::Checked : Qt::Unchecked);
	row_items << name_item;
	return row_items;
}

inline QList<QStandardItem *> addUncheckableRow (QString const & str)
{
	QList<QStandardItem *> row_items;
	QStandardItem * const name_item = new QStandardItem(str);
	name_item->setCheckable(false);
	row_items << name_item;
	return row_items;
}

inline QList<QStandardItem *> addTriRow (QString const & str, Qt::CheckState const state)
{
	QList<QStandardItem *> row_items;
	QStandardItem * const name_item = new QStandardItem(str);
	name_item->setCheckable(true);
	name_item->setEditable(false);
	name_item->setCheckState(state);
	row_items << name_item;
	return row_items;
}


/*inline QList<QStandardItem *> addTriRow (QString const & str, Qt::CheckState const checked, char const mode)
{
	QList<QStandardItem *> row_items;

	QStandardItem * const name_item = new QStandardItem(str);
	name_item->setCheckable(true);
	name_item->setEditable(false);
	row_items << name_item;
	QStandardItem * const mode_item = new QStandardItem(QString(mode));
	mode_item->setCheckable(false);
	mode_item->setEditable(false);
	row_items.append(mode_item);
	return row_items;
}*/

inline QList<QStandardItem *> addTriRow (Qt::CheckState const check0, QString const & str0, QString const & str1)
{
    QList<QStandardItem *> row_items;

    QStandardItem * const item0 = new QStandardItem(str0);
    item0->setCheckable(true);
    item0->setEditable(false);
		item0->setCheckState(check0);
    row_items << item0;
		
    QStandardItem * const item1 = new QStandardItem(str1);
    row_items.append(item1);
    item1->setCheckable(false);
		
    return row_items;
}

inline QList<QStandardItem *> addTriRow (Qt::CheckState const check0, QString const & str0, QString const & str1, QString const & str2)
{
    QList<QStandardItem *> row_items;

    QStandardItem * const item0 = new QStandardItem(str0);
    item0->setCheckable(true);
    item0->setEditable(false);
    row_items << item0;

    QStandardItem * const item1 = new QStandardItem(str1);
    row_items.append(item1);
    item1->setCheckable(false);

    QStandardItem * const item2 = new QStandardItem(str2);
    row_items.append(item2);
    item2->setCheckable(false);
    return row_items;
}

inline QList<QStandardItem *> add4Col (Qt::CheckState const check0, QString const & str0, QString const & str1, QString const & str2, QString const & str3)
{
    QList<QStandardItem *> row_items;

    QStandardItem * const item0 = new QStandardItem(str0);
    item0->setCheckable(true);
    item0->setEditable(false);
    row_items << item0;

    QStandardItem * const item1 = new QStandardItem(str1);
    row_items.append(item1);
    item1->setCheckable(false);

    QStandardItem * const item2 = new QStandardItem(str2);
    row_items.append(item2);
    item2->setCheckable(false);

    QStandardItem * const item3 = new QStandardItem(str3);
    row_items.append(item3);
    item3->setCheckable(false);
    return row_items;
}


inline QList<QStandardItem *> addTriRow (QString const & str, Qt::CheckState const checked, bool const inclusive)
{
	QString mode("E");
	if (inclusive)
		mode = "I";
    return addTriRow(checked, str, mode);
}

template <typename T, typename U>
void applyFnOnAllChildren (T fn, U instance, QAbstractItemModel * abs_model, Qt::CheckState state)
{
	QStandardItemModel * model = static_cast<QStandardItemModel *>(abs_model);
	QStandardItem * root = model->invisibleRootItem();
	QList<QStandardItem *> l = listChildren(root);

	for (int i = 0, ie = l.size(); i < ie; ++i)
	{
		l.at(i)->setCheckState(state);
		QString const & data = model->data(l.at(i)->index(), Qt::DisplayRole).toString();
		fn(instance, data);
	}
}
	 
inline QList<QStandardItem *> addRowTriState (QString const & str, E_NodeStates const state)
{
	QList<QStandardItem *> row_items;
	QStandardItem * const name_item = new QStandardItem(str);
	name_item->setCheckable(true);
	name_item->setTristate(true);	
	name_item->setCheckState(static_cast<Qt::CheckState>(state));
	row_items << name_item;
	return row_items;
}

/*inline void flipCheckState (QStandardItem * item)
{
	bool const checked = (item->checkState() == Qt::Checked);
	item->setCheckState(checked ? Qt::Unchecked : Qt::Checked);
}

inline void flipCheckStateChilds (QStandardItem * node)
{
	flipCheckState(node);
	int const rc = node->rowCount();
	for (int r = 0; r < rc; ++r)
	{
		QStandardItem * const child = node->child(r, 0);
		flipCheckStateChilds(child);
	}
}*/

inline void setCheckState (QStandardItem * const node, Qt::CheckState const state)
{
	if (node)
		node->setCheckState(state);
}

inline bool checkChildState (QStandardItem * const node, Qt::CheckState const state)
{
	bool result = true;
	if (node->rowCount() == 0)
		result &= (node->checkState() == state);
	else
		for (int r = 0, re = node->rowCount(); r < re; ++r)
		{
			QStandardItem * const child = node->child(r, 0);
			result &= checkChildState(child, state);
		}
	return result;
}

inline void setCheckStateChilds (QStandardItem * const node, Qt::CheckState const state)
{
	setCheckState(node, state);
	int const rc = node->rowCount();
	//qDebug("node=%s, state=%u", node->text().toStdString().c_str(), state);
	for (int r = 0; r < rc; ++r)
	{
		QStandardItem * const child = node->child(r, 0);
		setCheckStateChilds(child, state);
	}
}

inline void setCheckStateReverse (QStandardItem * n, Qt::CheckState const state)
{
	while (n)
	{
		n->setCheckState(state);
		n = n->parent();
	}
}

inline void collectPath (QStandardItem const * n, std::vector<QString> & tokens)
{
	while (n)
	{
		tokens.push_back(n->text());
		n = n->parent();
	}
}

/*inline void syncCheckBoxesWithFileFilters (QStandardItem const * node, E_FilterMode const fmode, tree_filter & ff, strings_t & stack)
{
	stack.push_back(node->text().toStdString());
	ff.exclude_to_state(stack, static_cast<E_NodeStates>(node->checkState()));
	bool const checked = (node->checkState() == Qt::Checked);
	//if (fmode == e_Include)
	//	ff.exclude_to_state(stack, !checked);
	//else
	//	ff.exclude_to_state(stack, checked);

	bool const descend = ((fmode == e_Include && checked) || (fmode == e_Exclude && !checked));
	if (descend)
	{
		int const rc = node->rowCount();
		for (int r = 0; r < rc; ++r)
		{
			QStandardItem const * const child = node->child(r, 0);
			syncCheckBoxesWithFileFilters(child, fmode, ff, stack);
		}
	}
	stack.pop_back();
}

inline void syncCheckBoxesWithFileFilters (QStandardItem const * node, E_FilterMode const fmode, tree_filter & ff)
{
	strings_t stack;
	stack.reserve(256);
	syncCheckBoxesWithFileFilters(node, fmode, ff, stack);
}
*/

