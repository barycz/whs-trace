#include "filter_fileline.h"
#include "constants.h"
#include "serialize.h"
// serialization stuff
#include <boost/serialization/type_info_implementation.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/serialization/serialization.hpp>
#include <boost/serialization/vector.hpp>
#include <serialize/ser_qt.h>
#include <fstream>

FilterFileLine::FilterFileLine (QWidget * parent)
	: FilterBase(parent)
	, m_ui(new Ui_FilterFileLine)
	, m_data()
	, m_model(0)
	, m_proxy(0)
	, m_proxy_selection(0)
{
	initUI();
	setupModel();
}

FilterFileLine::~FilterFileLine ()
{
	destroyModel();
	doneUI();
}

void FilterFileLine::initUI ()
{
	m_ui->setupUi(this);

	connect(m_ui->cutParentSpinBox, SIGNAL(valueChanged(int)), this, SLOT(onCutParentValueChanged(int)));
	connect(m_ui->collapseChildsButton, SIGNAL(clicked()), this, SLOT(onCollapseChilds()));
	getWidget()->setEditTriggers(QAbstractItemView::NoEditTriggers);
	connect(getWidget(), SIGNAL(clicked(QModelIndex)), this, SLOT(onClickedAtFileTree(QModelIndex)));
	//connect(getWidget(), SIGNAL(doubleClicked(QModelIndex)), this, SLOT(onDoubleClickedAtFileTree(QModelIndex)));
	getWidget()->header()->hide();

	connect(m_ui->filterFileComboBox, SIGNAL(editTextChanged(QString)), this, SLOT(onFilterFileComboChanged(QString)));
	bool const cancel_on = !m_ui->filterFileComboBox->currentText().isEmpty();
	m_ui->cancelFilterButton->setEnabled(cancel_on);
	connect(m_ui->cancelFilterButton, SIGNAL(clicked()), this, SLOT(onCancelFilterFileButton()));
	connect(m_ui->allButton, SIGNAL(clicked()), this, SLOT(onSelectAll()));
	connect(m_ui->noneButton, SIGNAL(clicked()), this, SLOT(onSelectNone()));
}

void FilterFileLine::doneUI ()
{
}

bool FilterFileLine::accept (DecodedCommand const & cmd) const
{
	QString file, line;
	if (!cmd.getString(tlv::tag_file, file))
		return true;
	if (!cmd.getString(tlv::tag_line, line))
		return true;

	bool excluded = false;
	if (!file.isNull() && !line.isNull() && !file.isEmpty() && !line.isEmpty())
	{
		TreeModelItem ff;
		bool const ff_present = isFileLinePresent(std::make_pair(file, line), ff);
		if (ff_present)
		{
			excluded |= ff.m_state == e_Unchecked;
		}
	}
	return !excluded;
}

void FilterFileLine::defaultConfig ()
{
}

void FilterFileLine::loadConfig (QString const & path)
{
	QString const fname = path + "/" + g_filterTag + "/" + typeName();
	FilterFileLine ffl;
	if (!::loadConfigTemplate(ffl, fname))
		defaultConfig();
	enable(ffl.enabled());
	mergeWithConfig(ffl.m_data);
}

void FilterFileLine::saveConfig (QString const & path)
{
	QString const fname = path + "/" + g_filterTag + "/" + typeName();
	::saveConfigTemplate(*this, fname);
}

void FilterFileLine::applyConfig ()
{
	FilterBase::applyConfig();
	destroyModel();
	setupModel();
}

FilterTreeModel::FilterTreeModel (QObject * parent, tree_data_t * data)
	: TreeModel<TreeModelItem>(parent, data)
{
	qDebug("%s", __FUNCTION__);
}

QModelIndex FilterTreeModel::insertItemWithPath (QStringList const & path, bool checked)
{
	QString const name = path.join("/");
	return insertItemWithHint(name, checked);
}



///////// file filters
void FilterFileLine::setupModel ()
{
	if (!m_model)
	{
		qDebug("new tree view file model");
		m_model = new FilterTreeModel(this, &m_data);

		  //->setFilterBehavior( KSelectionProxyModel::ExactSelection );
		m_proxy_selection = new QItemSelectionModel(m_model, this);
		m_proxy = new TreeProxyModel(m_model, m_proxy_selection);
	}
	getWidget()->setModel(m_model);
	m_model->syncExpandState(getWidget());
	getWidget()->hideLinearParents();
}

void FilterFileLine::destroyModel ()
{
	if (m_model)
	{
		qDebug("destroying file model");
		//disconnect(m_file_model, SIGNAL(invalidateFilter()), this, SLOT(onInvalidateFilter()));
		getWidget()->unsetModel(m_model);
		delete m_model;
		m_model = 0;
		delete m_proxy;
		m_proxy = 0;
		delete m_proxy_selection;
		m_proxy_selection = 0;
	}
}

void FilterFileLine::locateItem (QString const & item, bool scrollto, bool expand)
{
	fileModel()->selectItem(getWidget(), item, scrollto);
	if (expand)
		fileModel()->expandItem(getWidget(), item);
}

bool FilterFileLine::isFileLinePresent (fileline_t const & item, TreeModelItem & fi) const
{
	TreeModelItem const * tmp_fi = 0;
	bool const exists = m_data.is_present(item.first + "/" + item.second, tmp_fi);
	if (exists)
		fi = *tmp_fi;
	return exists;
}
bool FilterFileLine::isFileLinePresent (QString const & fileline, TreeModelItem & fi) const
{
	TreeModelItem const * tmp_fi = 0;
	bool const exists = m_data.is_present(fileline, tmp_fi);
	if (exists)
		fi = *tmp_fi;
	return exists;
}

void FilterFileLine::merge_rhs (node_t * lhs, node_t const * rhs)
{
	node_t const * rhs_child = rhs->children;
	while (rhs_child)
	{
		node_t * lhs_child = lhs->children;
		bool found = false;
		while (lhs_child)
		{
			if (lhs_child->key == rhs_child->key)
			{
				found = true;
				break;
			}
			lhs_child = lhs_child->next;
		}

		if (!found)
		{
			lhs_child = new node_t(*rhs_child);
			node_t::node_append(lhs, lhs_child);
		}
		else
			lhs_child->data = rhs_child->data;

		merge(lhs_child, rhs_child);

		rhs_child = rhs_child->next;
	}
}

void FilterFileLine::merge_state (node_t * lhs, node_t const * rhs)
{
	node_t * lhs_child = lhs->children;

	while (lhs_child)
	{
		node_t * rhs_child = rhs->children;
		bool found = false;
		while (rhs_child)
		{
			if (rhs_child->key == lhs_child->key)
			{
				found = true;
				// assert na state
				break;
			}
			rhs_child = rhs_child->next;
		}

		if (found)
		{
			lhs_child->data = rhs_child->data;
			merge_state(lhs_child, rhs_child);
		}
		else
		{
			node_t * parent = lhs_child->parent;
			if (parent)
			{
				switch (parent->data.m_state)
				{
					case e_Unchecked:
						m_data.set_state_to_childs(lhs_child, parent->data);
						break;
					case e_PartialCheck:
						m_data.set_state_to_childs(lhs_child, TreeModelItem(e_Unchecked, 1));
						break;
					case e_Checked:
						m_data.set_state_to_childs(lhs_child, parent->data);
						break;
				}
			}
		}

		lhs_child = lhs_child->next;
	}
}

void FilterFileLine::merge (node_t * lhs, node_t const * rhs)
{
	merge_rhs(lhs, rhs);
	merge_state(lhs, rhs);
}

void FilterFileLine::mergeWithConfig (file_filters_t const & rhs)
{   
	node_t * const rhs_root = rhs.root;
	if (m_data.root && rhs_root)
		m_data.root->data = rhs_root->data;
	merge(m_data.root, rhs_root);
} 

TreeView * FilterFileLine::getWidget () { return m_ui->view; }
TreeView const * FilterFileLine::getWidget () const { return m_ui->view; }

void FilterFileLine::onCutParentValueChanged (int i)
{
	//if (Connection * conn = findCurrentConnection())
	//	conn->onCutParentValueChanged(i);

	m_model->onCutParentValueChanged(i);
	//filterWidget()->getWidget()->hideLinearParents();
}

void FilterFileLine::onCollapseChilds ()
{
	m_model->collapseChilds(m_ui->view);
}

void FilterFileLine::recompile ()
{ }

void FilterFileLine::onCancelFilterFileButton ()
{
	m_ui->filterFileComboBox->clearEditText();
	m_ui->cancelFilterButton->setEnabled(false);
	m_ui->cancelFilterButton->setStyleSheet("color: rgb(128, 128, 128)"); 
}


void FilterFileLine::onFilterFileComboChanged (QString str)
{
	bool cancel_on = !str.isEmpty();
	m_ui->cancelFilterButton->setEnabled(cancel_on);
	if (cancel_on)
		m_ui->cancelFilterButton->setStyleSheet("color: rgb(255, 0, 0)"); 
	else
		m_ui->cancelFilterButton->setStyleSheet("color: rgb(128, 128, 128)"); 

	if (str.isEmpty())
	{
		m_ui->view->setModel(m_model);
	}
	else
	{
		if (m_ui->view->model() != m_proxy)
		{
			m_ui->view->setModel(m_proxy);
			m_proxy->setSourceModel(m_model);
		}
		m_proxy->setFindString(str);
	}
}


void FilterFileLine::onSelectAll ()
{
	m_model->stateToChildren(m_data.root, Qt::Checked);
	emitFilterChangedSignal();
}

void FilterFileLine::onSelectNone ()
{
	m_model->stateToChildren(m_data.root, Qt::Unchecked);
	emitFilterChangedSignal();
}

void FilterFileLine::onClickedAtFileTree (QModelIndex idx)
{
/*	TreeModel<DockedInfo>::node_t const * n = m_docked_widgets_model->getItemFromIndex(idx);
	QStringList const & dst = n->data.m_path;

	int const col = idx.column();
	Action a;
	a.m_type = static_cast<E_ActionType>(col);
	a.m_src_path = path();
	a.m_src = this;
	a.m_dst_path = dst;
	if (col == e_Visibility)
	{
		int const state = m_docked_widgets_model->data(idx, Qt::CheckStateRole).toInt();
		a.m_args.push_back(state);
	}
	if (col == e_InCentralWidget)
	{
		int const state = m_docked_widgets_model->data(idx, e_DockRoleCentralWidget).toInt();
		int const new_state = state == 0 ? 1 : 0;

		m_docked_widgets_model->setData(idx, new_state, e_DockRoleCentralWidget);
		a.m_args.push_back(new_state);
	}*/


	emitFilterChangedSignal();


/*	FilterFileLine * const node = m_model->getItemFromIndex(idx);
	FilterFileLine const * line_node = 0;

	s.clear();
	s.reserve(16);
	if (!node->hasChildren())
		line_node = node;
	else
		s.push_back(model->data(idx, Qt::DisplayRole).toString());

	QStandardItem * parent = node->parent();
	std::string file;
	QModelIndex parent_idx = model->indexFromItem(parent);
	while (parent_idx.isValid())
	{
		QString const & val = model->data(parent_idx, Qt::DisplayRole).toString();
		s.push_back(val);
		parent = parent->parent();
		parent_idx = model->indexFromItem(parent);
	}

	for (std::vector<QString>::const_reverse_iterator it=s.rbegin(), ite=s.rend(); it != ite; ++it)
		file += std::string("/") + (*it).toStdString();

	fileline_t filter_node(file, std::string());
	if (line_node)
	{
		QString const & val = model->data(idx, Qt::DisplayRole).toString();
		filter_node.second = val.toStdString();
	}
	std::string const fileline = filter_node.first + "/" + filter_node.second;

	Qt::CheckState const curr_state = node->checkState();
	if (curr_state == Qt::Checked)
	{
		// unchecked --> checked
		setCheckStateChilds(node, curr_state);
		conn->sessionState().m_file_filters.set_state_to_childs(fileline, static_cast<E_NodeStates>(curr_state));

		QStandardItem * p = node;
		while (p = p->parent())
		{
			bool const all_checked = checkChildState(p, Qt::Checked);
			if (all_checked)
			{
				//conn->sessionState().m_file_filters.set_state_to_childs(fileline, static_cast<E_NodeStates>(curr_state));
				p->setCheckState(Qt::Checked);
			}
		}
	}
	else if (curr_state == Qt::Unchecked)
	{
		// checked --> unchecked
		set_state_to_topdown(conn->sessionState().m_file_filters, fileline, static_cast<E_NodeStates>(curr_state), e_PartialCheck);
		setCheckStateChilds(node, curr_state);
		setCheckStateReverse(node->parent(), Qt::PartiallyChecked); // iff parent unchecked and clicked on leaf
	}

	E_NodeStates const new_state = static_cast<E_NodeStates>(curr_state);

	qDebug("file click! sync state of %s --> node_checkstate=%i", fileline.c_str(), node->checkState());
	conn->sessionState().m_file_filters.set_to_state(fileline, static_cast<E_NodeStates>(new_state));
	conn->onInvalidateFilter();*/
}

