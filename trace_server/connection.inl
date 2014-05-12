#pragma once
#include "dockwidget.h"

template <int TypeN>
typename SelectIterator<TypeN>::type Connection::dataWidgetFactory (QString const tag)
{
	typedef typename SelectIterator<TypeN>::type iterator;
	iterator it = m_data.get<TypeN>().find(tag);
	if (it == m_data.get<TypeN>().end())
	{
		qDebug("datawidget factory: creating type=%i with %s", TypeN, tag.toStdString().c_str());

		typedef typename SelectWidget<TypeN>::type widget_t;
		typedef typename SelectConfig<TypeN>::type config_t;

		QString const preset_name = getClosestPresetName();
		m_curr_preset = preset_name;
		char const * widget_prefix = g_fileTags[TypeN];
		char const * widget_fname = g_fileNames[TypeN];
		QString const fname = mkWidgetFileName(getGlobalConfig().m_appdir, getAppName(), preset_name, widget_prefix, tag, widget_fname);
		
		QStringList path;
		mkWidgetPath(static_cast<E_DataWidgetType>(TypeN), tag, path);
		config_t default_cfg;
		widget_t * const widget = new widget_t(this, default_cfg, fname, path);
		it = m_data.get<TypeN>().insert(tag, widget);

		widget->config().m_tag = tag;
		if (!preset_name.isEmpty())
		{
			QString const cfg_path = mkAppPresetPath(getGlobalConfig().m_appdir, getAppName(), m_curr_preset);
			widget->loadConfig(cfg_path);
		}

		//@TODO: pozdeji
		//m_main_window->mentionInPresetHistory(m_curr_preset);
		//m_main_window->setPresetAsCurrent(m_curr_preset);

		bool const visible = widget->config().m_show;
		DockWidget * dock = m_main_window->dockManager().mkDockWidget(*widget, visible);
		widget->setDockWidget(dock);
		dock->setWidget(widget);
		//note: applyConfig is handled by the caller

		QModelIndex const item_idx = m_main_window->m_dock_mgr.addActionAble(*widget, visible);

		if (getClosestFeatureState(static_cast<E_DataWidgetType>(TypeN)) == e_FtrEnabled && visible)
		{
			widget->setVisible(visible);
			getMainWindow()->onDockRestoreButton();
			//m_main_window->restoreDockWidget(dock);
		}
		else
		{
			widget->setVisible(false);
			// @TODO: visible to data_tree?
		}
	}
	return it;
}

template <int TypeN>
bool Connection::dataWidgetConfigPreload (QString const tag, typename SelectConfig<TypeN>::type & config)
{
	QString const preset_name = getClosestPresetName();
	char const * widget_prefix = g_fileTags[TypeN];
	char const * widget_fname = g_fileNames[TypeN];
	QString const fname = mkWidgetFileName(getGlobalConfig().m_appdir, getAppName(), preset_name, widget_prefix, tag, widget_fname);
	
	if (!preset_name.isEmpty())
	{
		return ::loadConfigTemplate(config, fname);
	}
	return false;
}

template <int TypeN>
typename SelectIterator<TypeN>::type Connection::dataWidgetFactoryFrom (QString const tag, typename SelectConfig<TypeN>::type const & config)
{
	typedef typename SelectIterator<TypeN>::type iterator;
	iterator it = m_data.get<TypeN>().find(tag);
	if (it == m_data.get<TypeN>().end())
	{
		qDebug("datawidget factory with config: creating type=%i with %s", TypeN, tag.toStdString().c_str());

		typedef typename SelectWidget<TypeN>::type widget_t;
		typedef typename SelectConfig<TypeN>::type config_t;

		QString const preset_name = getClosestPresetName();
		//m_curr_preset = preset_name; // 
		char const * widget_prefix = g_fileTags[TypeN];
		char const * widget_fname = g_fileNames[TypeN];
		QString const fname = mkWidgetFileName(getGlobalConfig().m_appdir, getAppName(), preset_name, widget_prefix, tag, widget_fname);
		
		QStringList path;
		mkWidgetPath(static_cast<E_DataWidgetType>(TypeN), tag, path);
		widget_t * const widget = new widget_t(this, config, fname, path);
		it = m_data.get<TypeN>().insert(tag, widget);
		widget->config().m_tag = tag;
		bool const visible = widget->config().m_show;
		DockWidget * dock = m_main_window->m_dock_mgr.mkDockWidget(*widget, visible);
		widget->setDockWidget(dock);
		dock->setWidget(widget);
		// no applyConfig here!

		if (visible)
		{
			widget->setVisible(visible);

			getMainWindow()->onDockRestoreButton();
			//m_main_window->restoreDockWidget(dd->m_wd);
		}
		else
		{
			widget->setVisible(false);
			// @TODO: visible to data_tree?
			// @TODO: visible to widget conf? probably not..
		}

		QModelIndex const item_idx = m_main_window->m_dock_mgr.addActionAble(*widget, visible);
	}
	return it;
}

template <int TypeN>
void Connection::removeDockedWidget (DockedWidgetBase * ptr) // ehm
{
	foreach (QString key, m_data.get<TypeN>().keys() )
	{
		if (m_data.get<TypeN>().value(key) == ptr)
		{
			m_data.get<TypeN>().remove(key);
			return;
		}
	}
}

