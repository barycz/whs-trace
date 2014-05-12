#include "colorizer_row.h"
//#include <tlv_parser/tlv_encoder.h>
#include "constants.h"
#include "serialize.h"
#include <QPainter>
#include "utils_qstandarditem.h"
#include "utils_color.h"
#include <boost/function.hpp>
#include <qtsln/qtcolorpicker/qtcolorpicker.h>

#include <logs/logwidget.h> // @TODO: fuck off

/*bool ColorizerRow::isMatchedColorizerText (QString str, QColor & fgcolor, QColor & bgcolor) const
{
	for (int i = 0, ie = m_data.size(); i < ie; ++i)
	{
		ColorizedRow const & ct = m_data.at(i);
		if (ct.exactMatch(str))
		{
			fgcolor = ct.m_qcolor;
			bgcolor = ct.m_bgcolor;
			//role = ct.m_role;
			return ct.m_is_enabled;
		}
	}
	return false;
}*/

ColorizerRow::ColorizerRow (QWidget * parent)
	: FilterBase(parent)
	, m_ui(new Ui_ColorizerRow)
	, m_data()
	, m_model(0)
	, m_src_model(0)
{
	initUI();
	setupModel();
}

ColorizerRow::~ColorizerRow ()
{
	destroyModel();
	doneUI();
}

void ColorizerRow::initUI ()
{
	m_ui->setupUi(this);
}

void ColorizerRow::doneUI ()
{
}

bool ColorizerRow::action (DecodedCommand const & cmd)
{
	QString msg;
	if (!cmd.getString(tlv::tag_msg, msg))
		return true;

	for (int i = 0, ie = m_data.size(); i < ie; ++i)
	{
		ColorizedRow const & ct = m_data[i];

		bool const is_match = ct.m_row == cmd.m_src_row;
		if (!is_match)
			continue;
		for (int i = 0, ie = m_src_model->columnCount(); i < ie; ++i)
		{
			QModelIndex const idx = m_src_model->index(cmd.m_src_row, i, QModelIndex());

			//@TODO: cache QMI in DecodedCommand
			m_src_model->setData(idx, ct.m_bgcolor, Qt::BackgroundRole);
			m_src_model->setData(idx, ct.m_fgcolor, Qt::ForegroundRole);

			//@TODO: if column != level
			//@TODO: if column != tid
		}

		/*bool const is_match = cmd.m_src_row == ct.m_row;

		int const col = m_src_model->logWidget().findColumn4TagCst(tlv::tag_msg);
		QModelIndex const idx = m_src_model->index(cmd.m_src_row, col, QModelIndex());
		if (is_match && idx.isValid())
		{
			m_src_model->setData(idx, ct.m_fgcolor, Qt::ForegroundRole);
			m_src_model->setData(idx, ct.m_bgcolor, Qt::BackgroundRole);
			return true;
		}*/
	}

	return false;
}

bool ColorizerRow::accept (DecodedCommand const & cmd) const
{
	return true;
}

void ColorizerRow::defaultConfig ()
{
	m_data.clear();
}

void ColorizerRow::loadConfig (QString const & path)
{
	QString const fname = path + "/" + g_colorizerTag + "/" + typeName();
	if (!::loadConfigTemplate(*this, fname))
		defaultConfig();
}

void ColorizerRow::saveConfig (QString const & path)
{
	QString const fname = path + "/" + g_colorizerTag + "/" + typeName();
	::saveConfigTemplate(*this, fname);
}

void ColorizerRow::setConfigToUI ()
{
	m_model->clear();
	for (int i = 0, ie = m_data.size(); i < ie; ++i)
	{
		QStandardItem * root = m_model->invisibleRootItem();
		QStandardItem * child = findChildByText(root, m_data[i].m_row_str);
		if (child == 0)
		{
			add(m_data[i].m_row_str, m_data[i].m_fgcolor, m_data[i].m_bgcolor);
		}
	}
}

void ColorizerRow::applyConfig ()
{
	FilterBase::applyConfig();
	setConfigToUI();
	recompileColorRows();
}


void ColorizerRow::clear ()
{
	onSelectNone();
	//m_data.clear();
}


///////// colorizer 
void ColorizerRow::setupModel ()
{
	if (!m_model)
	{
		qDebug("new tree view model");
		m_model = new QStandardItemModel;
	}
	m_ui->view->setModel(m_model);
	QStringList l;
	l.append("Enabled");
	l.append("Row");
	l.append("fg");
	l.append("bg");
	m_model->setHorizontalHeaderLabels(l);

	//m_ui->view->model()->setHeaderData(0, Qt::Horizontal, "Enabled");
	//m_ui->view->model()->setHeaderData(1, Qt::Horizontal, "Reg Exp");
	//m_ui->view->model()->setHeaderData(2, Qt::Horizontal, "fg");
	//m_ui->view->model()->setHeaderData(3, Qt::Horizontal, "bg");
	//m_ui->view->setSortingEnabled(true);
	m_ui->view->expandAll();

	m_model->setColumnCount(4);
	m_ui->view->setColumnWidth(0, 192);
	ColorizerRowDelegate * d = new ColorizerRowDelegate(this);
	m_ui->view->setItemDelegate(d);

	m_ui->view->setEditTriggers(QAbstractItemView::NoEditTriggers);
	connect(m_ui->addButton, SIGNAL(clicked()), this, SLOT(onAdd()));
	connect(m_ui->comboBox->lineEdit(), SIGNAL(returnPressed()), this, SLOT(onAdd()));
	connect(m_ui->rmButton, SIGNAL(clicked()), this, SLOT(onRm()));
	connect(m_ui->view, SIGNAL(clicked(QModelIndex)), this, SLOT(onClickedAt(QModelIndex)));
	connect(m_ui->allButton, SIGNAL(clicked()), this, SLOT(onSelectAll()));
	connect(m_ui->noneButton, SIGNAL(clicked()), this, SLOT(onSelectNone()));
	
	m_ui->fgButton->setStandardColors();
	m_ui->fgButton->setCurrentColor(QColor(Qt::black));
	m_ui->bgButton->setStandardColors();
	m_ui->bgButton->setCurrentColor(QColor(Qt::white));
}

void ColorizerRow::destroyModel ()
{
	//if (m_ui->view->itemDelegate())
	//	m_ui->view->setItemDelegate(0);
	if (m_ui->view->model() == m_model)
		m_ui->view->setModel(0);
	delete m_model;
	m_model = 0;
}

ColorizedRow const * ColorizerRow::findMatch (QString const & item) const
{
	for (int i = 0, ie = m_data.size(); i < ie; ++i)
		if (m_data.at(i).m_row_str == item)
		{
			return &m_data.at(i);
		}
	return 0;
}

bool ColorizerRow::isRowColored (int row) const
{
	for (int i = 0, ie = m_data.size(); i < ie; ++i)
	{
		ColorizedRow const & cr = m_data.at(i);
		if (cr.m_row == row && cr.m_is_enabled)
			return true;
	}
	return false;
}

void ColorizerRow::append (QString const & item)
{
	for (int i = 0, ie = m_data.size(); i < ie; ++i)
		if (m_data[i].m_row_str == item)
		{
			ColorizedRow & ct = m_data[i];
			ct.m_is_enabled = true;
			return;
		}
	m_data.push_back(ColorizedRow(item, Qt::blue, Qt::white));
}
void ColorizerRow::remove (QString const & item)
{
	for (int i = 0, ie = m_data.size(); i < ie; ++i)
		if (m_data[i].m_row_str == item)
		{
			ColorizedRow & item = m_data[i];
			item.m_is_enabled = false;
			return;
		}
}
ColorizedRow & ColorizerRow::findOrCreateColorizedRow (QString const & item)
{
	for (int i = 0, ie = m_data.size(); i < ie; ++i)
		if (m_data[i].m_row_str == item)
		{
			ColorizedRow & ct = m_data[i];
			return ct;
		}
	m_data.push_back(ColorizedRow(item, Qt::blue, Qt::white));
	return m_data.back();
}

//////// slots
void ColorizerRow::onSelectAll ()
{
	boost::function<void (ColorizerRow*, QString)> f = &ColorizerRow::append;
	applyFnOnAllChildren(f, this, m_model, Qt::Checked);
	emitFilterChangedSignal();
}

void ColorizerRow::onSelectNone ()
{
	boost::function<void (ColorizerRow*, QString)> f = &ColorizerRow::remove;
	applyFnOnAllChildren(f, this, m_model, Qt::Unchecked);
	emitFilterChangedSignal();
}

void ColorizerRow::onClickedAt (QModelIndex idx)
{
	QStandardItem * item = m_model->itemFromIndex(idx);
	Q_ASSERT(item);

	QString const & str = m_model->data(idx, Qt::DisplayRole).toString();
	bool const checked = (item->checkState() == Qt::Checked);
	ColorizedRow & ct = findOrCreateColorizedRow(item->text());
	if (checked)
	{
		append(str);
		recompileColorRow(ct);
		updateColorRow(ct);
	}
	else
	{
		recompileColorRow(ct);
		updateColorRow(ct);
		remove(str);
	}

	emitFilterChangedSignal();

/*
		if (!idx.isValid()) return;
		QStandardItemModel * model = static_cast<QStandardItemModel *>(m_config_ui.ui()->viewColorRow->model());
		QStandardItem * item = model->itemFromIndex(idx);
		Q_ASSERT(item);

		QString const & val = model->data(idx, Qt::DisplayRole).toString();
		bool const checked = (item->checkState() == Qt::Checked);

		// @TODO: if state really changed
		for (int i = 0, ie = m_filter_state.m_colorized_texts.size(); i < ie; ++i)
		{
			ColorizedRow & ct = m_filter_state.m_colorized_texts[i];
			if (ct.m_row_str == val)
			{
				if (checked)
				{
					m_filter_state.setColorRowChecked(val, checked);
					recompileColorRow(ct);
					updateColorRow(ct);
				}
				else
				{
					uncolorRow(ct);
					m_filter_state.setColorRowChecked(val, checked);
					recompileColorRow(ct);
				}
				break;
			}
		}
*/
}


void ColorizerRow::recompile ()
{ }


/*void ColorizerRow::locateItem (QString const & item, bool scrollto, bool expand)
{
	QModelIndexList indexList = m_model->match(m_model->index(0, 0), Qt::DisplayRole, item);
	if (!indexList.empty())
	{
		QModelIndex const selectedIndex(indexList.first());
		getWidget()->setCurrentIndex(selectedIndex);
	}
}*/

	//@TODO: dedup
	void ColorizerRow::actionColorRow (DecodedCommand const & cmd, ColorizedRow const & ct) const
	{
		bool const is_match = ct.m_row == cmd.m_src_row;
		if (!is_match)
			return;

		for (int i = 0, ie = m_src_model->columnCount(); i < ie; ++i)
		{
			QModelIndex const idx = m_src_model->index(cmd.m_src_row, i, QModelIndex());

			//@TODO: cache QMI in DecodedCommand
			m_src_model->setData(idx, ct.m_bgcolor, Qt::BackgroundRole);
			m_src_model->setData(idx, ct.m_fgcolor, Qt::ForegroundRole);

			//@TODO: if column != level
			//@TODO: if column != tid
		}
	}
	void ColorizerRow::actionUncolorRow (DecodedCommand const & cmd, ColorizedRow const & ct) const
	{
		bool const is_match = ct.m_row == cmd.m_src_row;
		if (!is_match)
			return;

		for (int i = 0, ie = m_src_model->columnCount(); i < ie; ++i)
		{
			QModelIndex const idx = m_src_model->index(cmd.m_src_row, i, QModelIndex());

			//@TODO: cache QMI in DecodedCommand
			m_src_model->setData(idx, QColor(Qt::white), Qt::BackgroundRole);
			m_src_model->setData(idx, QColor(Qt::black), Qt::ForegroundRole);
			//@TODO: if column != level
			//@TODO: if column != tid
		}
		/*for (size_t i = 0, ie = m_src_model->columnCount(); i < ie; ++i)
		{
			bool const is_match = ct.accept(val);
			//@TODO: cache QMI in DecodedCommand?
			int const col = m_src_model->logWidget().findColumn4TagCst(cmd.m_tvs[i].m_tag);
			QModelIndex const idx = m_src_model->index(cmd.m_src_row, col, QModelIndex());
			if (is_match && idx.isValid())
			{
				m_src_model->setData(idx, QColor(Qt::white), Qt::BackgroundRole);
				m_src_model->setData(idx, QColor(Qt::black), Qt::ForegroundRole);
			}
			//@TODO: if column != level
			//@TODO: if column != tid
		}*/
	}

	void ColorizerRow::updateColorRow (ColorizedRow const & ct)
	{
		for (size_t r = 0, re = m_src_model->dcmds().size(); r < re; ++r)
		{
			DecodedCommand const & dcmd = m_src_model->dcmds()[r];
			actionColorRow(dcmd, ct);
		}
	}

	void ColorizerRow::uncolorRow (ColorizedRow const & ct)
	{
		for (size_t r = 0, re = m_src_model->dcmds().size(); r < re; ++r)
		{
			DecodedCommand const & dcmd = m_src_model->dcmds()[r];
			actionUncolorRow(dcmd, ct);
		}
	}

	void ColorizerRow::onActivate (int)
	{
	}
	void ColorizerRow::onFgChanged () { onColorRowChanged(Qt::ForegroundRole); }
	void ColorizerRow::onBgChanged () { onColorRowChanged(Qt::BackgroundRole); }
	void ColorizerRow::onColorRowChanged (int role)
	{
		for (int i = 0, ie = m_data.size(); i < ie; ++i)
		{
			ColorizedRow & ct = m_data[i];
			QStandardItem * const root = m_model->invisibleRootItem();
			QString const qrow = ct.m_row_str;
			QStandardItem * const child = findChildByText(root, qrow);

			if (!child)
				continue;

			QModelIndex const fgidx = m_model->index(child->row(), 1);
			if (fgidx.isValid())
			{
				if (QtColorPicker * w = static_cast<QtColorPicker *>(m_ui->view->indexWidget(fgidx)))
					ct.m_fgcolor = w->currentColor();
			}
			QModelIndex const bgidx = m_model->index(child->row(), 2);
			if (bgidx.isValid())
			{
				if (QtColorPicker * w = static_cast<QtColorPicker *>(m_ui->view->indexWidget(bgidx)))
					ct.m_bgcolor = w->currentColor();
			}

			//TODO: this updates all of them, fixit
			updateColorRow(ct);
		}
	}

	void ColorizerRow::recompileColorRow (ColorizedRow & ct)
	{
		ct.m_is_enabled = true;
	}

	void ColorizerRow::recompileColorRows ()
	{
		for (int i = 0, ie = m_data.size(); i < ie; ++i)
		{
			ColorizedRow & ct = m_data[i];
			recompileColorRow(ct);
			//updateColorRow(ct);
		}
	}
	//void ColorizerRow::onDoubleClickedAtColorRowList (QModelIndex idx) { }

	ColorizedRow & ColorizerRow::add (QString const & row, QColor const & fg, QColor const & bg)
	{
		QStandardItem * root = m_model->invisibleRootItem();
		QStandardItem * child = findChildByText(root, row);
		if (child == 0)
		{
			QList<QStandardItem *> row_items = addRow(row, true);
			root->appendRow(row_items);
			child = findChildByText(root, row);

			QStandardItem * fgitem = new QStandardItem("fg");
			QStandardItem * bgitem = new QStandardItem("bg");
			QStandardItem * stitem = new QStandardItem("status");
			stitem->setCheckable(false);
			m_model->setItem(child->row(), 1, fgitem);
			m_model->setItem(child->row(), 2, bgitem);
			m_model->setItem(child->row(), 3, stitem);
			append(row);

			ColorizedRow & ct = findOrCreateColorizedRow(row);
			ct.m_fgcolor = fg;
			ct.m_bgcolor = bg;
			{
				QtColorPicker * w = mkColorPicker(m_ui->view, "fg", ct.m_fgcolor);
				connect(w, SIGNAL(colorChanged(const QColor &)), this, SLOT(onFgChanged()));
				QModelIndex const idx = m_model->indexFromItem(fgitem);
				m_ui->view->setIndexWidget(idx, w);
			}
			{
				QtColorPicker * w = mkColorPicker(m_ui->view, "bg", ct.m_bgcolor);
				connect(w, SIGNAL(colorChanged(const QColor &)), this, SLOT(onBgChanged()));
				QModelIndex const idx = m_model->indexFromItem(bgitem);
				m_ui->view->setIndexWidget(idx, w);
			}
			return ct;
		}
		else
		{
			ColorizedRow & ct = findOrCreateColorizedRow(row);
			return ct;
		}
	}

	void ColorizerRow::colorize (QString const & row, QColor const & fg, QColor const & bg)
	{
		ColorizedRow & ct = add(row, fg, bg);

		recompileColorRow(ct);
		updateColorRow(ct);
	}

	void ColorizerRow::onAdd ()
	{
		QString const qItem = m_ui->comboBox->currentText();
		if (!qItem.length())
			return;
		QColor const qFg = m_ui->fgButton->currentColor();
		QColor const qBg = m_ui->fgButton->currentColor();
		colorize(qItem, qFg, qBg);
	}

	void ColorizerRow::uncolorize (QString const & row)
	{
		QStandardItem * const root = m_model->invisibleRootItem();
		QStandardItem * const child = findChildByText(root, row);
		if (child)
		{
			//QModelIndex const idx = m_model->index(child->row(), 1);
			m_model->removeRow(child->row());
		}

		ColorizedRow & ct = findOrCreateColorizedRow(row);
		recompileColorRow(ct);
		uncolorRow(ct);
		remove(row);
	}

	void ColorizerRow::onRm ()
	{
		QModelIndex const idx = m_ui->view->currentIndex();
		QStandardItem * item = m_model->itemFromIndex(idx);
		if (!item)
			return;

		QString const & val = m_model->data(idx, Qt::DisplayRole).toString();
		uncolorize(val);
	}



//////// delegate
ColorizerRowDelegate::~ColorizerRowDelegate ()
{
	qDebug("%s", __FUNCTION__);
}
void ColorizerRowDelegate::paint (QPainter * painter, QStyleOptionViewItem const & option, QModelIndex const & index) const
{
	painter->save();
	QStyleOptionViewItemV4 option4 = option;
	initStyleOption(&option4, index);

	/*if (m_app_data && m_app_data->getDictCtx().m_names.size())
	{
		QVariant const value = index.data(Qt::DisplayRole);
		if (value.isValid() && !value.isNull())
		{
			Dict const & dict = m_app_data->getDictCtx();

			option4.text = dict.findNameFor(value.toString());
			QWidget const * widget = option4.widget;
			if (widget)
			{
				QStyle * style = widget->style();
				style->drawControl(QStyle::CE_ItemViewItem, &option4, painter, widget);
			}
		}
	}
	else*/
		QStyledItemDelegate::paint(painter, option4, index);
	painter->restore();
}

