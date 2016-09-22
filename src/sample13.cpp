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

//fm
#include <src/defs.h>
#include <src/socketutils.h>
#include <src/dispatcher_defs.h>

#include <src/connection.h>
#include <src/itch_handlers.h>
#include <src/smtp.h>
#include <src/adapter.h>
#include <src/app.h>
#include <src/helpers.h>
#include <src/itch_connection.h>
#include <src/itch_connection_helpers.h>

namespace  fastapp
{

template <typename Defs, typename Connection>
int run_market(::fastapp::App & app);

int run_tcp(::fastapp::App & app)
{
   return run_market<TcpDefs, fastapp::TcpConnection>(app);
}

int run_udp(::fastapp::App & app)
{
   return run_market<UdpDefs, fastapp::UdpConnection>(app);
}

template <typename Defs, typename Connection>
int run_market(::fastapp::App & app)
{
    typedef typename Defs::ReqData         ReqData;
    typedef typename Defs::InstReqData     InstReqData;
    typedef typename Defs::Login           Login;
    typedef typename Defs::ClientHeartbeat ClientHeartbeat;
    typedef typename Defs::Logout          Logout;

    // access connection

    auto & market = getConnection<Connection>(app);
    auto & soupbinDispatcher = getSoupbinDispatcher(market);
    auto & itchDispatcher    = getItchDispatcher(market);

    ::fastapp::itch::callbacks::Handlers callbacks(soupbinDispatcher, itchDispatcher, &market);
    callbacks.registerCallbacks(); //register callbacks
    callbacks.registerAllOn( [] () {return true; }); // enable console logging

    //parse symbols to subscribe for
    std::vector<std::string> symbols;
    ::fastapp::utils::split(symbols, market.getSymbols(), ",;:" );

    FMPRINT( "split" , "\n");
    if (symbols.size() == 0)
    {
        symbols.push_back("");
    }

    if (symbols[0].empty())
    {
        FMPRINT( KRED , "Sending empty subscription symbol" , KNRM , "\n");
    }
     
    Logout logout;
    populateLogout(logout);

    ClientHeartbeat hbc;
    populateHbc(hbc);

   
    if (!app.connect())
    {
        return -1;
    }
    
    int64_t tod = fastapp::Connection::timeOfDay();

    // populate Login Message 
    Login login;
    populateLogin(login, market.getSessionID(), market.getSeqNum(), market.getUserName(), market.getPassword() );
    //setLogin(login, *dispatcher);
    stampLogin(login, market.nextSequence());
    market.send(login, tod);
    FMPRINT( " sent login: " , market.lastSeq() , "\n");
   
    // heartbeat and wait for Login confirmation or reject 
    while (market.getSessionID().empty())
    {
        app.run_once();
    }
    
    tod = fastapp::Connection::timeOfDay();
    FMPRINT( Micro(tod) , " SessionID=" , market.getSessionID() , "\n");

    if (market.isConnected())
    {
        // populate subscription requests
        ReqData seq_data;
        fastapp::itch::SubscriptionRequest  & req = getSubscriptionRequest(seq_data);
        populateSubscriptionRequest(req, market.getSessionID(), market.getSubscription(), symbols[0] );
        for(auto & sym : symbols)
        {
            if (sym.empty () ) continue;
            tod = fastapp::Connection::timeOfDay();

            ::strncpy(req.instSym, sym.c_str(), fastapp::itch::sym_size);
            FMPRINT( KGRN , "Subscribe for '" , sym , "' sessionID: '" , std::string(req.sessionID, fastapp::soupbin::session_size) , "' - " , KNRM);

            req.reqID      ++;
            req.actionType = '1'; // subscribe
            req.updateType = '1'; // Book
            stampSubscriptionRequest(seq_data, market.nextSequence());
            //send subscription
            market.send(seq_data, tod);
        }


        // wait and disconnect
        for (int i = 0; i < 40; i ++)
        {
            app.run_once();
        }

        tod = ::fastapp::Connection::timeOfDay();
 
        FMPRINT( ::fastapp::Micro(tod) );

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
