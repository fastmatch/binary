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
#include <src/itch_connection.h>
#include <signal.h>

namespace  fastapp
{

template <typename Defs, typename Connection>
int run_market(::fastapp::App & app);

template<typename Connection> Connection getConnection(::fastapp::App &);

template<>
inline tcp_itch_connection_ref_t getConnection<tcp_itch_connection_ref_t>(::fastapp::App & app)
{
    fastapp::TcpConnection & tsession = app.getTcpConnection(0);
    return tcp_itch_connection_ref_t(tsession);
}

template<>
inline udp_itch_connection_ref_t getConnection<udp_itch_connection_ref_t>(::fastapp::App &  app)
{
    fastapp::UdpConnection & usession = app.getUdpConnection(0);
    return udp_itch_connection_ref_t(usession);
}

int run_tcp(::fastapp::App & app)
{
   return run_market<TcpDefs, tcp_itch_connection_ref_t>(app);
}

int run_udp(::fastapp::App & app)
{
   return run_market<UdpDefs, udp_itch_connection_ref_t>(app);
}


// init logging for all the different components
template<typename Defs>
void initCallbacks(auto & marketRef, auto & callbacks)
{
    callbacks.registerCallbacks(); //register callbacks

    auto * processor = marketRef.getProcessor();
    auto & market = marketRef.getSession();

    // register additional processor callbacks
    // helping to handle logging level and notification processing
    processor->registerHelpers
    (
        [&market] {return market.IsTraceOn();  },
        [&market] {return market.IsBookOn();   },
        [&market] (const std::string & source, const std::string & subject, const std::string & text)
        {
            return market.sendMail(source, subject, text);
        }
    );

    // register additional event handler callbacks
    // helping to handle logging level and notification processing
    callbacks.registerHelpers
    (
        [&market] {return market.IsTraceOn();  },
        [&market] {return market.IsSrvHbtOn(); },
        [&market] {return market.IsCliHbtOn(); },
        [&market] {return market.IsBookOn();   },
        [&market] (const std::string & source, const std::string & subject, const std::string & text)
        {
            return market.sendMail(source, subject, text);
        }
    );

    // define action in response to info message response
    // it is especial useful when subscribing for ALL symbols
    // because one subscription message
    processor->registerInfoMessage
    (
        [&market] (const fastapp::itch::InstrumentInfoMessage & msg, uint64_t seq)
        {
            int64_t tod = ::fastapp::Connection::timeOfDay();
            using namespace fastapp::utils;
            FMPRINT( ::fastapp::Hex(tod), " - InstrumentInfoMessage:" \
            , msg.type      , ", sym='" \
            , std::string(msg.instSym, ::fastapp::itch::sym_size) , "', instrumentID='" \
            , msg.instID    , "', instrumentType='" \
            , msg.instType  , "', setlementDate='" \
            , ::fastapp::Sec(msg.setlmentDate) , "'" , "\n");

            ReqData seq_data;
            ::fastapp::itch::SubscriptionRequest  & req = getSubscriptionRequest(seq_data);
            populateSubscriptionRequest(req, market.getSessionID(), market.getSubscription(), std::string(msg.instSym, ::fastapp::itch::sym_size) );

            if (market.IsBookSub())
            {
                req.reqID      ++;
                req.actionType = '1'; // subscribe
                req.updateType = '1'; // Book

                stampSubscriptionRequest(seq_data, market.nextSequence());
                market.send(seq_data, tod);

                if (market.IsTraceOn() && market.IsLowLevelOn())
                {
                    using namespace fastapp::utils;
                    print_outgoing(tod, seq_data);
                }
            }

            if (market.IsTradeSub())
            {
                req.reqID ++;
                req.actionType = '1'; // subscribe
                req.updateType = '2'; // Trade
                stampSubscriptionRequest(seq_data, market.nextSequence());
                market.send(seq_data, tod);
                if (market.IsTraceOn() && market.IsLowLevelOn())
                {
                    using namespace fastapp::utils;
                    print_outgoing(tod, seq_data);
                }
            }

            if (market.IsMidpointSub())
            {
                req.reqID ++;
                req.actionType = '1'; // subscribe
                req.updateType = '3'; // mid point
                market.send(seq_data, tod);
                stampSubscriptionRequest(seq_data, market.nextSequence());
                if (market.IsTraceOn() && market.IsLowLevelOn())
                {
                    using namespace fastapp::utils;
                    print_outgoing(tod, seq_data);
                }
            }
 
        }
    );


    processor->registerEOFSession
    (
        [&market] (const ::fastapp::soupbin::EndOfSession & m, int32_t size, int32_t seq)
        {
            int64_t tod = ::fastapp::Connection::timeOfDay();
//             , ::fastapps::utils::getTimeOfDay()
            FMPRINT(::fastapp::Micro(tod), " : EndOfSession : " , m.type , ":" , size , " : seq (" , seq , ")" , "\n");

            typename Defs::Logout logout;
            populateLogout(logout);
            stampLogout(logout, market.nextSequence());

            market.send(logout, tod);
            market.disable();
        }
    );

    //set print flag to indicate what is printed
    using namespace ::fastapp::utils;
    market.setPrintFlag(/*PrintFlags::ClientHrtBeat | PrintFlags::ServerHrtBeat | PrintFlags::Trace | */PrintFlags::Book );

    // set update flag to enable Book subscription
    if (market.getUpdateFlag() == 0)
    {    market.setUpdateFlag((uint64_t)UpdateFlags::Book);    }
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

    auto marketRef = getConnection<Connection>(app);
    auto & market    = marketRef.getSession();

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

    ::fastapp::itch::callbacks::Handlers callbacks(marketRef.getSoupbinDispatcher(), marketRef.getItchDispatcher(), market);

    initCallbacks<Defs>(marketRef, callbacks);

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
    ::fastapp::utils::formatTime(tod, std::cerr);
    FMPRINT( " SessionID=" , market.getSessionID() , "\n");

    if (market.isConnected())
    {
        // populate info requests
        InstReqData sbListReq;
        fastapp::itch::InstrumentListRequest & listRequest = getInstrumentListRequest(sbListReq);

        for(auto & sym : symbols)
        {
            if (sym.empty () ) continue;

            populateListRequest(listRequest, market.getSessionID(), sym );
            FMPRINT( KGRN , "List request for '" , sym , "' sessionID: '" , std::string(listRequest.sessionID, fastapp::soupbin::session_size) , "' - " , KNRM , "\n");;
            stampListRequest(sbListReq, market.nextSequence());
            market.send_recv(sbListReq, tod);
            if (market.IsTraceOn() && market.IsLowLevelOn())
            {
                using namespace fastapp::utils;
                print_outgoing (tod, sbListReq);
            }
        }

        app.run ();

    }

    FMPRINT( "Exiting.." , "\n");

    return 0;
}

static ::fastapp::App app;

static void hdl (int sig)
{
    app.terminate();
}

int run(int argc, char * argv[])
{
    if (argc < 3)
    {
        fastapp::App::usage(argc, argv);
        return 0;
    }

    // reg sig handler
    struct sigaction act;
    memset (&act, '\0', sizeof(act));
    act.sa_handler = &hdl;
    if (sigaction(SIGTERM, &act, NULL) < 0)
    {
        perror ("sigaction");
        return 1;
    }
    if (sigaction(SIGINT, &act, NULL) < 0)
    {
        perror ("sigaction");
        return 1;
    }

    app.setName("SA5");
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
        FMPRINT( "Exception runtime_error: '" , err.what() , "'\n");
    }
    catch(const std::exception &e)
    {
        FMPRINT( "Exception std::exception: '" , e.what() , "'\n");
    }
    catch(...)
    {
        FMPRINT( "Unknown exception\n");
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
