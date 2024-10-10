#pragma once

namespace imf::core
{

template<typename Iterator>
using iterator_range = boost::iterator_range<Iterator>;

template<typename ...Args>
auto make_iterator_range(Args&& ...args)
{
	return boost::make_iterator_range(std::forward<Args>(args)...);
}

}
