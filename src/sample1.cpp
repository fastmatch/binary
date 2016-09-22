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

// standard
#include <iostream>
#include <vector>
#include <queue>
#include <chrono>
#include <thread>
#include <string>


// fm
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
    // register some of the dispatcher callbacks 
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
    // print HB base on the value of trace flag
    dispatcher.registerAllOn
    (
        [&market] () {return market.IsSrvHbtOn()|| market.IsCliHbtOn() || market.IsTraceOn(); }
    );

    // set dispatcher for market connection
    // sipatcher responsible for handling all incoming messages
    
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

    market.registerOnConnect([](ConnectionBase * session)
    {
        int64_t tod = fastapp::Connection::timeOfDay();
        using namespace ::fastapp::sockets;
        Address local;
        if (0 == getLocalAddr( session->getSocket(), local) )
        {
            FMPRINT(::fastapp::Micro(tod), " Socket " , session->getSocket() , " is bound to local interface <" , local.ip , ":" ,  local.port ,   "> " , "\n");
            session->setLocalPort(local.port);
            if (session->getLocalIP().empty())
            {
                session->setLocalIP(local.ip);
            }
        }

        FMPRINT( KGRN, ::fastapp::Micro(tod), " Connected ITCH session: \""   , (session->getLocalIP().empty() ? local.ip : session->getLocalHost() + std::string("[") + local.ip + std::string("]")),":",session->getLocalPort(),"->",session->getRemoteHost(),":",session->getRemotePort() , "\" User:\"", session->getUserName(),"\" Socket: ",session->getSocket(), "\n", KNRM);
    }
    );

    market.registerOnDisconnect([](const ConnectionBase * session)
    {
        int64_t tod = fastapp::Connection::timeOfDay();

        FMPRINT( KGRN, ::fastapp::Micro(tod), " Disconnected ITCH session: \"", session->getLocalIP(),":",session->getLocalPort(),"->",session->getRemoteHost(),":",session->getRemotePort() ,"\" User:\"", session->getUserName(),"\" Socket: ",session->getSocket(), "\n", KNRM );
    }
    );
    
    Logout logout;
    populateLogout(logout);

    ClientHeartbeat hbc;
    populateHbc(hbc);

    // register callbacks
    registerCallbacks(market, dispatcher);

    // initiate synchronous connect for all connections created
    // connect call makes sense even for udp, 'cause it allows to use recv call without 
    // requirement to specify remote endpoint on every receive_from call

    market.setBlockingMode(app.getBlockingMode());
    market.setDelay(app.getSleep() );

    if (!market.setup())
    {
         return -2;
    }
 
    if (!market.connect()) 
    {
       // app.connect can be replaced with number of connection calls, see source for app.connect
       // also it is easier to make connection blocking, to simplify processing and only make it non blocking once connected if needed.
        return -1;
    }
    
    int64_t tod = fastapp::Connection::timeOfDay();

    // populate Login Message 
    Login login;
    populateLogin(login, market.getSessionID(), market.getSeqNum(), market.getUserName(), market.getPassword() );
    stampLogin(login, market.nextSequence());

    //submit Login request
    market.send(login, tod);
    FMPRINT( ::fastapp::Micro(tod), " sent login with seq=", market.lastSeq(), "\n"); 
   
    // exchange heartbeats with server and wait for Login confirmation or reject 
    while (market.getSessionID().empty())
    {
        market.run_once();
    }
    tod = fastapp::Connection::timeOfDay();
    FMPRINT( ::fastapp::Micro(tod), " SessionID=" , market.getSessionID(), "\n");

    tod = fastapp::Connection::timeOfDay();

    if (market.isConnected())
    {
        // exchange heartbeats, wait and disconnect
        for (int i = 0; i < 10; i ++)
        {
            market.run_once();
        }

        int64_t tod = ::fastapp::Connection::timeOfDay();
        FMPRINT( Micro(tod)); 
        stampLogout(logout, market.nextSequence());
        //send Logout message
        market.send(logout, tod);
        FMPRINT(" : send Logout, size : ", sizeof(logout) , "\n");

    }

    FMPRINT( "drop connection", "\n");
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
