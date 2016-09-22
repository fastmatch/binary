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
#include <sstream>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>

// fm
#include <src/defs.h>
#include <src/socketutils.h>
#include <src/dispatcher_defs.h>

#include <src/connection.h>
#include <src/socketutils.h>
#include <src/itch_handlers.h>
#include <src/ouch_handlers.h>
#include <src/smtp.h>
#include <src/adapter.h>
#include <src/app.h>
#include <src/helpers.h>
#include <src/ostore.h>
#include <src/print.h>

namespace  fastapp
{
    
typedef ::fastapp::callbacks::OuchAdapter<::fastapp::soupbin::Dispatcher, ::fastapp::ouch::callbacks::Handlers> ouch_processor_t;

bool precondition(::fastapp::TcpConnection& trade_con, const ::fastapp::itch::Book & book)
{
    using namespace ::fastapp;

    if (book.getSymbol().empty())
    {
        int64_t now = fastapp::timeOfDay() ;
        using namespace ::fastapp::utils;
        FMPRINT( KYEL ,  ::fastapp::Micro(now) , " Empty symbol/book id " , book.getSymbol(), "' bookId: '" , book.getInstanceID() , "' Buy :" , Qty(book.topAsk()->qty) , ", ", Rate (book.topAsk()->rate) , " ; Sell:" , ::fastapp::Qty(book.topBid()->qty), ", ", ::fastapp::Rate(book.topBid()->rate) , " ] ", KNRM , "\n");

        return false;
    }

    if (book.topAsk() == 0 || book.topBid() == 0)
    {
        if (trade_con.IsTraceOn() && trade_con.IsLowLevelOn())
        {
            int64_t now = fastapp::timeOfDay() ;
            using namespace ::fastapp::utils;
            FMPRINT( KYEL  , ::fastapp::Micro(now) , " One sided or empty book for symbol/book id " , book.getSymbol()\
            , "' bookId: '" , book.getInstanceID() \
            , " topAsk=" ,(book.topAsk() == 0 ? "=empty": "=avail" ), ", topBid=" \
            , (book.topBid()==0 ? "=empty" : "=avail") , KNRM , "\n");
        }
        return false;
    }

    if (trade_con.IsTraceOn() && trade_con.IsLowLevelOn() && trade_con.IsOuchOn())
    {
        using namespace fastapp;

        int64_t now = fastapp::timeOfDay() ;
        using namespace ::fastapp::utils;
        FMPRINT( KYEL  , Micro(now) , " Thinking of firing '" , book.getSymbol()\
        , "' bookId: '" , book.getInstanceID() \
        , "' Buy :"  , Qty(book.topAsk()->qty) \
        , ", "       , Rate (book.topAsk()->rate) \
        , " ; Sell:" , Qty(book.topBid()->qty) \
        , ", "       , Rate (book.topBid()->rate) \
        , " ] "      , KNRM , "\n");
    }

    return true;
}

void populate(::fastapp::ouch::ReplaceOrder& rpl_buy, ::fastapp::ouch::NewOrder& no, ::fastapp::ouch::Order & ord)
{
    using namespace ::fastapp::ouch;
    rpl_buy.ccypair =  no.ccypair;;
    rpl_buy.clordid =  no.clordid;
    rpl_buy.origclordid = ord.clordid;
    rpl_buy.qty     = 2*(no.qty/3);
    rpl_buy.rate    = no.rate;
}

void populateBuy(const ::fastapp::itch::Book& book, ::fastapp::ouch::NewOrder& buy)
{
    using namespace ::fastapp::ouch;
    buy.ccypair = ::fastapp::ouch::callbacks::sym_int(book.getSymbol());
    //((char*)((void *)&buy.ccypair))[7] = 0;
    buy.orderType = eOrderType::Limit;
    buy.side = eSide::Buy;
    buy.qty = book.topAsk()->qty;
    int64_t now = fastapp::timeOfDay() ;
    buy.minqty = now % 3 == 1 ? 10000000 : 0; // 100.000.
    buy.rate = book.topAsk()->rate;
    buy.tif = eTimeInForce::Ioc;
}


void populateSell(const ::fastapp::itch::Book& book, ::fastapp::ouch::NewOrder& sell)
{
    using namespace ::fastapp::ouch;
    sell.ccypair = ::fastapp::ouch::callbacks::sym_int(book.getSymbol());
    //((char*)((void*)&sell.ccypair))[7] = 0;
    sell.orderType = eOrderType::Limit;
    sell.side = eSide::Sell;
    sell.qty = book.topBid()->qty;
    int64_t now = fastapp::timeOfDay() ;
    sell.minqty = now % 3 == 1 ? 10000000 : 0; // 100.000.
    sell.rate = book.topBid()->rate;
    sell.tif = eTimeInForce::Ioc;
}

void populateDayBuy(const ::fastapp::itch::Book& book, ::fastapp::ouch::NewOrder& buy)
{
    using namespace ::fastapp::ouch;
    buy.ccypair = ::fastapp::ouch::callbacks::sym_int(book.getSymbol());
    //((char*)((void *)&buy.ccypair))[7] = 0;
    buy.orderType = eOrderType::Limit;
    buy.side = eSide::Buy;
    auto quote = book.botomBid();
    buy.qty = quote->qty;
    int64_t now = fastapp::timeOfDay() ;
    buy.minqty = now % 3 == 1 ? 10000000 : 0; // 100.000.
    buy.rate = quote->rate;
    buy.tif = eTimeInForce::Day;
}


void populateDaySell(const ::fastapp::itch::Book& book, ::fastapp::ouch::NewOrder& sell)
{
    using namespace ::fastapp::ouch;
    sell.ccypair = ::fastapp::ouch::callbacks::sym_int(book.getSymbol());
    //((char*)((void*)&sell.ccypair))[7] = 0;
    sell.orderType = eOrderType::Limit;
    sell.side = eSide::Sell;
    auto quote = book.bottomAsk();
    sell.qty = quote->qty;
    int64_t now = fastapp::timeOfDay() ;
    sell.minqty = now % 3 == 1 ? 10000000 : 0; // 100.000.
    sell.rate = quote->rate;
    sell.tif = eTimeInForce::Day;
}
int64_t check4MissedQuote(int64_t now, const ::fastapp::itch::Book& book,
        ::fastapp::ouch::Ostore* pstore, ::fastapp::ouch::Slot* slot,
        ::fastapp::TcpConnection& trade_con)
{
    const ::fastapp::ouch::Quote* q = 0;
    if (pstore->peek(slot, now, q))
    {
        ::fastapp::itch::eSide itchSide = (
                (q->side == ::fastapp::ouch::eSide::Buy) ?
                        ::fastapp::itch::eSide::Offer :
                        ::fastapp::itch::eSide::Bid);

        if (q != 0 && book.find_by_rate(10000000, q->rate, itchSide) != 0) // quote >= 100K
        {
            std::stringstream ss;
            ss <<  "Quote:<" << slot->sym << "," << slot->instid << "> "
                    << q->qty / 100 << "." << q->qty % 100 << "@"
                    << q->rate / 100000 << "." << q->rate % 100000
                    << " is on the book after " << now - q->time << "> micros"
                    << std::endl;
            // c.send_email(); //) process_delayed(slot.sym,q->time, *q);
            const std::string source = slot->sym;
            const std::string subject = "Missed quote : " + slot->sym;
            const std::string text = ss.str();
            FMPRINT( KRED , ::fastapp::Micro(now) , subject , text , KNRM , "\n");
            trade_con.sendMail(source, subject, text);
        }
        pstore->pop(slot, q);
    }
    return now;
}

int64_t report2BeSubmitedOrder(int64_t now, const ::fastapp::itch::Book& book,
        ::fastapp::TcpConnection& trade_con, NewReqData& buy_req,
        NewReqData& sell_req)
{
    if (trade_con.IsOuchOn() && trade_con.IsLowLevelOn())
    {
        using namespace ::fastapp::utils;
        using namespace ::fastapp::ouch;
        using namespace ::fastapp;

        FMPRINT( KGRN, ::fastapp::Micro(now)) ;

        NewOrder & buy = castOrderRequest(buy_req);
        NewOrder & sell = castOrderRequest(sell_req);

        using namespace fastapp;
        FMPRINT( " firing '" , book.getSymbol(), "' bookId: '" , book.getInstanceID() , "' clordid: '" , ::fastapp::Hex (buy.clordid ), "'|'" , buy.clordid , "': [Buy:", Qty(book.topAsk()->qty),", ", Rate(book.topAsk()->rate), ", ", Qty(sell.minqty), "], clordid: '" , Hex(sell.clordid ) , "'|'" , sell.clordid , "': [Sell:", Qty(book.topBid()->qty), ", " , Rate(book.topBid()->rate) , ", ", Qty(sell.minqty), KNRM , " ] " , "\n" );
    }
    return now;
}

int64_t report2BeSubmitedOrder(int64_t now, const ::fastapp::itch::Book& book,
        ::fastapp::TcpConnection& trade_con, NewReqData& buy_req)
{
    //FMPRINT( KRED , "15New" , "\n");
    if (trade_con.IsOuchOn() && trade_con.IsLowLevelOn())
    {

        using namespace ::fastapp::utils;
        using namespace ::fastapp::ouch;
        using namespace ::fastapp;

        FMPRINT( KGRN );

        NewOrder & buy = castOrderRequest(buy_req);

        FMPRINT( ::fastapp::Micro(now), " firing '" , book.getSymbol(), "' bookId: '" , book.getInstanceID(), "' clordid: '" , Hex(buy.clordid) , "'|'" , buy.clordid);

        ::fastapp::ouch::eSide side = buy.side;

        if (side == ::fastapp::ouch::eSide::Buy)
        {
            FMPRINT( "': [Buy:" , Qty(book.topAsk()->qty),", ", Rate(book.topAsk()->rate), ", ", Qty(buy.minqty), "] ", KNRM , " ] " , "\n");
        }
        else
        {
            FMPRINT( "': [Sell:", Qty(book.topBid()->qty), ", ", Rate(book.topBid()->rate), ", ", Qty(buy.minqty), KNRM , " ] " , "\n");
        }
    }    
    return now; 
}

int64_t report2BeSubmitedBuyRpl(int64_t now, const ::fastapp::itch::Book& book,
        ::fastapp::TcpConnection& trade_con,
         RplReqData& buy_req
)
{
    if (trade_con.IsOuchOn() && trade_con.IsLowLevelOn())
    {
        using namespace ::fastapp::utils;
        using namespace ::fastapp::ouch;
        using namespace ::fastapp;

        FMPRINT( KGRN, ::fastapp::Micro(now)," firing rpl '" , book.getSymbol(), "' bookId: '" , book.getInstanceID(), "' clordid: '" , Hex(castOrderRequest(buy_req).clordid) , "'|'" , castOrderRequest(buy_req).origclordid , "': [Buy:", Qty(book.topAsk()->qty), ", ", Rate(book.topAsk()->rate), KNRM , " ] " , "\n");
    }
    return now;
}

int64_t report2BeSubmitedSellRpl(int64_t now, const ::fastapp::itch::Book& book,
        ::fastapp::TcpConnection& trade_con,
         RplReqData& sell_req)
{
    if (trade_con.IsOuchOn() && trade_con.IsLowLevelOn())
    {
        using namespace ::fastapp::utils;
        using namespace ::fastapp::ouch;
        using namespace ::fastapp;

        FMPRINT( KGRN, ::fastapp::Micro(now)," firing rpl '" , book.getSymbol(), "' bookId: '" , book.getInstanceID(), "' clordid: '" , Hex( castOrderRequest(sell_req).clordid) , "'|'" , std::dec, castOrderRequest(sell_req).origclordid , "': [Sell:",Qty(book.topBid()->qty),", ", Rate(book.topBid()->rate), KNRM , " ] " , "\n");
    }

    return now;
}

void reportSubmited(NewReqData & nr)
{
    using namespace ::fastapp::utils;
    using namespace ::fastapp::ouch;
    NewOrder & ord = castOrderRequest(nr);
    FMPRINT(Qty(ord.minqty) );
}

void reportSubmited(RplReqData  & nr)
{
    using namespace ::fastapp::utils;
    using namespace ::fastapp::ouch;
    ReplaceOrder & ord = castOrderRequest(nr);

    FMPRINT( " <origclordid: '"\
    , Hex( ord.origclordid ) , "'|'" \
    , ord.origclordid , "> ");

}

void print_msgtype(NewReqData & nr)
{
    FMPRINT( "new");
}

void print_msgtype(RplReqData & nr)
{
    FMPRINT( "rpl");
}

int64_t reportSubmitedBuy(int64_t now, const ::fastapp::itch::Book& book,
        auto & buy_req, ::fastapp::TcpConnection& trade_con)
{
    if (trade_con.IsOuchOn())
    {
        using namespace ::fastapp::utils;
        using namespace ::fastapp::ouch;
        using namespace ::fastapp;

        FMPRINT( KGRNB, ::fastapp::Micro(now), " fired ");
        print_msgtype(buy_req);
        FMPRINT( " '" , book.getSymbol(), "' bookId: '" , book.getInstanceID(), "' clordid: '" , Hex(castOrderRequest(buy_req).clordid) , "'|'" \
        , castOrderRequest(buy_req).clordid , "': [Buy:", Qty(castOrderRequest(buy_req).qty), ", ",Rate(castOrderRequest(buy_req).rate), ", ");

        reportSubmited(buy_req);

        FMPRINT( "]" , KNRM ,"\n");
    }
    return now;
}

void reportSubmitedSell( int64_t now, const ::fastapp::itch::Book& book, auto & sell_req, bool is_ouch_on)
{
    if (is_ouch_on)
    {
        using namespace ::fastapp::utils;
        using namespace ::fastapp::ouch;
        using namespace ::fastapp;

        FMPRINT( KGRNB, ::fastapp::Micro(now), " fired ");
        print_msgtype(sell_req);
        FMPRINT( " '" , book.getSymbol(), "' bookId: '" , book.getInstanceID() );
        FMPRINT( "' clordid: '" , Hex(castOrderRequest(sell_req).clordid) , "'|'" , castOrderRequest(sell_req).clordid , "': [Sell:", Qty(castOrderRequest(sell_req).qty), ", ",  Rate(castOrderRequest(sell_req).rate), ", ");
        reportSubmited(sell_req);
        FMPRINT( KNRM , " ] " , "\n");
    }
}

void reportBSendIssue(int64_t now, const ::fastapp::itch::Book& book, auto & sell_req,
        ::fastapp::TcpConnection& trade_con)
{
    if (trade_con.IsOuchOn())
    {
        using namespace ::fastapp::utils;
        using namespace ::fastapp;

        FMPRINT( KRED , ::fastapp::Micro(now), " FAILED sending order sym '", book.getSymbol(), "' with clordid: '", Hex(castOrderRequest(sell_req).clordid) , "'|'"
        , castOrderRequest(sell_req).clordid, "': [Sell:", Qty(castOrderRequest(sell_req).qty), ", ", Rate(castOrderRequest(sell_req).rate), KNRM , " ] ");
    }
}

void reportASendIssue(int64_t now,const ::fastapp::itch::Book& book, auto & buy_req,
        ::fastapp::TcpConnection& trade_con)
{
    if (trade_con.IsOuchOn())
    {
        using namespace ::fastapp::utils;
        using namespace ::fastapp;

        FMPRINT( KRED , Micro(now), " FAILED sending order for sym '", book.getSymbol(), "' bookId: '" , book.getInstanceID()\
        , "' clordid: '" , Hex(castOrderRequest(buy_req).clordid) , "'|'" , castOrderRequest(buy_req).clordid , "': [Buy:" \
        , Qty(castOrderRequest(buy_req).qty), ", ", Rate(castOrderRequest(buy_req).rate), "]" , KNRM, "\n");
    }
}

void reportStatistcics(int64_t now, const ::fastapp::itch::Book& book,
        ::fastapp::ouch::Ostore* pstore)
{
    if(!pstore->time2print(now, book.getInstanceID())) return;
    FMPRINT( KRED );
    pstore->print(now, book.getInstanceID());
    FMPRINT( KNRM , "\n");
}

void onBookUpdate(::fastapp::ouch::Ostore * pstore, ::fastapp::TcpConnection & trade_con,  const ::fastapp::itch::Book & book)
{
    if (!precondition(trade_con, book)) return;

    static int ctr = 0;
    int64_t now = fastapp::timeOfDay() ;
    if (pstore->time2submit(now, book.getInstanceID()) )
    {
        if (book.topBid() != 0 && book.topAsk() != 0)
        {
            using namespace ::fastapp::ouch;
            NewReqData buy_req;
            NewReqData sell_req;

            NewOrder & buy  = getNewOrderRequest(buy_req);
            NewOrder & sell = getNewOrderRequest(sell_req);

            populateBuy(book, buy);

            populateSell(book, sell);

            ::fastapp::ouch::Slot * slot
            = pstore->find_slot_by_instid(book.getInstanceID());
            if (slot != 0)
            {
                if (slot->sym.empty())
                {
                    slot->sym  = book.getSymbol();
                }

                now = check4MissedQuote(now, book, pstore, slot, trade_con);
            }

            if (pstore->submit_order(book.getInstanceID(), buy, sell) )
            {
                now = report2BeSubmitedOrder(now, book, trade_con, buy_req, sell_req);

                if (trade_con.send(buy_req , now) > 0)
                {
                    now = reportSubmitedBuy(now, book, buy_req, trade_con);

                    if (trade_con.send(sell_req, now) > 0)
                    {
                        reportSubmitedSell(now, book, sell_req, trade_con.IsOuchOn());
                    }
                    else
                    {
                        reportBSendIssue(now, book, sell_req, trade_con);
                    }

                    if (trade_con.IsOuchOn()) FMPRINT( "\n");
                }
                else
                {
                    reportASendIssue(now, book, buy_req, trade_con);
                }

                if (pstore->time2print(now, book.getInstanceID()) )
                {
                    reportStatistcics(now, book, pstore);
                }
            }
            ctr++;
        }
        // store.print();
    }
}


void onBookUpdateRpl(::fastapp::ouch::Ostore * pstore, ::fastapp::TcpConnection & trade_con,  const ::fastapp::itch::Book & book)
{
    if (!precondition(trade_con, book)) return;

    static int ctr = 0;
    int64_t now = fastapp::timeOfDay() ;

    if (book.topBid() == 0 && book.topAsk() == 0) return;


    if (!pstore->time2submit(now, book.getInstanceID()) )
        return;

    ::fastapp::ouch::Slot * slot
     = pstore->find_slot_by_instid(book.getInstanceID());


    if (slot == 0)
    {
        slot = pstore->allocate(book.getInstanceID(), book.getSymbol());
    }

    bool first_ever_evt = false;

    if (slot->sym.empty())
    {
        slot->sym  = book.getSymbol();
        first_ever_evt = true;
    }


    if (!pstore->time2submit(now, *slot) )
        return;

    {
        using namespace ::fastapp::ouch;
        NewReqData buy_req;
        NewReqData sell_req;

        NewOrder & buy  = getNewOrderRequest(buy_req);
        NewOrder & sell = getNewOrderRequest(sell_req);

        populateDayBuy(book, buy);
        buy.tif = eTimeInForce::Day;

        populateDaySell(book, sell);
        sell.tif = eTimeInForce::Day;

        ::fastapp::ouch::Slot * slot
         = pstore->find_slot_by_instid(book.getInstanceID());
        bool rpl_buy_flag  = false;
        bool rpl_sell_flag = false;
        //if (slot != 0)
        {
            now = check4MissedQuote(now, book, pstore, slot, trade_con);

            if ( pstore->time2submit(now, *slot) )
            {
                //FMPRINT( KRED , " 8Rpl " , KNRM , "\n");
                if (!first_ever_evt && validBuy(*slot) )
                {
                    //FMPRINT( KRED , " 9Rpl " , KNRM , "\n");
                    RplReqData  rpl_buy_req ;
                    ReplaceOrder & rpl_buy =  getReplaceRequest(rpl_buy_req);
                    //submit_buy_replace(pstore, slot, buy);
                    populate(rpl_buy, buy, slot->buy);

                    if (pstore->submit_replace(book.getInstanceID(),  rpl_buy, eSide::Buy))
                    {
                        //FMPRINT( KRED , "10Rpl " , KNRM , "\n");
                        now = report2BeSubmitedBuyRpl(now, book, trade_con, rpl_buy_req);
                        if (trade_con.send(rpl_buy_req , now) > 0)
                        {
                            now = reportSubmitedBuy(now, book, rpl_buy_req, trade_con);
                        }
                        else
                        {
                            reportASendIssue(now, book, rpl_buy_req, trade_con);
                        }
                        if (trade_con.IsOuchOn()) FMPRINT( "\n");
                    }
                    rpl_buy_flag = true;
                }

                //FMPRINT( KRED , " 5Rpl " , KNRM , "\n");
                if (!first_ever_evt && validSell(*slot))
                {
                    //FMPRINT( KRED , " 6Rpl " , KNRM , "\n");
                    RplReqData  rpl_sell_req ;
                    ReplaceOrder & rpl_sell =  getReplaceRequest(rpl_sell_req);
                    //submit_buy_replace(pstore, slot, buy);
                    populate(rpl_sell, sell, slot->sell);

                    //submit_sell_replace(pstore, slot, sell);
                    if (pstore->submit_replace(book.getInstanceID(),  rpl_sell, eSide::Sell))
                    {
                        //FMPRINT( KRED , " 7Rpl " , KNRM , "\n");
                        now = report2BeSubmitedSellRpl(now, book, trade_con, rpl_sell_req);
                        if (trade_con.send(rpl_sell_req , now) > 0)
                        {
                            reportSubmitedSell(now, book, rpl_sell_req, trade_con.IsOuchOn());
                        }
                        else
                        {
                            reportBSendIssue(now, book, rpl_sell_req, trade_con);
                        }
                        if (trade_con.IsOuchOn()) FMPRINT( "\n");
                    }
                    rpl_sell_flag = true;
                }
            }

            if (pstore->time2print(now, book.getInstanceID()) )
            {
                reportStatistcics(now, book, pstore);
            }

            //FMPRINT( KRED , " 11New " , KNRM , "\n");
            if (!rpl_buy_flag)
            {
                int64_t qty =  (buy.qty/3)*2;
                buy.qty = qty;
               //FMPRINT( KRED , " 12New " , KNRM , "\n");
               if (pstore->submit_order(book.getInstanceID(),  buy))
               {
                   now = report2BeSubmitedOrder(now, book, trade_con, buy_req);
                   if (trade_con.send(buy_req , now) > 0)
                   {
                       now = reportSubmitedBuy(now, book, buy_req, trade_con);
                   }
                   else
                   {
                       reportASendIssue(now, book, buy_req, trade_con);
                   }
                   if (trade_con.IsOuchOn()) FMPRINT( "\n");
               }
            }

            //FMPRINT( KRED , " 13New " , KNRM , "\n");
            if (!rpl_sell_flag)
            {
                int64_t qty =  (sell.qty/3)*2;
                sell.qty = qty;

                //FMPRINT( KRED , " 14New " , KNRM , "\n");
                if (pstore->submit_order(book.getInstanceID(),  sell))
                {
                    now = report2BeSubmitedOrder(now, book, trade_con, sell_req);
                    if (trade_con.send(sell_req , now) > 0)
                    {
                        reportSubmitedSell(now, book, sell_req, trade_con.IsOuchOn());
                    }
                    else
                    {
                        reportBSendIssue(now, book, sell_req, trade_con);
                    }
                    if (trade_con.IsOuchOn()) FMPRINT( "\n");
                }
            }

        }
        ctr++;
    }
        // store.print();
}

template <typename Defs, typename Connection, typename MarketProcessor>
int run_sniper(::fastapp::App & app, Connection & market, MarketProcessor & itch_p, ::fastapp::TcpConnection & trade_con)
{
    typedef typename Defs::ReqData         ReqData;
    typedef typename Defs::InstReqData     InstReqData;
    typedef typename Defs::Login           Login;
    typedef typename Defs::ClientHeartbeat ClientHeartbeat;
    typedef typename Defs::Logout          Logout;

    Logout mkt_logout;
    populateLogout(mkt_logout);

    ::fastapp::soupbin::Logout ord_logout;
    populateLogout(ord_logout);


    ::fastapp::ouch::Ostore store;
    ::fastapp::ouch::Ostore * pstore = &store;
    store.setDelta(app.getDelta()); // set frequency of firing orders
    store.setPrintDelta(app.getPrintDelta()); // set frequency of printing statistics

    // itch_p.init();
    itch_p.registerBookEvent
    (
        [pstore, &trade_con] (void * book_ptr, const ::fastapp::itch::BookUpdate & bu)
        {
            if (book_ptr == 0) return;

            ::fastapp::itch::Book & book = *(::fastapp::itch::Book *)book_ptr;

            onBookUpdate(pstore, trade_con,  book);
        }
    );
    
    if (app.getAlgo() == App::SRpl)
    {
       itch_p.registerBookEvent
       (
           [pstore, &trade_con]  (void * book_ptr, const ::fastapp::itch::BookUpdate & bu)
           {
            if (book_ptr == 0) return;

            ::fastapp::itch::Book & book = *(::fastapp::itch::Book *)book_ptr;

              onBookUpdateRpl(pstore, trade_con,  book);
           }
       );
    }
    
    market.setProcessor
    (
        [&itch_p](void * ptr, int32_t size) 
        {
            itch_p.process(ptr, size);    
        } 
    );
    
    itch_p.registerHelpers
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

    ouch_processor_t ouch_p(pstore);
//    ouch_p.init(pstore);
    ouch_p.setConnection(&trade_con);
    
    trade_con.setProcessor
    (
        [&ouch_p](void * ptr, int32_t size) 
        {
            ouch_p.process(ptr, size);    
        } 
    );    
    ouch_p.registerHelpers
    (
        [&trade_con] {return trade_con.IsTraceOn();  },
        [&trade_con] {return trade_con.IsSrvHbtOn(); },
        [&trade_con] {return trade_con.IsCliHbtOn(); },
        [&trade_con] {return trade_con.IsOuchOn();   },
        [&trade_con] (const std::string & session) {return trade_con.setSessionID(session); } ,
        [&trade_con] (const std::string & source, const std::string & subject, const std::string & text)
        {
            return trade_con.sendMail(source, subject, text);
        } 
    );
    
    bool enable_sub = false;
    
    itch_p.registerInfoMessage
    (
        [&market, &enable_sub, &app] (const ::fastapp::itch::InstrumentInfoMessage & msg, uint64_t seq)
        {
            int64_t tod = ::fastapp::Connection::timeOfDay();
            using namespace ::fastapp::utils;
            using namespace ::fastapp;
                    
            FMPRINT( Micro(tod), " - InstrumentInfoMessage:"\
            , msg.type      , ", sym='"\
            , std::string(msg.instSym, ::fastapp::itch::sym_size) , "', instrumentID='" \
            , msg.instID    , "', instrumentType='"\
            , msg.instType  , "', setlementDate='", ::fastapp::Sec(msg.setlmentDate), "'" , "\n");
            
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
                    using namespace ::fastapp::utils;
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
                    using namespace ::fastapp::utils;
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
                    using namespace ::fastapp::utils;
                    print_outgoing(tod, seq_data);
                }
            }            
        }
    );
    
    
    itch_p.registerEOFSession 
    ( 
        [&market, &mkt_logout] (const ::fastapp::soupbin::EndOfSession & m, int32_t size, int32_t seq)
        {
            int64_t tod = ::fastapp::Connection::timeOfDay();
            FMPRINT( Micro(tod), " : EndOfSession : " , m.type , ":" , size , " : seq (" , seq , ")"  , "\n");
            stampLogout(mkt_logout, market.nextSequence());
            market.send(mkt_logout, tod);
            market.disable();
        }
    );

    ouch_p.registerEOFSession 
    ( 
        [&trade_con, &ord_logout] (const ::fastapp::soupbin::EndOfSession & m, int32_t size, int32_t seq)
        {
            int64_t tod = ::fastapp::Connection::timeOfDay();
            FMPRINT( Micro(tod), " : EndOfSession : " , m.type , ":" , size , " : seq (" , seq , ")"  , "\n");
            trade_con.send(ord_logout, tod);
            trade_con.disable();
        }
    );
    
    if (!app.setup())
    {
        return -1;
    }
    
    ::fastapp::soupbin::Login order_login;
    populateLogin(order_login, trade_con.getSessionID(), trade_con.getSeqNum(), trade_con.getUserName(), trade_con.getPassword() );
    setLogin(order_login, ouch_p);
    
    Login market_login;
    populateLogin(market_login, market.getSessionID(), market.getSeqNum(), market.getUserName(), market.getPassword() );
    setLogin(market_login, itch_p);
    
    ClientHeartbeat mkt_hbc;
    populateHbc(mkt_hbc);
    
    ::fastapp::soupbin::ClientHeartbeat ord_hbc;
    populateHbc(ord_hbc);
    
    
    FMPRINT( "time_delta=" , market.time_delta , "\n");
    FMPRINT( "Mkt Login size:"   , sizeof(market_login) , "\n");
    FMPRINT( "Order Login size:" , sizeof(order_login ) , "\n");

    int64_t tod = ::fastapp::Connection::timeOfDay();
    stampLogin(market_login, market.nextSequence());
    market.send(market_login, tod);
    
    trade_con.send(order_login, tod);
    
    tod = ::fastapp::Connection::timeOfDay();
//    print_outgoing(tod, login);

    std::vector<std::string> symbols;
//    boost::split(symbols, market.getSymbols(), boost::is_any_of(",;:") );

    ::fastapp::utils::split(symbols, market.getSymbols().c_str(), ",;:" );
    
    FMPRINT( "split" , "\n");
    
    if (symbols.size() == 0)
    {
        symbols.push_back("");
    }
    
    if (symbols[0].empty())
    {
        FMPRINT( KRED , "Sending empty subscription symbol" , KNRM , "\n");
    }
 
    tod = ::fastapp::Connection::timeOfDay();
    enable_sub = market.listEnabled();
    
    while (market.getSessionID().empty() || trade_con.getSessionID().empty())
    {
        app.run_once();
    }
    
    FMPRINT( "\n", "Market SessionID=" , market.getSessionID() , "\n");
    FMPRINT( "\n" ,"Order  SessionID=" , trade_con.getSessionID() , "\n");
    tod = ::fastapp::Connection::timeOfDay();
    if (enable_sub)
    {
        InstReqData sbListReq;
        ::fastapp::itch::InstrumentListRequest & listRequest = getInstrumentListRequest(sbListReq);

        for(auto & sym : symbols)
        {
            if (sym.empty () ) continue;
            
            populateListRequest(listRequest, market.getSessionID(), sym );
            FMPRINT( KGRN , "List request for '" , sym , "' sessionID: '" , std::string(listRequest.sessionID, ::fastapp::soupbin::session_size) , "' - " , KNRM , "\n");
            stampListRequest(sbListReq, market.nextSequence());
            market.send(sbListReq, tod);
            if (market.IsTraceOn() && market.IsLowLevelOn())
            {
                using namespace ::fastapp::utils;
                print_outgoing (tod, sbListReq);        
            }
            app.run_once();
        }
    }

    if (!enable_sub)
    {
        ReqData seq_data;
        ::fastapp::itch::SubscriptionRequest  & req = getSubscriptionRequest(seq_data);
        populateSubscriptionRequest(req, market.getSessionID(), market.getSubscription(), symbols[0] );    
        for(auto & sym : symbols)
        {
            if (sym.empty () ) continue;
            tod = ::fastapp::Connection::timeOfDay();

            ::strncpy(req.instSym, sym.c_str(), ::fastapp::itch::sym_size);
            FMPRINT( KGRN , "Subscribe for '" , sym , "' sessionID: '" , std::string(req.sessionID, ::fastapp::soupbin::session_size) , "' - " , KNRM);
            if (market.IsBookSub())
            {
                req.reqID      ++;
                req.actionType = '1'; // subscribe
                req.updateType = '1'; // Book
                stampSubscriptionRequest(seq_data, market.nextSequence());
                market.send(seq_data, tod); 

                if (market.IsTraceOn() && market.IsLowLevelOn())
                {
                    using namespace ::fastapp::utils;
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
                    using namespace ::fastapp::utils;
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
                    using namespace ::fastapp::utils;
                    print_outgoing(tod, seq_data);
                }
            } 
            FMPRINT( KNRM , "\n");
            
            app.run_once();
        }   
    }

    tod = ::fastapp::Connection::timeOfDay();

    stampHeartbeat(mkt_hbc, market.nextSequence());
    market.send_recv(mkt_hbc, tod);
    
    trade_con.send_recv(ord_hbc, tod);
    FMPRINT(Micro(tod),  " ; Out (ClientHeartbeat) before run" , "\n");
    
    
    //sleep(3);
    app.run();
    
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
        ::fastapp::App::usage(argc, argv);
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

    app.parseCmd(argc, argv);

    if (app.numOfTcpConnections() < 1)
    {
        FMPRINT( "too few tcp connections, exiting" , "\n");
        return -1;
    }   
    
    if (app.numOfTcpConnections() + app.numOfUdpConnections()> 2)
    {
        FMPRINT( "too many connections, exiting" , "\n");
        return -1;
    }   
    
    if (app.numOfTcpConnections() + app.numOfUdpConnections() != 2)
    {
        FMPRINT( "too few connections, exiting" , "\n");
        return -2;
    };

    ::fastapp::TcpConnection * market_ptr = 0, * order_ptr = 0;
    ::fastapp::UdpConnection * udp_market_ptr = 0; //, * udp_order_ptr = 0;
    
    for (size_t i = 0; i < app.numOfTcpConnections(); i ++)
    {
        ::fastapp::TcpConnection & c = app.getTcpConnection(i);
        ::fastapp::ConnectionBase::Protocol protocol = c.getProtocol();
        if (::fastapp::ConnectionBase::Protocol::ITCH == protocol)
        {
            market_ptr = &c;
        } 
        else if (::fastapp::ConnectionBase::Protocol::OUCH == protocol)
        {
            order_ptr   = &c;
        }
    }   

    for (size_t i = 0; i < app.numOfUdpConnections(); i ++)
    {
        ::fastapp::UdpConnection & c = app.getUdpConnection(i);
        ::fastapp::ConnectionBase::Protocol protocol = c.getProtocol();
        if (::fastapp::ConnectionBase::Protocol::ITCH == protocol)
        {
            udp_market_ptr = &c;
        } 
        else if (::fastapp::ConnectionBase::Protocol::OUCH == protocol)
        {
            FMPRINT( "order udp connections are not allowed" , "\n");
            exit(-6);
        }
    }   

    
    if (order_ptr == 0)
    {
        FMPRINT( "order connection is not defined" , "\n");
        exit(-3);
    }
    ::fastapp::Connection & trade_con  = *order_ptr;
    
    if (market_ptr == 0 && udp_market_ptr == 0)
    {
        FMPRINT( "market connection is not defined" , "\n");
        exit(-3);
    }

    if (market_ptr != 0 && udp_market_ptr == 0)
    {
        ::fastapp::TcpConnection & market = *market_ptr;
        itch_processor_t p;

        return run_sniper<TcpDefs>(app, market, p, trade_con);
    }
    else if (market_ptr == 0 && udp_market_ptr != 0)
    {
        ::fastapp::UdpConnection & market = *udp_market_ptr;
        udp_itch_processor_t p;
        return run_sniper<UdpDefs>(app, market, p, trade_con);
    }
    else
    {
        FMPRINT( "invalid configuration" , "\n");
        return -1;
    }
    
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
