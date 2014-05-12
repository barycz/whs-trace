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
#include <boost/call_traits.hpp>
#include <cstring>

template <class Key, class Data>
struct NNode
{
	typedef typename boost::call_traits<Key>::param_type key_param_t;
	Key 	key;
	Data	data;
	int		row;
	NNode * next;
	NNode * prev;
	NNode * parent;
	NNode * children;

	NNode () : key(), data(), row(0), next(0), prev(0), parent(0), children(0) { }
	explicit NNode (key_param_t k, typename boost::call_traits<Data>::param_type d)
		: key(k), data(d), row(0), next(0), prev(0), parent(0), children(0)
	{ }

	explicit NNode (char_type_t const * k_left, char_type_t const * k_right, typename boost::call_traits<Data>::param_type d)
		: key(k_left, k_right - k_left), data(d), row(0), next(0), prev(0), parent(0), children(0)
	{ }

	explicit NNode (NNode const & rhs)
		: key(rhs.key), data(rhs.data), row(0), next(0), prev(0), parent(0), children(0)
	{ }



	~NNode () { }

	static NNode * node_append (NNode * parent, NNode * node) { return node_insert_before(parent, NULL, node); }
	static NNode * node_prepend (NNode * parent, NNode * node) { return node_insert_before (parent, parent->children, node); }
	static NNode * node_insert_before (NNode * parent, NNode * sibling, NNode * node)
	{
		node->parent = parent;

		if (sibling)
		{
			if (sibling->prev)
			{
				node->prev = sibling->prev;
				node->prev->next = node;
				node->next = sibling;
				sibling->prev = node;
			}
			else
			{
				node->parent->children = node;
				node->next = sibling;
				sibling->prev = node;
			}
		}
		else
		{
			if (parent->children)
			{
				sibling = parent->children;
				while (sibling->next)
					sibling = sibling->next;
				node->prev = sibling;
				sibling->next = node;
				node->row = sibling->row + 1;
			}
			else
				node->parent->children = node;
		}
		return node;
	}

	static NNode * node_child (NNode * node, key_param_t k)
	{     
		node = node->children;
		while (node)
		{
			if (node->key == k)
				return node;
			node = node->next;
		}
		return 0;
	}

	static NNode * node_child (NNode * node, int r)
	{     
		node = node->children;
		while (node)
		{
			if (node->row == r)
				return node;
			node = node->next;
		}
		return 0;
	}

	static NNode const * node_child (NNode const * node, key_param_t k)
	{     
		node = node->children;
		while (node)
		{
			if (node->key == k)
				return node;
			node = node->next;
		}
		return 0;
	}

	static NNode const * node_child_fast (NNode const * node, char const * left, char const * right)
	{     
		node = node->children;
		while (node)
		{
			if (0 == strncmp(node->key.c_str(), left, right - left))
				return node;
			node = node->next;
		}
		return 0;
	}

	static NNode * node_child_fast (NNode * node, char const * left, char const * right)
	{     
		node = node->children;
		while (node)
		{
			if (0 == strncmp(node->key.c_str(), left, right - left))
				return node;
			node = node->next;
		}
		return 0;
	}

#if defined USE_QT_STRING
	static NNode const * node_child_fast (NNode const * node, char_type_t const * left, char_type_t const * right)
	{     
		QString const key = QString::fromRawData(left, right - left);
		node = node->children;
		while (node)
		{
			if (0 == QString::compare(node->key, key, Qt::CaseInsensitive))
				return node;
			node = node->next;
		}
		return 0;
	}
	static NNode * node_child_fast (NNode * node, char_type_t const * left, char_type_t const * right)
	{     
		QString const key = QString::fromRawData(left, right - left);
		node = node->children;
		while (node)
		{
			if (0 == QString::compare(node->key, key, Qt::CaseInsensitive))
				return node;
			node = node->next;
		}
		return 0;
	}
#endif


	static bool is_leaf (NNode * node) { return node->children == 0; }
	static bool is_root (NNode * node) { return node->parent == 0 && node->prev == 0 && node->next == 0; }

	static void node_destroy (NNode * root)
	{
		if (!is_root(root))
			node_unlink(root);
		nodes_free(root);
	}

	static void node_unlink (NNode * node)
	{ 
		if (node->prev)
			node->prev->next = node->next;
		else if (node->parent)
			node->parent->children = node->next;
		node->parent = NULL;
		if (node->next)
		{
			node->next->prev = node->prev;
			node->next = NULL;
		}
		node->prev = NULL;
	}

	static void nodes_free (NNode * node)
	{
		while (node)
		{     
			NNode * next = node->next;
			if (node->children)
				nodes_free(node->children);
			delete node;
			node = next;
		}
	}

	unsigned count_childs () const
	{
		unsigned count = 0;
		NNode const * child = children;
		while (child)
		{
			++count;
			child = child->next;
		}
		return count;
	}

	template <class ArchiveT>
	void save (ArchiveT & a, unsigned const) const
	{
		a.register_type(static_cast<NNode *>(NULL));
		a & boost::serialization::make_nvp("key", key);
		a & boost::serialization::make_nvp("data", data);
		unsigned count = count_childs();
		a & boost::serialization::make_nvp("count", count);

		NNode * child = children;
		for (size_t i = 0; i < count; ++i)
		{
			a & boost::serialization::make_nvp("child", child);
			child = child->next;
		}
	}

	template <class ArchiveT>
	void load (ArchiveT & a, unsigned const)
	{
		a.register_type(static_cast<NNode *>(NULL));
		a & boost::serialization::make_nvp("key", key);
		a & boost::serialization::make_nvp("data", data);
		unsigned count = 0;
		a & boost::serialization::make_nvp("count", count);

		NNode * * child = &children;
		NNode * prev = 0;
		for (int i = 0; i < static_cast<int>(count); ++i)
		{
			a & boost::serialization::make_nvp("child", *child);
			(*child)->parent = this;
			(*child)->prev = prev;
			prev = *child;
			(*child)->row = i;
			child = &(*child)->next;
		}
	}

	BOOST_SERIALIZATION_SPLIT_MEMBER()
};

