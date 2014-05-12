#include "dockdelegates.h"
#include <QPushButton>
#include "dockmanager.h"

CloseButtonDelegate::CloseButtonDelegate (DockManager & mgr, QObject * parent)
	: QStyledItemDelegate(parent)
	, m_dock_mgr(mgr)
{ }

QWidget * CloseButtonDelegate::createEditor (QWidget * parent, QStyleOptionViewItem const &, QModelIndex const & idx) const
{
	if (idx.isValid() && idx.column() == DockManager::e_Column_Close)
	{
		QPushButton * b = new QPushButton("X", parent);
		b->setStyleSheet("color: rgb(255, 0, 0)");
		b->setProperty("idx", QVariant(idx));
		connect(b, SIGNAL(clicked()), &m_dock_mgr, SLOT(onCloseButton()));
		return b;
	}
	return 0;
}

void CloseButtonDelegate::setEditorData (QWidget * editor, QModelIndex const & index) const { }
void CloseButtonDelegate::setModelData (QWidget * editor, QAbstractItemModel * model, QModelIndex const & index) const { }
void CloseButtonDelegate::updateEditorGeometry (QWidget * editor, QStyleOptionViewItem const & option, QModelIndex const & idx) const { editor->setGeometry(option.rect); }


ControlWidgetDelegate::ControlWidgetDelegate (DockManager & mgr, QObject * parent)
	: QStyledItemDelegate(parent)
	, m_dock_mgr(mgr)
{ }

QWidget * ControlWidgetDelegate::createEditor (QWidget * parent, QStyleOptionViewItem const &, QModelIndex const & idx) const
{
	if (idx.isValid() && idx.column() == DockManager::e_Column_ControlWidget)
	{
		if (DockManager::node_t const * n = m_dock_mgr.m_model->getItemFromIndex(idx))
		{
			QStringList const & dst = n->data.m_path;
			if (ActionAble * aa = m_dock_mgr.findActionAble(dst.join("/")))
			{
				if (QWidget * w = aa->controlWidget())
				{
					w->setParent(parent);
					return w;
				}
			}
		}
	}
	return 0;
}

void ControlWidgetDelegate::destroyEditor (QWidget * editor, QModelIndex const & index) const
{
	// no-op
}
void ControlWidgetDelegate::setEditorData (QWidget * editor, QModelIndex const & index) const { }
void ControlWidgetDelegate::setModelData (QWidget * editor, QAbstractItemModel * model, QModelIndex const & index) const { }
void ControlWidgetDelegate::updateEditorGeometry(QWidget * editor, QStyleOptionViewItem const & option, QModelIndex const & idx) const { editor->setGeometry(option.rect); }



