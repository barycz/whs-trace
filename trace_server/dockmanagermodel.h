#pragma once
#include "treemodel.h"
#include "dockedinfo.h"
struct DockManager;

class DockManagerModel : public TreeModel<DockedInfo>
{
	DockManager & m_manager;
	Q_OBJECT
public:

	explicit DockManagerModel (DockManager & mgr, QObject * parent = 0, tree_data_t * data = 0);
	~DockManagerModel ();

	QModelIndex insertItemWithPath (QStringList const & path, bool checked);
	QModelIndex testItemWithPath (QStringList const & path);

	TreeModel<DockedInfo>::node_t const * getItemFromIndex (QModelIndex const & index) const { return itemFromIndex(index); }
	TreeModel<DockedInfo>::node_t * getItemFromIndex (QModelIndex const & index) { return itemFromIndex(index); }
	//DockedWidgetBase const * getWidgetFromIndex (QModelIndex const & index) const;
	//DockedWidgetBase * getWidgetFromIndex (QModelIndex const & index);

	virtual int columnCount (QModelIndex const & parent) const;
	virtual QVariant data (const QModelIndex & index, int role = Qt::DisplayRole) const;
	Qt::ItemFlags flags (QModelIndex const & index) const;
	virtual bool setData (QModelIndex const & index, QVariant const & value, int role = Qt::EditRole);

public slots:
	void onExpanded (QModelIndex const & idx) { expanded(idx); }
	void onCollapsed (QModelIndex const & idx) { collapsed(idx); }
};


