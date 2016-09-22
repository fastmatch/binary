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

#ifndef OUCH_HDRS_H
#define	OUCH_HDRS_H

#include <src/defs.h>

#include <src/soup_handlers.h>

#include <src/ouch/dispatcher.h>

#include <src/ostore.h>

namespace fastapp
{
   
namespace ouch
{
    
namespace callbacks
{

inline  int64_t sym_int(const char * sym)
{
    return *reinterpret_cast<int64_t *>((void *)sym);    
}

inline  int64_t sym_int(const std::string & sym)
{
    return sym_int(sym.c_str());
}

inline  const char * sym_cstr(const ccypair_int_t & sym)
{
    return (const char *)((void *)&(sym));
}

class Handlers : public ::fastapp::soupbin::callbacks::Handlers
{

public:

    Handlers (::fastapp::soupbin::Dispatcher & sbd, ::fastapp::ouch::Dispatcher & oud)
    : ::fastapp::soupbin::callbacks::Handlers(sbd)
    , ouchd(oud)
    , _store(0)
    , _trade_con(0)
    {
    }
    
    void registerCallbacks ()
    {
        registerSoupbinCallbacks () ;
        registerOuchCallbacks () ;
    }
 
    void registerOuchCallbacks () ;

    
    ::fastapp::callbacks::is_func_t                       IsOuchOn;
    
    void setStore(::fastapp::ouch::Ostore * store)
    {
        _store = store;
    }

    void setConnection(::fastapp::TcpConnection * trade_con)
    {
        _trade_con = trade_con;
    }

protected:
    ::fastapp::ouch::Dispatcher    & ouchd;
    ::fastapp::ouch::Ostore *      _store;
    ::fastapp::TcpConnection *     _trade_con;
};
    
void printRejectCode(eRejectReasonCode code);
void printErrorCode(eErrorCode errorCode);

}

}

}
#endif	/* SOUPBIN_HDRS_H */

