#pragma once
#include <QStyledItemDelegate>
struct DockManager;

class CloseButtonDelegate : public QStyledItemDelegate
{
	Q_OBJECT
	DockManager & m_dock_mgr;
public:
	CloseButtonDelegate (DockManager & mgr, QObject * parent = 0);
	virtual QWidget * createEditor (QWidget * parent, QStyleOptionViewItem const & option, QModelIndex const & index) const;
	virtual void setEditorData (QWidget * editor, QModelIndex const & index) const;
	virtual void setModelData (QWidget * editor, QAbstractItemModel * model, QModelIndex const & index) const;
	virtual void updateEditorGeometry (QWidget * editor, QStyleOptionViewItem const & option, QModelIndex const & index) const;
};

class ControlWidgetDelegate : public QStyledItemDelegate
{
	Q_OBJECT
	DockManager & m_dock_mgr;
public:
	ControlWidgetDelegate (DockManager & mgr, QObject * parent = 0);
	virtual QWidget * createEditor (QWidget * parent, QStyleOptionViewItem const & option, QModelIndex const & index) const;
	virtual void setEditorData (QWidget * editor, QModelIndex const & index) const;
	virtual void setModelData (QWidget * editor, QAbstractItemModel * model, QModelIndex const & index) const;
	virtual void updateEditorGeometry (QWidget * editor, QStyleOptionViewItem const & option, QModelIndex const & index) const;
	virtual void destroyEditor (QWidget * editor, QModelIndex const & index) const;
};



