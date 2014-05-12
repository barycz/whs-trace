#pragma once
#include <vector>
#include <memory>
#include <boost/call_traits.hpp>
#include <functional>
#include <algorithm>
#include "hash.h"

template <typename KeyT>
struct ItemInfo
{
	unsigned m_ref_count;
	unsigned m_hash;
	KeyT m_key;

	ItemInfo () : m_ref_count(0), m_hash(0) { }

	ItemInfo (typename boost::call_traits<KeyT>::param_type key)
		: m_ref_count(0), m_hash(static_cast<unsigned>(hash<KeyT>()(key))), m_key(key) { }

	friend bool operator< (ItemInfo const & lhs, ItemInfo const & rhs)
	{
		return rhs.m_ref_count < lhs.m_ref_count; // @NOTE: reverse order rhs < lhs
	}

	friend bool operator== (ItemInfo const & lhs, ItemInfo const & rhs)
	{
		return lhs.m_key == rhs.m_key;
	}

	template <class ArchiveT>
	void serialize (ArchiveT & ar, unsigned const version)
	{
		ar & boost::serialization::make_nvp("ref_count", m_ref_count);
		ar & boost::serialization::make_nvp("hash", m_hash);
		ar & boost::serialization::make_nvp("key", m_key);
	}
};

template <typename KeyT>
struct History
{
	unsigned m_key_limit;
	int m_current_item;
	typedef KeyT key_t;
	typedef std::vector<ItemInfo<key_t> > dict_t;
	typedef typename dict_t::iterator it_t;
	typedef typename dict_t::const_iterator const_it_t;
	dict_t m_dict;

	History (unsigned item_limit)
		: m_key_limit(item_limit)
		, m_current_item(-1)
	{ 
		m_dict.reserve(m_key_limit);
	}

	void sort () { std::sort(m_dict.begin(), m_dict.end()); }

	void insert (typename boost::call_traits<KeyT>::param_type key)
	{
		unsigned const key_hash = static_cast<unsigned>(hash<KeyT>()(key));
		for (size_t i = 0, ie = m_dict.size(); i < ie; ++i)
		{
			if (m_dict[i].m_hash == key_hash && m_dict[i].m_key == key)
			{
				++m_dict[i].m_ref_count;
				return;
			}
		}

		sort();

		// item not found, replace another
		if (m_dict.size() >= m_key_limit)
			m_dict.pop_back();
		m_dict.push_back(ItemInfo<KeyT>(key));
	}

	void insert_no_refcount (typename boost::call_traits<KeyT>::param_type key)
	{
		if (!find(key))
			m_dict.push_back(ItemInfo<KeyT>(key));
	}

	bool find (typename boost::call_traits<KeyT>::param_type key)
	{
		unsigned const key_hash = static_cast<unsigned>(hash<KeyT>()(key));
		for (size_t i = 0, ie = m_dict.size(); i < ie; ++i)
			if (m_dict[i].m_hash == key_hash && m_dict[i].m_key == key)
				return true;
		return false;
	}

	void remove (typename boost::call_traits<KeyT>::param_type key)
	{
		m_dict.erase(std::remove(m_dict.begin(), m_dict.end(), key), m_dict.end());
	}

	template <class ArchiveT>
	void serialize (ArchiveT & ar, unsigned const version)
	{
		ar & boost::serialization::make_nvp("dict", m_dict);
		ar & boost::serialization::make_nvp("current_item", m_current_item);
	}

	size_t size () const { return m_dict.size(); }
	KeyT const & operator[] (size_t i) const { return m_dict[i].m_key; }
};

