#include "colorizer_string.h"
//#include <tlv_parser/tlv_encoder.h>
#include "constants.h"
#include "serialize.h"
#include <QPainter>
#include "utils_qstandarditem.h"
#include "utils_color.h"
#include <boost/function.hpp>
#include <qtsln/qtcolorpicker/qtcolorpicker.h>

#include <logs/logwidget.h> // @TODO: fuck off

ColorizerString::ColorizerString (QWidget * parent)
	: FilterBase(parent)
	, m_ui(new Ui_ColorizerString)
	, m_data()
	, m_model(0)
	, m_src_model(0)
{
	initUI();
	setupModel();
}

ColorizerString::~ColorizerString ()
{
	destroyModel();
	doneUI();
}

void ColorizerString::initUI ()
{
	m_ui->setupUi(this);
}

void ColorizerString::doneUI ()
{
}

void ColorizerString::actionColor (DecodedCommand const & cmd, ColorizedString const & ct, QColor const & fg, QColor const & bg) const
{
	for (size_t i = 0, ie = cmd.m_tvs.size(); i < ie; ++i)
	{
		QString const & val = cmd.m_tvs[i].m_val;

		if (bool const is_match = ct.accept(val))
		{
			int const col = m_src_model->storage2Column(i);
			//@TODO: if column != level
			//@TODO: if column != tid
			QModelIndex const idx = m_src_model->index(cmd.m_src_row, col, QModelIndex());
			if (ct.m_is_enabled && is_match && idx.isValid())
			{
				m_src_model->setData(idx, bg, Qt::BackgroundRole);
				m_src_model->setData(idx, fg, Qt::ForegroundRole);
			}
		}
	}
}
void ColorizerString::actionUncolor (DecodedCommand const & cmd, ColorizedString const & ct) const
{
	actionColor(cmd, ct, QColor(Qt::black), QColor(Qt::white));
}

bool ColorizerString::action (DecodedCommand const & cmd)
{
	for (size_t i = 0, ie = m_data.size(); i < ie; ++i)
	{
		ColorizedString const & ct = m_data[i];
		actionColor(cmd, ct, ct.m_fgcolor, ct.m_bgcolor);
	}
	return false;
}

bool ColorizerString::accept (DecodedCommand const & cmd) const
{
	return true;
}

void ColorizerString::defaultConfig ()
{
	m_data.clear();
	/*m_data.push_back(ColorizedString(".*[Ww]arning.*", QColor(Qt::black), QColor(Qt::yellow)));
	m_data.push_back(ColorizedString(".*[Ee]rror.*", QColor(Qt::black), QColor(Qt::red)));
	for (int i = 0, ie = m_data.size(); i < ie; ++i)
	{
		ColorizedString & ct = m_data[i];
		ct.m_is_enabled = 1;
	}*/
}

void ColorizerString::loadConfig (QString const & path)
{
	QString const fname = path + "/" + g_colorizerTag + "/" + typeName();
	if (!::loadConfigTemplate(*this, fname))
		defaultConfig();
}

void ColorizerString::saveConfig (QString const & path)
{
	QString const fname = path + "/" + g_colorizerTag + "/" + typeName();
	::saveConfigTemplate(*this, fname);
}

void ColorizerString::setConfigToUI ()
{
	m_model->clear();
	for (size_t i = 0, ie = m_data.size(); i < ie; ++i)
	{
		QStandardItem * root = m_model->invisibleRootItem();
		QStandardItem * child = findChildByText(root, m_data[i].m_str);
		if (child == 0)
		{
			add(m_data[i].m_str, m_data[i].m_fgcolor, m_data[i].m_bgcolor);
		}
	}
}

void ColorizerString::applyConfig ()
{
	FilterBase::applyConfig();
	setConfigToUI();
}


void ColorizerString::clear ()
{
	onSelectNone();
	//m_data.clear();
}


///////// colorizer
void ColorizerString::setupModel ()
{
	if (!m_model)
	{
		qDebug("new tree view model");
		m_model = new QStandardItemModel;
	}
	m_ui->view->setModel(m_model);
	QStringList l;
	l.append("Enabled");
	l.append("String");
	l.append("fg");
	l.append("bg");
	m_model->setHorizontalHeaderLabels(l);

	m_ui->view->expandAll();

	m_model->setColumnCount(4);
	m_ui->view->setColumnWidth(0, 192);
	ColorizerStringDelegate * d = new ColorizerStringDelegate(this);
	m_ui->view->setItemDelegate(d);

	m_ui->view->setEditTriggers(QAbstractItemView::NoEditTriggers);
	connect(m_ui->addButton, SIGNAL(clicked()), this, SLOT(onAdd()));
	connect(m_ui->comboBox->lineEdit(), SIGNAL(returnPressed()), this, SLOT(onAdd()));
	connect(m_ui->rmButton, SIGNAL(clicked()), this, SLOT(onRm()));
	connect(m_ui->view, SIGNAL(clicked(QModelIndex)), this, SLOT(onClickedAt(QModelIndex)));
	connect(m_ui->allButton, SIGNAL(clicked()), this, SLOT(onSelectAll()));
	connect(m_ui->noneButton, SIGNAL(clicked()), this, SLOT(onSelectNone()));

	m_ui->fgButton->setStandardColors();
	m_ui->fgButton->setCurrentColor(QColor(Qt::blue));
	m_ui->bgButton->setStandardColors();
	m_ui->bgButton->setCurrentColor(QColor(Qt::white));
}

void ColorizerString::destroyModel ()
{
	//if (m_ui->view->itemDelegate())
	//	m_ui->view->setItemDelegate(0);
	if (m_ui->view->model() == m_model)
		m_ui->view->setModel(0);
	delete m_model;
	m_model = 0;
}

ColorizedString const * ColorizerString::findMatch (QString const & item) const
{
	for (size_t i = 0, ie = m_data.size(); i < ie; ++i)
		if (m_data[i].m_str == item)
		{
			return &m_data.at(i);
		}
	return 0;
}

void ColorizerString::append (QString const & item)
{
	for (size_t i = 0, ie = m_data.size(); i < ie; ++i)
		if (m_data[i].m_str == item)
		{
			ColorizedString & ct = m_data[i];
			ct.m_is_enabled = true;
			return;
		}
	m_data.push_back(ColorizedString(item, Qt::blue, Qt::white));
}
void ColorizerString::remove (QString const & item)
{
	for (size_t i = 0, ie = m_data.size(); i < ie; ++i)
		if (m_data[i].m_str == item)
		{
			ColorizedString & item = m_data[i];
			item.m_is_enabled = false;
			return;
		}
}
ColorizedString & ColorizerString::findOrCreateColorizedString (QString const & item)
{
	for (size_t i = 0, ie = m_data.size(); i < ie; ++i)
		if (m_data[i].m_str == item)
		{
			ColorizedString & ct = m_data[i];
			return ct;
		}
	m_data.push_back(ColorizedString(item, Qt::blue, Qt::white));
	return m_data.back();
}

//////// slots
void ColorizerString::onSelectAll ()
{
	boost::function<void (ColorizerString*, QString)> f = &ColorizerString::append;
	applyFnOnAllChildren(f, this, m_model, Qt::Checked);
	emitFilterChangedSignal();
}

void ColorizerString::onSelectNone ()
{
	boost::function<void (ColorizerString*, QString)> f = &ColorizerString::remove;
	applyFnOnAllChildren(f, this, m_model, Qt::Unchecked);
	emitFilterChangedSignal();
}

void ColorizerString::onClickedAt (QModelIndex idx)
{
	QStandardItem * item = m_model->itemFromIndex(idx);
	Q_ASSERT(item);

	QString const & str = m_model->data(idx, Qt::DisplayRole).toString();
	bool const checked = (item->checkState() == Qt::Checked);
	ColorizedString & ct = findOrCreateColorizedString(item->text());
	if (checked)
	{
		append(str);
		updateColor(ct);
	}
	else
	{
		updateColor(ct);
		remove(str);
	}

	emitFilterChangedSignal();
}


void ColorizerString::recompile ()
{ }


/*void ColorizerString::locateItem (QString const & item, bool scrollto, bool expand)
{
	QModelIndexList indexList = m_model->match(m_model->index(0, 0), Qt::DisplayRole, item);
	if (!indexList.empty())
	{
		QModelIndex const selectedIndex(indexList.first());
		getWidget()->setCurrentIndex(selectedIndex);
	}
}*/


void ColorizerString::updateColor (ColorizedString const & ct)
{
	for (size_t r = 0, re = m_src_model->dcmds().size(); r < re; ++r)
	{
		DecodedCommand const & dcmd = m_src_model->dcmds()[r];
		actionColor(dcmd, ct, ct.m_fgcolor, ct.m_bgcolor);
	}
}

void ColorizerString::uncolor (ColorizedString const & ct)
{
	for (size_t r = 0, re = m_src_model->dcmds().size(); r < re; ++r)
	{
		DecodedCommand const & dcmd = m_src_model->dcmds()[r];
		actionUncolor(dcmd, ct);
	}
}

void ColorizerString::onActivate (int)
{ }
void ColorizerString::onFgChanged () { onColorButtonChanged(Qt::ForegroundRole); }
void ColorizerString::onBgChanged () { onColorButtonChanged(Qt::BackgroundRole); }
void ColorizerString::onColorButtonChanged (int role)
{
	for (size_t i = 0, ie = m_data.size(); i < ie; ++i)
	{
		ColorizedString & ct = m_data[i];
		QStandardItem * const root = m_model->invisibleRootItem();
		QStandardItem * const child = findChildByText(root, ct.m_str);

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
		updateColor(ct);
	}
}

//void ColorizerString::onDoubleClickedAtColorStringList (QModelIndex idx) { }

ColorizedString & ColorizerString::add (QString const & str, QColor const & fg, QColor const & bg)
{
	QStandardItem * root = m_model->invisibleRootItem();
	QStandardItem * child = findChildByText(root, str);
	if (child == 0)
	{
		QList<QStandardItem *> row_items = addRow(str, true);
		root->appendRow(row_items);
		child = findChildByText(root, str);

		QStandardItem * fgitem = new QStandardItem("fg");
		QStandardItem * bgitem = new QStandardItem("bg");
		QStandardItem * stitem = new QStandardItem("status");
		stitem->setCheckable(false);
		m_model->setItem(child->row(), 1, fgitem);
		m_model->setItem(child->row(), 2, bgitem);
		m_model->setItem(child->row(), 3, stitem);
		append(str);

		ColorizedString & ct = findOrCreateColorizedString(str);
		ct.m_fgcolor = fg;
		ct.m_bgcolor = bg;
		ct.m_is_enabled = true;
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
		ColorizedString & ct = findOrCreateColorizedString(str);
		return ct;
	}
}

void ColorizerString::onAdd ()
{
	QString const qItem = m_ui->comboBox->currentText();
	if (!qItem.length())
		return;
	QColor const qFg = m_ui->fgButton->currentColor();
	QColor const qBg = m_ui->bgButton->currentColor();
	ColorizedString & ct = add(qItem, qFg, qBg);

	updateColor(ct);
}

void ColorizerString::onRm ()
{
	QModelIndex const idx = m_ui->view->currentIndex();
	QStandardItem * item = m_model->itemFromIndex(idx);
	if (!item)
		return;
	QString const & val = m_model->data(idx, Qt::DisplayRole).toString();
	m_model->removeRow(idx.row());

	ColorizedString & ct = findOrCreateColorizedString(val);
	uncolor(ct);
	remove(val);
}



//////// delegate
ColorizerStringDelegate::~ColorizerStringDelegate ()
{
	qDebug("%s", __FUNCTION__);
}
void ColorizerStringDelegate::paint (QPainter * painter, QStyleOptionViewItem const & option, QModelIndex const & index) const
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






