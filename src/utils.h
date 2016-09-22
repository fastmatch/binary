//
//    This file is part of Fastmatch binary market data and order flow examples.
//
//    Fastmatch binary market data and order flow examples are free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.
//
//    Fastmatch binary market data and order flow examples are distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with Fastmatch binary market data examples.  If not, see <http://www.gnu.org/licenses/>.
//

#ifndef __FASTAPP_UTILS_H__
#define __FASTAPP_UTILS_H__

#include <vector>
#include <string>
#include <ostream>
#include <iostream>


namespace fastapp
{
namespace utils
{
    // Returns current time in usecs from Jan 1, 1970
int64_t getTimeNow();

inline constexpr int64_t ticksPerSecond()
{
    return 1000000LL;
}

int64_t getTimeOfDay();

void split(std::vector<std::string> & out, char * str, const char delimiter[]);

void split(std::vector<std::string> & out, const std::string & str, const char delimiter[]);

void hex (std::ostream& os, const char* addr, size_t len);

void hex (const char* addr, size_t len, std::string& escaped);

void hex_simple (std::ostream& os, const char* addr, size_t len);

inline void hex_simple (std::ostream& os, const auto & val)
{
    const char * addr = (const char *)(void *)&val;
    size_t len = sizeof(val);
    hex_simple (os, addr, len) ;
}
 
}

}
#endif

