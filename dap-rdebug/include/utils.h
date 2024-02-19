#pragma once

#include <algorithm>
#include <string>
#include <sstream>
#include <regex>

template<typename InputIt>
std::string join(InputIt first,
	InputIt last,
	const std::string& separator = ", ",
	const std::string& concluder = "")
{
	if (first == last)
	{
		return concluder;
	}

	std::stringstream ss;
	ss << *first;
	++first;

	while (first != last)
	{
		//ss << separator;
		ss << *first;
		++first;
	}

	//ss << concluder;

	return ss.str();
}

//std::string toLower(std::string s)
//{
//	std::transform(s.begin(), s.end(), s.begin(),
//		[](unsigned char c) { return std::tolower(c); } // correct
//	);
//
//	return s;
//}
