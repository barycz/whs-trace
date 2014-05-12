#pragma once
/*#include <tuple>

template <int N, int SizeN, typename TupleT>
struct do_recurse : do_recurse<N + 1, SizeN, TupleT>
{
	template <typename FunctionT>
	void operator () (FunctionT & f, TupleT & t)
	{
		f(std::get<N>(t));
		do_recurse<N + 1, TSize, TupleT>::operator() (f, t);
	}
};

template <int N, typename TupleT>
struct do_recurse<N, N, TupleT>
{
	template <typename FunctionT>
	void operator () (FunctionT & f, TupleT & t) {}
};

template <typename Function, typename... Args>
void recurse (Function & f, std::tuple<Args...> & t)
{
	do_recurse<0, sizeof...(Args), std::tuple<Args...>>() (f, t);
}*/
