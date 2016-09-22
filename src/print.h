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

#ifndef _FAST_FMPRINT_H__
#define _FAST_FMPRINT_H__

#include <cstdint>
#include <ctime>
#include <src/utils.h>

namespace fastapp
{


#pragma pack(push, 1)

struct TIME32
{
    int32_t t;
    explicit TIME32(int32_t tim) : t(tim)
    {}

    friend std::ostream& operator<<(std::ostream& os, const TIME32& tim);
};

struct TIME64
{
    int64_t t;
    explicit TIME64(int64_t tim) : t(tim)
    {}

    friend std::ostream& operator<<(std::ostream& os, const TIME64& tim);
};


inline std::ostream& operator << (std::ostream& os, const TIME32 & tim)
{
    
    return os;
}

inline std::ostream& operator << (std::ostream& os, const TIME64 & tim)
{
    
    return os;
}

struct Rate 
{
    int64_t rate;
    explicit Rate(int64_t r) : rate(r) {}

    friend  std::ostream& operator << (std::ostream& os, const Rate & r);
};

inline std::ostream& operator << (std::ostream& os, const Rate & r)
{
    os << std::dec << std::setfill('0') << std::setw(10) << r.rate/100000 <<  "." <<  std::setfill('0') << std::setw(5) <<  r.rate%100000;
    return os;
}


struct Qty
{
    int64_t qty;
    explicit Qty(int64_t q) : qty(q) {}
    friend std::ostream& operator << (std::ostream& os, const Qty & q);
   
};

inline std::ostream& operator << (std::ostream& os, const Qty & q)
{
    os << std::dec << std::setfill('0') << std::setw(10) << q.qty/100 << "." << std::setfill('0') << std::setw(2) << q.qty%100;
//    os << std::dec << q.qty/100 << "." << q.qty%100;

    return os;
}


struct Sec
{
    time_t sec;
    explicit Sec(int64_t s) : sec(s) {}

    friend  std::ostream& operator << (std::ostream& os, const Sec & sec);

};

inline std::ostream & operator << (std::ostream & os, const Sec & sec)
{
    struct tm * tmif = localtime ( &sec.sec );

    os   <<(tmif->tm_year + 1900) << "/" << std::setfill ('0') << std::setw(2)
         << tmif->tm_mon  << "/"  << std::setfill ('0') << std::setw(2)
         << tmif->tm_mday << " "  << std::setfill ('0') << std::setw(2)
         << tmif->tm_hour << ":"  << std::setfill ('0') << std::setw(2)
         << tmif->tm_min  << ":"  << std::setfill ('0') << std::setw(2)
         << tmif->tm_sec;
    return os;
}


struct Micro
{
    uint64_t micro;
    explicit Micro(uint64_t m) : micro(m) {};
    friend  std::ostream& operator << (std::ostream & os, const Micro & m);
};

inline std::ostream & operator << (std::ostream & os, const Micro & m)
{
    time_t rawtime = m.micro/1000000LL;
    struct tm * tmif = localtime ( &rawtime );

    os << std::setfill ('0') << std::setw(2)<< tmif->tm_mon <<"/" << std::setfill ('0') << std::setw(2) << tmif->tm_mday << " " << std::setfill ('0') << std::setw(2) << tmif->tm_hour <<":" << std::setfill ('0') << std::setw(2) <<tmif->tm_min << ":" << std::setfill ('0') << std::setw(2) <<tmif->tm_sec << "." << m.micro%1000000LL;

    return os;
}

struct Hex
{
    void * ptr;
    size_t size;
    bool   pretty;
    Hex(void * p , size_t s) : ptr(p), size(s), pretty(true) {}
    Hex(const void * p , size_t s) : ptr((void *)p), size(s), pretty(true) {}
    Hex(const auto & e) : ptr((void *)&e), size(sizeof(e)), pretty(false) {}
};

inline std::ostream & operator << (std::ostream & os, const Hex & h)
{
   if(h.pretty) 
   {
       ::fastapp::utils::hex(os, (const char *)(h.ptr), h.size); 
   }
   else 
   {
       ::fastapp::utils::hex_simple(os, (const char *)(h.ptr), h.size); 
   }

   return os;
}

#pragma pack(pop)

inline int print()
{
    return 0;
}
template <typename T>
inline int print(const T & t)
{
    std::cerr << t;
    return 0;
}

template <typename T, typename ...TYPES>
inline int print(const T & t, TYPES ... types)
{
    print<T>(t);
    return print (types...);
}

//#define FMPRINT_PATH(p) print.h

#ifndef FMPRINT // 1

    #ifdef NOFMPRINT  // 2                    // print

        #define FMPRINT(args...)

    #else

       #if defined(FMPRINT_PATH) // 3 

           #include FMPRINT_PATH(print)

       #else  // pp     // 3

           #define FMPRINT(args...) ::fastapp::print(args)   // print

       #endif // 3

    #endif // 2

#endif // 1


}

#endif


