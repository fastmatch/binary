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
#include <src/print.h>

namespace  fastapp
{


template <typename Defs, typename Connection, typename Processor>
int run_market(::fastapp::App & app, Connection & market, Processor & p)
{
    typedef typename Defs::ReqData         ReqData;
    typedef typename Defs::InstReqData     InstReqData;
    typedef typename Defs::Login           Login;
    typedef typename Defs::ClientHeartbeat ClientHeartbeat;
    typedef typename Defs::Logout          Logout;
    
    
    Logout logout;
    populateLogout(logout);
    
    //p.init();
    
    market.setProcessor
    (
        [&p](void * ptr, int32_t size) 
        {
            p.process(ptr, size);    
        } 
    );

    p.registerHelpers
    (
        [&market] {return market.IsTraceOn();  },
        [&market] {return market.IsSrvHbtOn(); },
        [&market] {return market.IsCliHbtOn(); },
        [&market] {return market.IsBookOn();   },
        [&market] (const std::string & session) {return market.setSessionID(session); } ,
        [&market] (const std::string & source, const std::string & subject, const std::string & text) 
        {
            return market.sendMail(source, subject, text); 
        } 
    );
    
    bool enable_sub = false;
    
    p.registerInfoMessage
    (
        [&market, &enable_sub, &app] (const fastapp::itch::InstrumentInfoMessage & msg, uint64_t seq)
        {
            int64_t tod = ::fastapp::Connection::timeOfDay();
            using namespace fastapp::utils;
                    
            FMPRINT( ::fastapp::Micro(tod) , " - InstrumentInfoMessage:"\
            , msg.type      , ", sym='"\
            , std::string(msg.instSym, ::fastapp::itch::sym_size) , "', instrumentID='" \
            , msg.instID    , "', instrumentType='"\
            , msg.instType  , "', setlementDate='"\
            , Sec(msg.setlmentDate)  , "'" , "\n");
            
            if (!enable_sub) return;
                    
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
                
                stampSubscriptionRequest(seq_data, market.nextSequence());
                market.send(seq_data, tod);    
                if (market.IsTraceOn() && market.IsLowLevelOn())
                {
                    using namespace fastapp::utils;
                    print_outgoing(tod, seq_data);
                }
            }            
        }
    );
    
    
    p.registerEOFSession 
    ( 
        [&market, &logout] (const ::fastapp::soupbin::EndOfSession & m, int32_t size, int32_t seq)
        {
            int64_t tod = ::fastapp::Connection::timeOfDay();
            FMPRINT( ::fastapp::Micro(tod) , " : EndOfSession : " , m.type , ":" , size , " : seq (" , seq , ")" , "\n");
            
            stampLogout(logout, market.nextSequence());
            market.send(logout, tod);
            market.disable();
        }
    );
        
    if (!app.setup())
    {
        return -1;
    }
    
    
    Login login;
    populateLogin(login, market.getSessionID(), market.getSeqNum(), market.getUserName(), market.getPassword() );

    setLogin(login, p);
    
    ClientHeartbeat hbc;
    populateHbc(hbc);
    
    
    FMPRINT( "time_delta=" , market.time_delta , "\n");
    FMPRINT( "Login size:" , sizeof(Login) , "\n");

    int64_t tod = fastapp::Connection::timeOfDay();
    stampLogin(login, market.nextSequence());
    market.send(login, tod);
    FMPRINT( "sent login" , market.lastSeq() , "\n");
    
    tod = fastapp::Connection::timeOfDay();
//    print_outgoing(tod, login);

    std::vector<std::string> symbols;
    ::fastapp::utils::split(symbols, market.getSymbols(), ",;:" );
    
    FMPRINT( "split" , "\n");

    p.setStuckDelay(app.getStuckSec());
    
    if (symbols.size() == 0)
    {
        symbols.push_back("");
    }
    
    if (symbols[0].empty())
    {
        FMPRINT( KRED , "Sending empty subscription symbol" , KNRM , "\n");
    }
 
    {    
        ReqData seq_data;
        fastapp::itch::SubscriptionRequest  & req = getSubscriptionRequest(seq_data);
        populateSubscriptionRequest(req, market.getSessionID(), market.getSubscription(), "EUR/USD" );    
        stampSubscriptionRequest(seq_data, market.nextSequence());
        if (market.IsBookSub())
        {
            FMPRINT( "book sub" , "\n");
            market.send_recv(seq_data, tod);
        }

        tod = fastapp::Connection::timeOfDay();


        req.reqID      ++;
        req.updateType = '2'; // Trade
        if (market.IsTradeSub())
        {
            FMPRINT( "trade sub" , "\n");

            stampSubscriptionRequest(seq_data, market.nextSequence());            
            market.send_recv(seq_data, tod);

            req.actionType = '2'; // unsubscribe
            stampSubscriptionRequest(seq_data, market.nextSequence());
            market.send_recv(seq_data, tod);

            req.reqID ++;
            req.actionType = '1'; // subscribe
            stampSubscriptionRequest(seq_data, market.nextSequence());
            market.send_recv(seq_data, tod);
        }

        tod = fastapp::Connection::timeOfDay();
        
        stampLogout(logout, market.nextSequence());
        market.send_recv(logout, tod);
        FMPRINT( KGRN ,  "logout: " , market.lastSeq() , KNRM , "\n");
    }

    tod = fastapp::Connection::timeOfDay();
    enable_sub = market.listEnabled();
    
    stampLogin(login, market.nextSequence());
    market.send_recv(login, tod);
    FMPRINT( "sent login: " , market.lastSeq() , "\n");
    
    while (market.getSessionID().empty())
    {
        market.run_once();
    }
    
    FMPRINT( "\n" ,"SessionID=" , market.getSessionID() , "\n");
    tod = fastapp::Connection::timeOfDay();
    if (enable_sub)
    {
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
    }

    if (!enable_sub)
    {
        ReqData seq_data;
        fastapp::itch::SubscriptionRequest  & req = getSubscriptionRequest(seq_data);
        populateSubscriptionRequest(req, market.getSessionID(), market.getSubscription(), symbols[0] );    
        for(auto & sym : symbols)
        {
            if (sym.empty () ) continue;
            tod = fastapp::Connection::timeOfDay();

            ::strncpy(req.instSym, sym.c_str(), fastapp::itch::sym_size);
            FMPRINT( KGRN , "Subscribe for '" , sym , "' sessionID: '" , std::string(req.sessionID, fastapp::soupbin::session_size) , "' - " , KNRM);
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
            FMPRINT( KNRM , "\n");
        }   
    }

    tod = ::fastapp::Connection::timeOfDay();
    stampHeartbeat(hbc, market.nextSequence());
    market.send_recv(hbc, tod);
    FMPRINT( ::fastapp::Micro(tod) ,  " ; Out (ClientHeartbeat) before run; seq:" , market.lastSeq() , "\n");    
    
    //sleep(3);
    app.run();
    
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
        fastapp::TcpConnection & market = app.getTcpConnection(0);
        itch_processor_t p;

        return run_market<TcpDefs>(app, market, p);
    }
    else if (app.numOfUdpConnections() > 0)
    {
        fastapp::UdpConnection & market = app.getUdpConnection(0);
        udp_itch_processor_t p;
        return run_market<UdpDefs>(app, market, p);
    }

    return -1;
}

}

int main(int argc, char * argv[])
{
    try
    {
        return fastapp::run (argc,argv);
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
    
    return -1;
}
