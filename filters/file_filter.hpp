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

//#define USE_STD_STRING 1
#if !defined USE_STD_STRING
#	define USE_QT_STRING 1
#endif

#if (USE_QT_STRING == 1)
#	include <QString>
	typedef QString string_type_t;
	typedef QChar char_type_t;
	inline char_type_t const * get_raw_cstring (string_type_t const & str) { return str.constData(); }
#endif
#if (USE_STD_STRING == 1)
#	include <string>
	typedef std::string string_type_t;
	typedef char char_type_t;
	inline char_type_t const * get_raw_cstring (string_type_t const & str) { return str.c_str(); }
#endif

#include <vector>
#include <boost/serialization/split_member.hpp>
#include <boost/serialization/nvp.hpp>
#include "nnode.hpp"

template <class FilterT>
struct tree_filter
{
	typedef FilterT filter_t;
	typedef NNode<string_type_t, filter_t> node_t;
	node_t * root;

	tree_filter ()
		: root(new node_t("/", filter_t()))
	{ }

	~tree_filter ()
	{
		clear();
	}

	void clear ()
	{
		if (root)
		{
			node_t::node_destroy(root);
			root = 0;
		}
	}

	bool empty () const { return !(root && root->children); }


	node_t * set_to_state (string_type_t const & path, filter_t const & data)
	{
		char_type_t const * const bgn = get_raw_cstring(path);
		//char_type_t const * const bgn = path.constData();
		char_type_t const * const end = bgn + path.size() + 1;

		char_type_t const * left = bgn;
		char_type_t const * right = bgn;
		node_t * level = root;
		for (; right < end; ++right)
		{
			char_type_t const c = *right;
			if ((c == ':' && right == bgn + 1) || c == '/' || c == '\\' || c == '\0' || right + 1 == end)
			{
				if (right - left > 0)
				{
					node_t * child = node_t::node_child_fast(level, left, right);
					if (child == 0)
					{
						child = new node_t(left, right, data);
						node_t::node_append(level, child);
					}
					level = child;
				}

				if (right + 1 < end)
					left = right + 1;
			}
		}

		level->data = data;
		return level;
	}

/*	bool is_present (string_type_t const & path, filter_t & data) const
	{
		tokenizer_t tok(path, separator);
		node_t const * level = root;
	   	for (tokenizer_t::iterator it = tok.begin(), ite = tok.end(); it != ite; ++it)
		{
			level = node_t::node_child(level, *it);
			if (level == 0)
				return false; // node not in tree
			data = level->data;
		}
		return true;
	}*/

	node_t const * is_present (string_type_t const & path, filter_t const * & fi) const
	{
		fi = 0;
		char_type_t const * const bgn = path.constData();
		char_type_t const * const end = bgn + path.size() + 1;

		char_type_t const * left = bgn;
		char_type_t const * right = bgn;
		node_t const * level = root;
		for (; right < end; ++right)
		{
			char_type_t const c = *right;
			if ((c == ':' && right == bgn + 1) || c == '/' || c == '\\' || c == '\0' || right + 1 == end)
			{
				if (right - left > 0)
				{
					level = node_t::node_child_fast(level, left, right);
					if (level == 0)
						return 0; // node not in tree
					fi = &level->data;
				}
				if (right + 1 < end)
					left = right + 1;
			}
		}
		return level;
	}

	void set_state_to_childs (node_t * node, filter_t const & data)
	{
		node = node->children;
		while (node)
		{
			node->data = data;
			set_state_to_childs(node, data);
			node = node->next;
		}
	}

	void set_state_to_childs (std::string const & path, filter_t const & data)
	{
		tokenizer_t tok(path, separator);
		node_t * level = root;
	   	tokenizer_t::const_iterator it = tok.begin(), ite = tok.end();
		while (it != ite)
		{
			level = node_t::node_child(level, *it);
			if (level == 0)
				return;

			++it;
			if (it == ite)
			{
				level->data = data;
				set_state_to_childs(level, data);
			}
		}
	}

	void set_state_to_parents (node_t * node, filter_t const & data)
	{
		while (node = node->parent)
		{
			node->data = data;
			set_state_to_parents(node, data);
		}
	}
/*
	void set_state_to_topdown (std::string const & path, E_NodeStates fw_state, E_NodeStates rev_state)
	{
		tokenizer_t tok(path, separator);
		node_t * level = root;
	   	tokenizer_t::const_iterator it = tok.begin(), ite = tok.end();
		while (it != ite)
		{
			level = node_t::node_child(level, *it);
			if (level == 0)
				return;

			++it;
			if (it == ite)
			{
				level->data.m_state = fw_state;
				set_state_to_childs(level, fw_state);
				set_state_to_parents(level, rev_state);
			}
		}
	}
*/
	void reassemble_path (std::vector<node_t *> const & nodes, string_type_t & path) const
	{
		for (size_t i = 0, ie = nodes.size(); i < ie; ++i)
		{
			path.append(nodes[i]->key);
			if (i + 1 < ie)
				path.append("/");
		}
	}

	template <class ArchiveT>
	inline void save (ArchiveT & a, unsigned const /*version*/) const
	{
		a.register_type(static_cast<node_t *>(NULL));
		a & boost::serialization::make_nvp("root", root);
	}
	 
	template <class ArchiveT>
	inline void load (ArchiveT & a, unsigned const /*version*/)
	{
		a.register_type(static_cast<node_t *>(NULL));
		a & boost::serialization::make_nvp("root", root);
	}

	BOOST_SERIALIZATION_SPLIT_MEMBER()
	 
	void dump_filter_impl (node_t const * node, std::vector<node_t *> & nodes, string_type_t & output) const
	{
		if (node && node->children)
		{
			node_t * child = node->children;

			while (child)
			{
				node_t * current = child;
				nodes.push_back(current);
				{

					string_type_t path;
					path.reserve(128);
					reassemble_path(nodes, path);
					output.append("\n");

					if (current->data.m_state == 2 /*e_Checked*/)
						output += "X ";
					else if (current->data.m_state == 1 /*e_PartialCheck*/)
						output += "# ";
					else
						output += "- ";

					if (current->data.m_collapsed)
						output += "c ";
					else
						output += "E ";
					output.append(path);
					path.clear();
				}
				dump_filter_impl(current, nodes, output);
				nodes.pop_back();
				child = current->next;
			}
		}
	}

	void dump_filter (string_type_t & output) const
	{
		output.reserve(128);
		std::vector<node_t *> path;
		path.reserve(32);
		dump_filter_impl(root, path, output);
	}
};

