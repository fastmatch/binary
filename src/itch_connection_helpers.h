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

#ifndef ITCH_CONNECTION_HELPERS_H__
#define ITCH_CONNECTION_HELPERS_H__

#include <src/itch_connection.h>

namespace  fastapp
{

template<typename Connection> Connection & getConnection(::fastapp::App &);

template<>
inline fastapp::TcpConnection & getConnection<fastapp::TcpConnection>(::fastapp::App & app)
{
    typedef tcp_itch_connection_t conn_t;
    fastapp::TcpConnection & session = app.getTcpConnection(0);
    conn_t::initProcessor(session);
    conn_t::processor_t * prc = conn_t::getProcessor(session);
    conn_t::initConsoleLogging(prc, session);
    return session;
}

template<>
inline fastapp::UdpConnection & getConnection<fastapp::UdpConnection>(::fastapp::App &  app)
{
    typedef udp_itch_connection_t conn_t;
    fastapp::UdpConnection & session = app.getUdpConnection(0);
    conn_t::initProcessor(session);
    conn_t::processor_t * prc = conn_t::getProcessor(session);
    conn_t::initConsoleLogging(prc, session);

    return session;
}

inline ::fastapp::soupbin::Dispatcher & getSoupbinDispatcher(::fastapp::TcpConnection & session)
{
    typedef tcp_itch_connection_t conn_t;
    return conn_t::getSoupbinDispatcher(session);
}

inline ::fastapp::soupbin::Dispatcher & getSoupbinDispatcher(::fastapp::UdpConnection & session)
{
    typedef udp_itch_connection_t conn_t;
    return conn_t::getSoupbinDispatcher(session);
}

inline ::fastapp::itch::Dispatcher & getItchDispatcher(::fastapp::TcpConnection & session)
{
    typedef tcp_itch_connection_t conn_t;
    return conn_t::getItchDispatcher(session);
}

inline ::fastapp::itch::Dispatcher & getItchDispatcher(::fastapp::UdpConnection & session)
{
    typedef udp_itch_connection_t conn_t;
    return conn_t::getItchDispatcher(session);
}

inline tcp_itch_connection_t::processor_t * getProcessor(::fastapp::TcpConnection & session)
{
    typedef tcp_itch_connection_t conn_t;
    return conn_t::getProcessor(session);
}

inline udp_itch_connection_t::processor_t * getProcessor(::fastapp::UdpConnection & session)
{
    typedef udp_itch_connection_t conn_t;
    return conn_t::getProcessor(session);
}


}

#endif

