#include "dockmanagerview.h"
#include <QKeyEvent>

DockManagerView::DockManagerView (QWidget * parent)
	: TreeView(parent)
{
	setEditTriggers(QAbstractItemView::CurrentChanged);
}

void DockManagerView::keyPressEvent (QKeyEvent * e)
{
	if (e->type() == QKeyEvent::KeyPress)
	{
		bool const ctrl = (e->modifiers() & Qt::ControlModifier) == Qt::ControlModifier;
		bool const shift = (e->modifiers() & Qt::ShiftModifier) == Qt::ShiftModifier;
		bool const alt = (e->modifiers() & Qt::AltModifier) == Qt::AltModifier;
		if (!ctrl && !shift && !alt && e->key() == Qt::Key_Delete)
		{
			e->accept();
			emit removeCurrentIndex(currentIndex());
			return;
		}
	}
	QTreeView::keyPressEvent(e);
}

