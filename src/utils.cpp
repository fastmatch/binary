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

#include <src/defs.h>
#include <src/utils.h>

#include <sys/time.h>
#include <time.h>
#include <stdio.h>
#include <string.h>

namespace fastapp
{
namespace utils
{

int64_t getTimeNow()
{
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);

    return t.tv_sec * 1000000LL + t.tv_nsec/1000LL;
}

int64_t getTimeOfDay()
{
    struct timeval t;
    gettimeofday( &t, NULL );

    return t.tv_sec * 1000000LL + t.tv_usec;
}


void hex (std::ostream& os, const char* addr, size_t len)
{
    static const char* hexdigits = "0123456789abcdef";

    for (const char* p = addr; p != addr + len; ++p) {
        if ((unsigned)(*p + 1) <= 256 && std::isgraph (*p)) 
        {
            char buf[] = { ' ', ' ', *p};
            os.write (buf, 3);
            //os.put (*p);
        } else {
            char buf[] = { ' ', ' ', 0, 0 };
            unsigned char c = static_cast<unsigned char> (*p & 0xff);
            buf[2] = hexdigits [(c >> 4) & 0xf];
            buf[3] = hexdigits [c & 0xf];
            os.write (buf, 4);
        }
    }
}

void hex (const char* addr, size_t len, std::string& escaped)
{
    static const char* hexdigits = "0123456789abcdef";

    // Calculate exact length of result with escapes
    size_t result_len = len;

    for (const char* p = addr; p != addr + len; ++p) {
        if ((unsigned)(*p + 1) > 256 || !std::isgraph (*p)) {
            result_len += 3;
        }
    }

    escaped.reserve (result_len); // + result_len/8

    for (const char* p = addr; p != addr + len; ++p) {
        if ((unsigned)(*p + 1) <= 256 && std::isgraph (*p)) {
            escaped.push_back (*p);
        } else {
            char buf[] = { '0', 'x', 0, 0 };
            unsigned char c = static_cast<unsigned char> (*p & 0xff);
            buf[2] = hexdigits [(c >> 4) & 0xf];
            buf[3] = hexdigits [c & 0xf];
            escaped.append (buf, 4);
        }
    }
}


void hex_simple (std::ostream& os, const char* addr, size_t len)
{
    static const char* hexdigits = "0123456789abcdef";

    os.write ("0x", 2);
    for (const char* p = addr; p != addr + len; ++p) 
    {
        unsigned char c = static_cast<unsigned char> (*p & 0xff);
        char buf[] = { 0, 0 };
        buf[0] = hexdigits [(c >> 4) & 0xf];
        buf[1] = hexdigits [c & 0xf];
        os.write (buf, 2);
    }
}


void split(std::vector<std::string> & out, char * str, const char * delimiter )
{
    const char * tok = 0;
    char * saveptr;

    if ( (tok = strtok_r(str, delimiter, &saveptr))  == 0) return;
    out.push_back(tok);

    while ( (tok = strtok_r(NULL, delimiter, &saveptr))  != 0 )
    {
        out.push_back(tok);
    }
}

void split(std::vector<std::string> & out, const std::string & str, const char * delimiter)
{
    split(out, const_cast<char *>(str.c_str() ), delimiter);
}

} // End of utils namespace
} // End of fastapp namespace
