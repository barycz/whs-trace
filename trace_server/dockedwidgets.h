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
#include <QObject>
#include <QString>
#include <QMap>
#include "controlbardockedwidgets.h"

struct DockedWidgetsConfig {

	bool	m_auto_scroll;
	bool	m_show;
	int		m_sync_group;
	int		m_level;
	QString m_time_units_str;
	float	m_time_units;
	int		m_widget_recv_level;

	DockedWidgetsConfig ()
		: m_auto_scroll(false)
		, m_show(true)
		, m_sync_group(0)
		, m_level(3)
		, m_time_units_str("ms")
		, m_time_units(stringToUnitsValue(m_time_units_str))
	{ }

	template <class ArchiveT>
	void serialize (ArchiveT & ar, unsigned const version)
	{
		ar & boost::serialization::make_nvp("autoscroll", m_auto_scroll);
		ar & boost::serialization::make_nvp("show", m_show);
		ar & boost::serialization::make_nvp("sync_group", m_sync_group);
		ar & boost::serialization::make_nvp("level", m_level);
		ar & boost::serialization::make_nvp("time_units_str", m_time_units);
		ar & boost::serialization::make_nvp("time_units", m_time_units);
		ar & boost::serialization::make_nvp("widget_recv_level", m_widget_recv_level);
	}
};

// Q_OBJECT cannot handle templates, so we do the QObject stuff here
class DockedWidgetsBase: public QObject
{
	DockedWidgetsBase(const DockedWidgetsBase &);
	Q_OBJECT
public:
	DockedWidgetsBase(QObject * parent = NULL): QObject(parent) {}

signals:
	// TODO: change the type after we create some qt-friendly base for all the widgets
	void widgetAdded(const QString & path, QWidget * w);
	void widgetRemoved(const QString & path, QWidget * w);

};

template <class WidgetT, class ConfigT>
class DockedWidgets
	: public DockedWidgetsBase
	, public ActionAble
{
	DockedWidgets(const DockedWidgets &);
public:
	enum { e_type = WidgetT::e_type };
	typedef WidgetT widget_t;
	typedef ConfigT config_t;
	typedef QString key_t;
	typedef widget_t * value_t;
	typedef QMap<key_t, value_t> widget_map_t;
	typedef typename widget_map_t::iterator iterator;
	typedef typename widget_map_t::const_iterator const_iterator;
	typedef QList<key_t> key_list_t;

	DockedWidgets ()
		: ActionAble(QStringList())
		, m_control_bar(0)
	{
		m_queue.reserve(1024);
		m_control_bar = new ControlBarDockedWidgets();
	}

	~DockedWidgets ()
	{
		//m_main_window->dockManager().removeActionAble(*this);
	}

	//////////////////////////////////////////////////////////////////////////
	// QMap-compatible interface
	iterator begin() { return m_map.begin(); }
	iterator end() { return m_map.end(); }
	const_iterator begin() const { return m_map.begin(); }
	const_iterator end() const { return m_map.end(); }
	iterator find(const key_t & key) { return m_map.find(key); }
	const_iterator find(const key_t & key) const { return m_map.find(key); }
	const value_t value(const key_t & key, const value_t & defaultValue = value_t()) const { return m_map.value(key, defaultValue); }
	key_list_t keys() const { return m_map.keys(); }

	iterator insert(const key_t & key, const value_t & value)
	{
		iterator ret = m_map.insert(key, value);
		emit widgetAdded(value->joinedPath(), value);
		return ret;
	}

	void remove(const key_t & key)
	{
		// TODO: this does not delete the widget - remove this inconsistency
		iterator it = find(key);
		if(it == end())
		{
			return;
		}
		emit widgetRemoved((*it)->joinedPath(), *it);
		m_map.erase(it);
	}

	void clear()
	{
		for (iterator it = begin(); it != end(); ++it)
		{
			emit widgetRemoved((*it)->joinedPath(), *it);
			(*it)->setParent(0);
			delete *it;
		}
		m_map.clear();
	}
	//////////////////////////////////////////////////////////////////////////

	virtual bool handleAction (Action * a, E_ActionHandleType sync)
	{
		if (a->type() == e_Close)
		{
			clear();
			return true;
		}
		else
		{
			for (iterator it = begin(), ite = end(); it != ite; ++it)
				(*it)->handleAction(a, sync);
			return true;
		}
	}

	virtual QWidget * controlWidget () { return m_control_bar; }

	QList<DecodedCommand> & queue() { return m_queue; }
	const QList<DecodedCommand> & queue() const { return m_queue; }

private:
	DockedWidgetsConfig m_config;
	ControlBarDockedWidgets * m_control_bar;
	QList<DecodedCommand> m_queue;
	widget_map_t m_map;
};


