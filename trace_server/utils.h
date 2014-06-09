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
#include <QString>
#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QSettings>
#include <QInputDialog>
#include "constants.h"

inline QString unquoteString (QString const & str)
{
	QString result = str;
	if (result.startsWith("\""))
		result.remove(0, 1);
	if (result.endsWith("\""))
		result.remove(result.size() - 1, 1);
	return result;
}

inline QString unquoteString (QString const & str, bool enable_unquote, bool enable_simplify)
{
	//QString const simple = enable_simplify ? str.simplified() : str;
	QString const simple = enable_simplify ? str.trimmed() : str;
	return enable_unquote ? unquoteString(simple) : simple;
}

template<class C>
void clearListView (C * v)
{
	if (v && v->model())
	{
		static_cast<QStandardItemModel *>(v->model())->clear();
	}
}

inline bool existsFile (char const * fname) { QFile file(fname); return file.exists(); }
inline bool existsFile (QString const & fname) { QFile file(fname); return file.exists(); }

inline bool validatePresetName (QString const & str)
{
	//@TODO: valid filename
	return true;
/*	int const slash_pos = str.lastIndexOf(QChar('/'));
	bool const slash_present = (slash_pos != -1);
	// @TODO: some more checks? make in tmp, verify in tmp, return result
	return slash_present;*/
}

inline QString promptInputDialog (QStringList const & items)
{
	QString const default_pname = g_defaultPresetName;
	QString preset_name;
	bool ok = true;
	while (ok)
	{
		QString const pname = QInputDialog::getItem(0, "Save current preset", "Preset name:", items, 0, true, &ok);
		if (ok && validatePresetName(pname))
		{
			preset_name = pname;
			break;
		}
	}
	if (!validatePresetName(preset_name))
		return default_pname;
	return preset_name;
}

inline int findPresetsForApp (QString const & appdir, QString const & appname, QStringList & presets)
{
	QString presetdir = appdir + "/" + appname;
	//QDirIterator directories(presetdir, QDir::Dirs | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
	QDirIterator directories(presetdir, QDir::Dirs | QDir::NoDotAndDotDot, QDirIterator::NoIteratorFlags);
	while (directories.hasNext()) {
		directories.next();
		//qDebug("candidate for preset: %s", directories.fileName().toStdString().c_str());
		presets << directories.fileName();
	}
	return presets.size();
}

inline QString promptAndCreatePresetName ()
{
	QString const default_pname = g_defaultPresetName;
	QStringList items;
	items.push_front(default_pname);
    return promptInputDialog(items);
}
inline QString promptAndCreatePresetNameForApp (QString const & app_dir, QString const & app_name)
{
	QString const default_pname = g_defaultPresetName;
	// pre-select default_pname (for text-replace mode)
	QStringList items;
	findPresetsForApp(app_dir, app_name, items);
	items.push_front(default_pname);
    return promptInputDialog(items);
}

inline bool checkAppPresetPath (QString const & appdir, QString const & app_name, QString const & preset_name)
{
	QString presetdir = appdir + "/" + app_name + "/" + preset_name;
	if (existsFile(presetdir))
		return true;
	return false;
}

inline QString createAppPresetPath (QString const & appdir, QString const & app_name, QString const & preset_name)
{
	QString presetdir = appdir + "/" + app_name + "/" + preset_name;
	QDir d;
	d.mkpath(presetdir);
	qDebug("mk app preset path: %s", presetdir.toStdString().c_str());
	return presetdir;
}

inline QString mkWidgetFileName (QString const & appdir, QString const & app_name, QString const & preset_name, QString const & class_name, QString const & tag, QString const & widget_fname)
{
	QString widgetdir = appdir + "/" + app_name + "/" + preset_name + "/" + class_name + "/" + tag;
	//QDir d;
	QString const fname = widgetdir + "/" + widget_fname;
	qDebug("cfg fname for data: %s", fname.toStdString().c_str());
	return fname;
}

inline bool mkPath (QString const & dir)
{
	QDir d;
	return d.mkpath(dir);
}

inline QString mkWidgetDir (QString const & appdir, QString const & app_name, QString const & preset_name, QString const & class_name, QString const & tag)
{
	QString presetdir = appdir + "/" + app_name + "/" + preset_name + "/" + class_name + "/" + tag;
	QDir d;
	d.mkpath(presetdir);
	return presetdir;
}

inline QString getWidgetFileNameAndMkPath (QString const & appdir, QString const & app_name, QString const & preset_name, QString const & class_name, QString const & tag, QString const & widget_fname)
{
	QString widgetdir = appdir + "/" + app_name + "/" + preset_name + "/" + class_name + "/" + tag;
	QDir d;
	d.mkpath(widgetdir);
	QString const fname = widgetdir + "/" + widget_fname;
	qDebug("mk cfg path for fname: %s", fname.toStdString().c_str());
	return fname;
}

inline QString mkPresetPath (QString const & app_dir, QString const & preset_name)
{
	QString const path = app_dir + "/" + preset_name;
	return path;
}

inline QString mkAppPresetPath (QString const & app_dir, QString const & app_name, QString const & name)
{
	QString const path = app_dir + "/" + app_name + "/" + name;
	return path;
}

inline void reassemblePath (std::vector<QString> const & tokens, QString & path)
{
	for (std::vector<QString>::const_reverse_iterator it = tokens.rbegin(), ite = tokens.rend(); it != ite; ++it)
	{
		path += *it;
		path += '/';
	}
}

#include <filters/file_filter.hpp>
template <class F>
void set_state_to_childs (tree_filter<F> const & ff, std::string const & file, E_NodeStates state);
template <class F>
void set_state_to_parents (tree_filter<F> const & ff, typename tree_filter<F>::node_t * node, E_NodeStates state);
template <class F>
void set_state_to_childs (tree_filter<F> const & ff, typename tree_filter<F>::node_t * node, E_NodeStates state);

template <class F>
inline void set_state_to_topdown (tree_filter<F> const & ff, std::string const & file, E_NodeStates fw_state, E_NodeStates rev_state)
{
	typedef typename tree_filter<F>::tokenizer_t tokenizer_t;
	tokenizer_t tok(file, ff.separator);
	typename tree_filter<F>::node_t * level = ff.root;
	typename tokenizer_t::const_iterator it = tok.begin(), ite = tok.end();
	while (it != ite)
	{
		level = tree_filter<F>::node_t::node_child(level, *it);
		if (level == 0)
			return;

		++it;
		if (it == ite)
		{
			level->data.m_state = fw_state;
			set_state_to_childs(ff, level, fw_state);
			set_state_to_parents(ff, level, rev_state);
		}
	}
}

template <class F>
inline void set_state_to_childs (tree_filter<F> const & ff, std::string const & file, E_NodeStates state)
{
	typename tree_filter<F>::tokenizer_t tok(file, ff.separator);
	typename tree_filter<F>::node_t * level = ff.root;
	typename tree_filter<F>::tokenizer_t::const_iterator it = tok.begin(), ite = tok.end();
	while (it != ite)
	{
		level = tree_filter<F>::node_t::node_child(level, *it);
		if (level == 0)
			return;

		++it;
		if (it == ite)
		{
			level->data.m_state = state;
			set_state_to_childs(ff, level, state);
		}
	}
}

template <class F>
inline void set_state_to_childs (tree_filter<F> const & ff, typename tree_filter<F>::node_t * node, E_NodeStates state)
{
	node = node->children;
	while (node)
	{
		node->data.m_state = state;
		set_state_to_childs(ff, node, state);
		node = node->next;
	}
}

template <class F>
inline void set_state_to_parents (tree_filter<F> const & ff, typename tree_filter<F>::node_t * node, E_NodeStates state)
{
	while (node = node->parent)
	{
		node->data.m_state = state;
		set_state_to_parents(ff, node, state);
	}
}

