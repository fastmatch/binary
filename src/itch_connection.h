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

#ifndef ITCH_CONNECTION_H__
#define ITCH_CONNECTION_H__

#include <src/connection.h>

namespace fastapp
{
template <typename DISP, typename PROTO>
struct ItchConnection : public TmplConnection<PROTO>
{
public:
    typedef typename ::fastapp::callbacks::ItchBareAdapter<DISP> processor_t;

    ::fastapp::itch::Dispatcher & getItchDispatcher()
    {
        processor_t * processor = getProcessor(*this);
        return processor->getItchDispatcher();
    }

    fastapp::soupbin::Dispatcher & getSoupbinDispatcher()
    {
        processor_t * processor = getProcessor(*this);
        return processor->getSoupbinDispatcher();
    }

    static fastapp::itch::Dispatcher & getItchDispatcher(ConnectionBase & session)
    {
        processor_t * processor = getProcessor(session);
        return processor->getItchDispatcher();
    }

    static fastapp::soupbin::Dispatcher & getSoupbinDispatcher(ConnectionBase & session)
    {
        processor_t * processor = getProcessor(session);
        return processor->getSoupbinDispatcher();
    }

    ItchConnection() : TmplConnection<PROTO> ()
    {
        initProcessor(*this);
    }

    void initConsoleLogging()
    {
        processor_t * processor = getProcessor(*this);
        initConsoleLogging(processor, *this);
    }

    processor_t * getProcessor()
    {
        processor_t * processor = getProcessor(*this);
        return processor;
    }

    static processor_t * getProcessor(::fastapp::ConnectionBase & session)
    {
       processor_t * processor = (processor_t *) session.getVoidProcessor().get();
       return processor;
    }

    static void initConsoleLogging( processor_t * dispatcher, ::fastapp::ConnectionBase & session)
    {
        // print HB
        dispatcher->registerAllOn
        (
            [] () {return true; }
        );

        //set print flag to indicate what should be printed by default
        using namespace ::fastapp::utils;
        session.setPrintFlag(PrintFlags::ClientHrtBeat | PrintFlags::ServerHrtBeat | PrintFlags::Trace | PrintFlags::Book);
    }

    static void initProcessor(::fastapp::ConnectionBase & session)
    {
        initProcessor(new processor_t, session);
    }

    static void initProcessor(processor_t * dispatcher, ::fastapp::ConnectionBase & session)
    {
        // set dispatcher for market connection
        session.setProcessor
        (
            [dispatcher](void * ptr, int32_t size)
            {
                dispatcher->process(ptr, size);
            }
        );

        session.attach
        (
            std::shared_ptr<void>
            ( dispatcher
            , [](void * p) { delete static_cast<processor_t *>(p); }
            )
        );

        // dont print anything
         dispatcher->registerAllOn
         (
             [] () {return false; }
         );

         //set print flag to indicate what should be printed by default
         using namespace ::fastapp::utils;
         session.setPrintFlag(0);
    }

};




template <typename DISP, typename PROTO>
struct ItchConnectionRef 
{
public:
    typedef typename ::fastapp::callbacks::ItchBareAdapter<DISP> processor_t;
    typedef ItchConnection<DISP, PROTO>  session_t;
    typedef TmplConnection<PROTO> base_session_t;

    ::fastapp::itch::Dispatcher & getItchDispatcher()
    {
        processor_t * processor = session_t::getProcessor(*ptr);
        return processor->getItchDispatcher();
    }

    fastapp::soupbin::Dispatcher & getSoupbinDispatcher()
    {
        processor_t * processor = session_t::getProcessor(*ptr);
        return processor->getSoupbinDispatcher();
    }

    ItchConnectionRef(base_session_t * p) : ptr(p)
    {
        if (p == 0) throw std::runtime_error("NULL ptr exception");
        session_t::initProcessor(*ptr);
    }

    ItchConnectionRef(base_session_t & p) : ptr(&p) 
    {
        session_t::initProcessor(*ptr);
    }

    ItchConnectionRef(const ItchConnectionRef & ref) : ptr(const_cast<base_session_t *>(ref.ptr) ) 
    {
    }

    ItchConnectionRef & operator = (const ItchConnectionRef & ref) 
    {
        ptr = const_cast<base_session_t *>(ref.ptr);
    }

    static ItchConnectionRef  construct(TmplConnection<PROTO> & session)
    {
        return ItchConnectionRef(&session);
    }

    void initConsoleLogging()
    {
        processor_t * processor = session_t::getProcessor(*ptr);
        initConsoleLogging(processor, *ptr);
    }

    processor_t * getProcessor()
    {
        processor_t * processor = session_t::getProcessor(*ptr);
        return processor;
    }

    TmplConnection<PROTO> & getSession() 
    {
        return *ptr;
    }

protected:
    TmplConnection<PROTO> * ptr;
};

typedef ::fastapp::callbacks::ItchBareAdapter<::fastapp::soupbin::Dispatcher>       tcp_britch_processor_t;
typedef ::fastapp::callbacks::ItchBareAdapter<::fastapp::soupbin_udp::Dispatcher>   udp_britch_processor_t;

typedef ItchConnection<::fastapp::soupbin::Dispatcher, ::fastapp::sockets::TCP>     tcp_itch_connection_t;
typedef ItchConnection<::fastapp::soupbin_udp::Dispatcher, ::fastapp::sockets::UDP> udp_itch_connection_t;

typedef ItchConnectionRef<::fastapp::soupbin::Dispatcher, ::fastapp::sockets::TCP>     tcp_itch_connection_ref_t;
typedef ItchConnectionRef<::fastapp::soupbin_udp::Dispatcher, ::fastapp::sockets::UDP> udp_itch_connection_ref_t;



}

#endif

