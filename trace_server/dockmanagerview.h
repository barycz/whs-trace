#pragma once
#include "treeview.h"

class DockManagerView : public TreeView
{
	Q_OBJECT
public:
	DockManagerView (QWidget * parent = 0);

	virtual void DockManagerView::keyPressEvent (QKeyEvent * e);
signals:
	void removeCurrentIndex (QModelIndex const & idx);
};

