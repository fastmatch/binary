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

#ifndef DISPATCHER_DEFS_H
#define	DISPATCHER_DEFS_H

#include <src/itch/itch.h>
#include <src/soupbin/soupbin.h>
#include <src/soupbin/dispatcher.h>
#include <src/soupbin/dispatcher_udp.h>
#include <src/itch/dispatcher.h>
#include <src/itch_handlers.h>
#include <src/adapter.h>

namespace fastapp
{
typedef ::fastapp::callbacks::ItchAdapter<::fastapp::soupbin::Dispatcher     , ::fastapp::itch::callbacks::Handlers> itch_processor_t;
typedef itch_processor_t                                                                                             tcp_itch_processor_t;
typedef ::fastapp::callbacks::ItchAdapter<::fastapp::soupbin_udp::Dispatcher , ::fastapp::itch::callbacks::Handlers> udp_itch_processor_t;

template <typename Login, typename Processor> 
void setLogin(Login & login, Processor & p )
{
    p.set(login);
}

template <typename Processor> 
void setLogin(TcpLogin & login, Processor & p )
{
    p.set(login);
}


template <typename Processor> 
void setLogin(UdpLogin & login, Processor & p )
{
    p.set(login.msg);
}
    
}

#endif	/* DISPATCHER_DEFS_H */

