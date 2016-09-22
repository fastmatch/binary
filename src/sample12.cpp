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

//standard
#include <iostream>
#include <vector>
#include <queue>
#include <chrono>
#include <thread>
#include <string>

//boost
//#include <boost/algorithm/string.hpp>
//#include <boost/lexical_cast.hpp>

//my
#include <src/defs.h>
#include <src/socketutils.h>
#include <src/dispatcher_defs.h>

#include <src/connection.h>
#include <src/itch_handlers.h>
#include <src/smtp.h>
#include <src/adapter.h>
#include <src/app.h>
#include <src/helpers.h>
#include <src/print.h>

namespace  fastapp
{

template<typename Connection> Connection & getConnection(::fastapp::App &);

template<>
inline fastapp::TcpConnection & getConnection<fastapp::TcpConnection>(::fastapp::App & app)
{
    return app.getTcpConnection(0);
}    

template<>
inline fastapp::UdpConnection & getConnection<fastapp::UdpConnection>(::fastapp::App &  app)
{
    return app.getUdpConnection(0);
}

template <typename Defs, typename Processor, typename Connection>
int run_market(::fastapp::App & app);

int run_tcp(::fastapp::App & app)
{
   return run_market<TcpDefs, itch_processor_t, fastapp::TcpConnection>(app);
}

int run_udp(::fastapp::App & app)
{
   return run_market<UdpDefs, udp_itch_processor_t, fastapp::UdpConnection>(app);
}


void registerCallbacks(auto & market, auto & dispatcher)
{
    // store sessionID
    dispatcher.registerSessionID
    (
        [&market] (const std::string & session)
        {return market.setSessionID(session); }
    );

    market.registerOnSend
    (
        [&dispatcher](const TcpLogin & login)
        {
            dispatcher.set(login);
        }
    );

    // print HB
    dispatcher.registerAllOn
    (
        [&market] () {return true; }
    );

    // set dispatcher for market connection
    market.setProcessor
    (
        [&dispatcher](void * ptr, int32_t size)
        {
            dispatcher.process(ptr, size);
        }
    );

    //set print flag to indicate what should be printed by default
    using namespace ::fastapp::utils;
    market.setPrintFlag(PrintFlags::ClientHrtBeat | PrintFlags::ServerHrtBeat | PrintFlags::Trace);

}

template <typename Defs, typename Processor, typename Connection>
int run_market(::fastapp::App & app)
{
    typedef typename Defs::ReqData         ReqData;
    typedef typename Defs::InstReqData     InstReqData;
    typedef typename Defs::Login           Login;
    typedef typename Defs::ClientHeartbeat ClientHeartbeat;
    typedef typename Defs::Logout          Logout;

    Processor dispatcher;
    auto & market = getConnection<Connection>(app);

    Logout logout;
    populateLogout(logout);

    ClientHeartbeat hbc;
    populateHbc(hbc);

    registerCallbacks(market, dispatcher);
   
    if (!app.connect())
    {
        return -1;
    }
    
    int64_t tod = fastapp::Connection::timeOfDay();

    // populate Login Message 
    Login login;
    populateLogin(login, market.getSessionID(), market.getSeqNum(), market.getUserName(), market.getPassword() );
    //setLogin(login, dispatcher);
    stampLogin(login, market.nextSequence());
    market.send(login, tod);
    FMPRINT( " sent login: " , market.lastSeq() , "\n");
   
    // heartbeat and wait for Login confirmation or reject 
    while (market.getSessionID().empty())
    {
        app.run_once();
    }
    
    tod = fastapp::Connection::timeOfDay();
    FMPRINT(::fastapp::Hex(tod), " SessionID=" , market.getSessionID() , "\n");

    if (market.isConnected())
    {
        // wait and disconnect
        for (int i = 0; i < 5; i ++)
        {
            app.run_once();
        }

        tod = ::fastapp::Connection::timeOfDay();
        FMPRINT( ::fastapp::Hex(tod) );

        stampLogout(logout, market.nextSequence());
        //send Logout
        market.send(logout, tod);

        FMPRINT( ::fastapp::Micro(tod), " : send Logout, size : " , sizeof(logout) , "\n");

        // wait and disconnect
        for (int i = 0; i < 3; i ++)
        {
            app.run_once();
        }
    }

    FMPRINT( "drop connection" , "\n");
    // disconnect
    market.disconnect() ;

    // disable connection
    market.disable() ;

    return 0;
}

int run(int argc, char * argv[])
{
    if (argc < 3)
    {
        fastapp::App::usage(argc, argv);
        return 0;
    }

    fastapp::App app;
    app.parseCmd(argc, argv);
    
    if (app.numOfTcpConnections() > 0)
    {
        return run_tcp(app);
    }
    else if (app.numOfUdpConnections() > 0)
    {
        return run_udp(app);
    }

    return -1;
}

}


void printException()
{
    try
    {
        throw;
    }
    catch(const ::fastapp::sockets::SocketError & ex)
    {
        FMPRINT( "Exception SocketError: '" , ex.what() , "' ; ec=" , ex.error_code() , "\n");
    }
    catch(const ::fastapp::sockets::SocketDisconnect &dex)
    {
        FMPRINT( "Exception SocketDisconnect: '" , dex.what() , KNRM , "' ; ec=" , dex.error_code() , "\n");
    }
    catch(const std::runtime_error &err)
    {
        FMPRINT( "Exception runtime_error: '" , err.what() , "'" , "\n");
    }
    catch(const std::exception &e)
    {
        FMPRINT( "Exception std::exception: '" , e.what() , "'" , "\n");
    }
    catch(...)
    {
        FMPRINT( "Unknown exception" , "\n");
    }

}

int main(int argc, char * argv[])
{
    try
    {
        return fastapp::run (argc,argv);
    }
    catch(...)
    {
        printException();
    }
    
    return -1;
}
