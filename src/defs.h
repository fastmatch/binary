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

#ifndef CLIENT_DEFS_H__
#define CLIENT_DEFS_H__

#include <iostream>
#include <iostream>     // std::cerr, std::endl
#include <iomanip>      // std::setw
#include <type_traits>
#include <functional>
#include <utility>
#include <time.h>
#include <src/utils.h>
#include <src/print.h>
 
#define KNRM  "\x1B[0m"  // normal
#define KRED  "\x1B[31m" // red
#define KGRN  "\x1B[32m" // green
#define KYEL  "\x1B[33m" // yellow
#define KBLU  "\x1B[34m" // blue
#define KMAG  "\x1B[35m" // magenta
#define KCYN  "\x1B[36m" // scyan
#define KWHT  "\x1B[37m" // white
#define RESET "\033[0m"  // reset
#define KGRNB  "\x1B[1;32m" // green


namespace fastapp
{
namespace callbacks
{
typedef std::function<bool () > is_func_t;
typedef std::function<void (const std::string & ) > set_id_t;
typedef std::function<void (const std::string &, const std::string &, const std::string &) > send_email_t;
}

}

namespace fastapp
{
namespace utils
{
    
enum PrintFlags : uint64_t
{
    DisablePrt    = 0,
    ClientHrtBeat = 1,
    ServerHrtBeat = 2,
            Trace = 4,
            Book  = 8,
         Lowlevel = 16,
         Ouch     = 32
};

inline std::ostream & formatPx (int64_t rate, std::ostream & strm)
{
    strm << std::dec << rate/100000 << "." << rate%100000;
    return strm;
}

inline std::ostream & formatQty(int64_t qty, std::ostream & strm)
{
    strm << std::dec << qty/100 << "." << qty%100;
    return strm;
}


inline std::ostream & formatTime(uint64_t microTime, std::ostream & strm)
{
    time_t rawtime = microTime/1000000LL;
    struct tm * tmif = localtime ( &rawtime );
        
    strm << std::setfill ('0') << std::setw(2)<< tmif->tm_mon <<"/" << std::setfill ('0') << std::setw(2) << tmif->tm_mday << " " << std::setfill ('0') << std::setw(2) << tmif->tm_hour <<":" << std::setfill ('0') << std::setw(2) <<tmif->tm_min << ":" << std::setfill ('0') << std::setw(2) <<tmif->tm_sec << "." << microTime%1000000LL;
    // "Update Time: " <<  
    return strm;
}

inline std::ostream & formatSec(time_t rawtime, std::ostream & strm)
{
    struct tm * tmif = localtime ( &rawtime );
        
    strm <<(tmif->tm_year + 1900) << "/" << std::setfill ('0') << std::setw(2) 
         << tmif->tm_mon  << "/"  << std::setfill ('0') << std::setw(2) 
         << tmif->tm_mday << " "  << std::setfill ('0') << std::setw(2) 
         << tmif->tm_hour << ":"  << std::setfill ('0') << std::setw(2) 
         << tmif->tm_min  << ":"  << std::setfill ('0') << std::setw(2) 
         << tmif->tm_sec;
    return strm;
}

template<typename E>
constexpr auto to_integral(E e) -> typename std::underlying_type<E>::type 
{
   return static_cast<typename std::underlying_type<E>::type>(e);
}

inline void print_impl(int64_t time, const char * data, int32_t size, const char * prefix)
{
    using namespace fastapp;
    FMPRINT( Micro(time) , " : " , prefix, Hex(data, size) , "' ; len=" , size , "\n");
}

inline void print_outgoing(int64_t time, const char * data, int32_t size)
{
    print_impl(time, data, size, "Outgoing :'");
}

inline void print_outgoing(int64_t time, const char * data, int32_t size, const char * prefix)
{
    std::string pre ("Outgoing :");
    pre += prefix;
    print_impl(time, data, size, pre.c_str());
}
inline void print_incoming(int64_t time, const char * data, int32_t size)
{
    print_impl(time, data,size, "Incoming packet:'");
}

template<typename T>
void print_impl(int64_t time, T &data, const char * prefix)
{
    print_impl(time, (const char *)&data, sizeof(T), prefix);
}

template<typename T>
void print_outgoing(int64_t time, T &data)
{
    print_outgoing(time, (const char *)&data, sizeof(T));
}

template<typename T>
void print_outgoing(int64_t time, T &data, const char * prefix)
{
    print_outgoing(time, (const char *)&data, sizeof(T), prefix);
}

template<typename T>
void print_incoming(int64_t time, T &data)
{
    print_incoming(time, (const char *)&data, sizeof(T));
}

} // utils
} //fastapp


#endif

