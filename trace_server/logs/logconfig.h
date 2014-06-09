#pragma once
#include <QString>
#include <QVector>
#include <QColor>
#include <dockedconfig.h>
#include <constants.h>
#include <findconfig.h>
#include <colorizeconfig.h>

namespace logs {

	struct LogConfig : DockedConfigBase
	{
		QString m_tag;
		int m_history_ln;
		QString m_font;
		int m_fontsize;
		int m_row_width;
		int m_indent_level;
		int m_cut_path_level;
		int m_cut_namespace_level;
		int m_curr_tooltab;
		std::vector<QString> 	m_columns_setup;		/// column setup for each registered application
		std::vector<int> 		m_columns_sizes;		/// column sizes for each registered application
		std::vector<QString> 	m_columns_align;		/// column align for each registered application
		std::vector<QString> 	m_columns_elide;		/// column elide for each registered application
		std::vector<QColor> 	m_thread_colors;		/// predefined coloring of threads
		std::vector<QString> 	m_storage_order;
		bool m_in_view;
		bool m_filtering;
		bool m_clr_filters;
		bool m_scopes;
		bool m_indent;
		bool m_cut_path;
		bool m_cut_namespaces;
		bool m_dt_enabled;
		bool m_filter_proxy;
		bool m_find_proxy;
		bool m_simplify_strings;
		bool m_unquote_strings;
		bool m_csv_has_header;
		QString m_csv_separator;
		FindConfig m_find_config;
		ColorizeConfig m_colorize_config;

		LogConfig ()
			: m_tag()
			, m_history_ln(128*128)
			, m_font("Verdana")
			, m_fontsize(10)
			, m_row_width(18)
			, m_indent_level(2)
			, m_cut_path_level(2)
			, m_cut_namespace_level(3)
			, m_curr_tooltab(1) // 1 == filters
			, m_in_view(true)
			, m_filtering(true)
			, m_clr_filters(true)
			, m_scopes(true)
			, m_indent(true)
			, m_cut_path(true)
			, m_cut_namespaces(true)
			, m_dt_enabled(false)
			, m_filter_proxy(false)
			, m_find_proxy(false)
			, m_simplify_strings(false)
			, m_unquote_strings(false)
			, m_csv_has_header(false)
			, m_csv_separator()
		{ }

		LogConfig (QString const & tag)
			: m_tag(tag)
			, m_history_ln(128*128)
			, m_font("Verdana")
			, m_fontsize(10)
			, m_row_width(18)
			, m_indent_level(2)
			, m_cut_path_level(2)
			, m_cut_namespace_level(3)
			, m_curr_tooltab(1) // 1 == filters
			, m_in_view(true)
			, m_filtering(true)
			, m_clr_filters(true)
			, m_scopes(true)
			, m_indent(true)
			, m_cut_path(true)
			, m_cut_namespaces(true)
			, m_dt_enabled(false)
			, m_filter_proxy(false)
			, m_find_proxy(false)
			, m_simplify_strings(false)
			, m_unquote_strings(false)
			, m_csv_has_header(false)
			, m_csv_separator()
		{ }

		template <class ArchiveT>
		void serialize (ArchiveT & ar, unsigned const version)
		{
			DockedConfigBase::serialize(ar, version);
			ar & boost::serialization::make_nvp("tag", m_tag);
			ar & boost::serialization::make_nvp("history_ln", m_history_ln);
			ar & boost::serialization::make_nvp("font", m_font);
			ar & boost::serialization::make_nvp("fontsize", m_fontsize);
			ar & boost::serialization::make_nvp("row_width", m_row_width);
			ar & boost::serialization::make_nvp("indent_level", m_indent_level);
			ar & boost::serialization::make_nvp("cut_path_level", m_cut_path_level);
			ar & boost::serialization::make_nvp("cut_namespace_level", m_cut_namespace_level);
			ar & boost::serialization::make_nvp("curr_tooltab", m_curr_tooltab);
			ar & boost::serialization::make_nvp("columns_setup", m_columns_setup);
			ar & boost::serialization::make_nvp("columns_sizes", m_columns_sizes);
			ar & boost::serialization::make_nvp("columns_align", m_columns_align);
			ar & boost::serialization::make_nvp("columns_elide", m_columns_elide);
			ar & boost::serialization::make_nvp("thread_colors", m_thread_colors);
			ar & boost::serialization::make_nvp("storage_order", m_storage_order);
			ar & boost::serialization::make_nvp("in_view", m_in_view);
			ar & boost::serialization::make_nvp("filtering", m_filtering);
			ar & boost::serialization::make_nvp("clr_filters", m_clr_filters);
			ar & boost::serialization::make_nvp("scopes", m_scopes);
			ar & boost::serialization::make_nvp("indent", m_indent);
			ar & boost::serialization::make_nvp("cut_path", m_cut_path);
			ar & boost::serialization::make_nvp("cut_namespaces", m_cut_namespaces);
			ar & boost::serialization::make_nvp("dt_enabled", m_dt_enabled);
			ar & boost::serialization::make_nvp("find_config", m_find_config);
			ar & boost::serialization::make_nvp("simplify_strings", m_simplify_strings);
			ar & boost::serialization::make_nvp("unquote_strings", m_unquote_strings);
			ar & boost::serialization::make_nvp("csv_has_header", m_csv_has_header);
			ar & boost::serialization::make_nvp("csv_separator", m_csv_separator);
			ar & boost::serialization::make_nvp("colorize_config", m_colorize_config);
		}

		void clear ()
		{
			*this = LogConfig();
		}
	};

	bool loadConfig (LogConfig & config, QString const & fname);
	bool saveConfig (LogConfig const & config, QString const & fname);
	void fillDefaultConfig (LogConfig & config);
	bool validateConfig (logs::LogConfig const & cfg);
}

