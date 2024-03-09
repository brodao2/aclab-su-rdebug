/*
Copyright 2024 Alan Cândido (brodao@gmail.com)

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

  http: //www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

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
