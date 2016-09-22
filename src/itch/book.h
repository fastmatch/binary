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

#ifndef QUOTE_H
#define    QUOTE_H

#include <iostream>
#include <stdio.h>
#include <ctype.h>
#include <iomanip>

#include <deque>
#include <src/defs.h>
#include <src/socketutils.h>
#include <algorithm>
#include <src/print.h>

namespace fastapp
{
    
namespace itch
{ 
struct Quote
{
    int32_t  id;
    int64_t  qty;
    int64_t  minQty;
    int32_t  rate;
    int16_t  maxDelay;
    bool     isNew;
    int64_t  time;
};

inline bool operator == ( const Quote& q1, const Quote& q2 )
{
    return q1.id == q2.id;
}

auto sortBids = []( const Quote& q1, const Quote& q2 ) { return q1.rate > q2.rate || (q1.rate == q2.rate && q1.qty < q2.qty); };
auto sortAsks = []( const Quote& q1, const Quote& q2 ) { return q1.rate < q2.rate || (q1.rate == q2.rate && q1.qty > q2.qty); };

class Book
{

public:    
    Book()
    : _symbol(std::string("")),
    _seqNum(0),
    _prevSeqNum(0),
    _bookTime(0),
    _delta(120 * ::fastapp::utils::ticksPerSecond() )
    {
    }

    void setStuckDelay(int64_t sec)
    {
        _delta = (sec * ::fastapp::utils::ticksPerSecond() );
    }
    
    Book (const Book & d) = default;
    Book (Book && d) = default;
    Book & operator = (Book && d) = default;
    Book & operator = (Book & d) = default;
    ~Book () = default;
    
    void setSymbol(const std::string & sym)
    {
        _symbol = sym;
    }
    
    const std::string & getSymbol() const
    {
        return _symbol;
    }
    
    void add (fastapp::itch::eSide side, const Quote & q)
    {
        using namespace fastapp::itch;
        switch (side)
        {
            case eSide::Bid:
                _bids.push_back(q);
                break;
            case eSide::Offer:
                _asks.push_back(q);
                break;
        }
    }
    
    void remove (const Quote & q)
    {
        erase(_bids, q);
        erase(_asks, q);
    }
    
    void add (const PriceAdd  & a)
    {
        Quote q = {a.priceUpdateID, a.qty, a.minQty, a.rate, a.maxDelay, true, ::fastapp::utils::getTimeNow() };
        add((fastapp::itch::eSide)a.side, q);
    }
    
    void remove (const PriceCancel & c)
    {
        Quote q = {c.priceUpdateID,0,0,0,0,false,0};
        remove(q);
    }

    static void erase(std::deque<Quote> & deq, const Quote & q)
    {
        auto it = std::find(deq.begin(), deq.end(), q);
        if (it != deq.end()) {deq.erase(it); }
    }
    
    bool update(int64_t ts, int64_t seqnum, int16_t instanceID)
    {
        bool ret = true;
        _bookTime = ts;
        _prevSeqNum = _seqNum;
        if(_instid != instanceID) _instid = instanceID;
        
        _instid = instanceID;
        if (_seqNum != 0 &&  _prevSeqNum + 1 != seqnum)
        {
            ret = false;
        }
        
        _seqNum = seqnum;
        
        return ret;
    }

    void clear()
    {
        _bids.clear();
        _asks.clear();
    }
    
    void wipe()
    {
        _bids.clear();
        _asks.clear();
        _seqNum = 0;
        _prevSeqNum = 0;
        _bookTime = 0;        
    }

    void allign_and_check()
    {
        std::sort(_bids.begin(),_bids.end(), sortBids);
        std::sort(_asks.begin(),_asks.end(), sortAsks);  
        
        check_top();
    }

    const Quote * topBid() const
    {
        if (_bids.empty() ) return 0;
    
        return &(_bids.front());

    }

    const Quote * botomBid() const
    {
        if (_bids.empty() ) return 0;

        return &(_bids.back());

    }

    const Quote * topAsk() const
    {
        if (_asks.empty() ) return 0;
        return &(_asks.front());
    }
    
    const Quote * bottomAsk() const
    {
        if (_asks.empty() ) return 0;
        return &(_asks.back());
    }

    void check_top(auto nextBuy, auto nextSell)
    {
        if (!_bids.empty() && !_asks.empty() && nextBuy->rate > nextSell->rate)
        {
            std::stringstream ss;
            ss << KRED << "Crossed Book! buy:"<< nextBuy->rate << "> sell:" << nextSell->rate  << "; seqNum=" << _seqNum << KNRM << std::endl;
            const std::string text = ss.str();
            std::string subject="Crossed Book in symbol: '"  + getSymbol() + "'";
            if (_sendEmail) 
            {
                _sendEmail(getSymbol(), subject, text);
            }
            
            FMPRINT(text);
        }
              
    }

    const Quote * find_by_rate(int64_t qty, int32_t rate, eSide side) const
    {
        auto &  coll = (side == eSide::Bid) ? _bids : _asks;

        for(const Quote & q : coll)
        {
            if (q.rate == rate && q.qty >= qty)
            {
                return &q;
            }
        }
        
        return 0;
    }
    
    void check_top()
    {
    auto nextBuy  = _bids.begin();
    auto nextSell = _asks.begin();       
        
        check_top(nextBuy, nextSell);        
    }
    
    
    void print_and_check(int16_t instid)
    {
        using namespace fastapp;

        int64_t timenow =  ::fastapp::utils::getTimeNow();
        
        FMPRINT("Update Time: " , Micro(_bookTime) , ", Sym: " , _symbol , ", SeqNum: " , _seqNum , ", Buy/Bid | Sell/Ask" );
        if (_symbol.empty()) FMPRINT( ", InstrumentID: " , instid);
        FMPRINT("\n");

        std::sort(_bids.begin(),_bids.end(), sortBids);
        std::sort(_asks.begin(),_asks.end(), sortAsks);
        
        auto nextBuy  = _bids.begin();
        auto nextSell = _asks.begin();       
        
        //check_top(nextBuy, nextSell);
        
        static std::string STUCK = std::string(KRED) + "ST";
        static std::string GOOD  = std::string("GD");
        // static const int64_t delta = _sec_num * ( ::utils::ticksPerSecond() );
        bool stuck = false;
        std::stringstream ss;

        while( nextBuy != _bids.end() && nextSell != _asks.end() )
        {
            const std::string * stuck_buy  = ((nextBuy->maxDelay !=0 && timenow -  nextBuy->time  > _delta ) ? &STUCK : &GOOD);
            const std::string * stuck_sell = ((nextSell->maxDelay!=0 && timenow -  nextSell->time > _delta ) ? &STUCK : &GOOD);
                
            stuck = stuck || (stuck_buy  == &STUCK);
            stuck = stuck || (stuck_sell == &STUCK);
                
            ss << "  " << (nextBuy->isNew ? KGRN : KNRM) << *stuck_buy<< nextBuy->id << " " << std::setprecision(5) << std::fixed 
                           << std::setw(8) << ((double)nextBuy->rate)/100000 << " " << std::setprecision(2) << std::fixed << std::setw(12) << nextBuy->qty/100 
                           << " " << ( nextBuy->maxDelay==0 ? 'Y' : 'N' ) << " ; " << std::setfill('0') << std::setw(12) << timenow - nextBuy->time << "   " << (nextSell->isNew ? KGRN : KNRM) << *stuck_sell << nextSell->id 
                           << " " << std::setprecision(5) << std::fixed << std::setw(8) << ((double)nextSell->rate)/100000 << " " << std::setprecision(2) 
                           << std::fixed << std::setw(12) << nextSell->qty/100 << " " << ( nextSell->maxDelay==0 ? 'Y' : 'N' ) << " ; " << std::setfill('0') << std::setw(12) << timenow - nextSell->time << KNRM << std::endl;

            nextBuy->isNew = false;
            nextSell->isNew = false;
                
            ++nextBuy;
            ++nextSell;
        }

        while( nextBuy != _bids.end() )
        {
            const std::string * stuck_buy  = (nextBuy->maxDelay !=0 && timenow - nextBuy->time > _delta ) ? &STUCK : &GOOD;
            
            ss << "  " << (nextBuy->isNew ? KGRN : KNRM) << *stuck_buy << nextBuy->id << " " << std::setprecision(5) 
                              << std::fixed << std::setw(8) << ((double)nextBuy->rate)/100000 << " " << std::setprecision(2) << std::fixed 
                              << std::setw(12) << nextBuy->qty/100 << " " << ( nextSell->maxDelay==0 ? 'Y' : 'N' ) << " ; " << std::setfill('0') << std::setw(12) << timenow - nextBuy->time << KNRM 
                              << "    ------------------------------------------------      " << std::endl;

            nextBuy->isNew = false;
            ++nextBuy;
            
            stuck = stuck || (stuck_buy == &STUCK);
        }

        while( nextSell != _asks.end() )
        {
            const std::string * stuck_sell = (nextSell->maxDelay!=0 && timenow - nextSell->time > _delta ) ? &STUCK : &GOOD;
            
            ss << "   ------------------------------------------------ \t" << (nextSell->isNew ? KGRN : KNRM) 
                              << *stuck_sell << nextSell->id << " " << std::setprecision(5) << std::fixed << std::setw(8) << ((double)nextSell->rate)/100000 << " " 
                              << std::setprecision(2) << std::fixed << std::setw(12) << nextSell->qty/100 << " " 
                              << ( nextSell->maxDelay==0 ? 'Y' : 'N' ) << " ; " << std::setfill('0') << std::setw(12) << timenow - nextSell->time << KNRM << std::endl;

            nextSell->isNew = false;
            ++nextSell;
            stuck = stuck || (stuck_sell == &STUCK);
        }

        std::string && text = ss.str();
        
        FMPRINT(text, "\n");
        
        if (stuck)
        {
            _sendEmail(getSymbol(), "stuck quote", text);
        }
    }
    
    void setEmailCallback(fastapp::callbacks::send_email_t      sendEmail)
    {
        _sendEmail = sendEmail;
    }
    
    
    int64_t getSeqNum ()const
    {
        return _seqNum;
    }
    
    int64_t getPrevSeqNum() const
    {
        return _prevSeqNum;
    }
    
    int16_t  getInstanceID() const
    {
        return _instid;
    }
    
protected:    
    std::deque<Quote> _bids;
    std::deque<Quote> _asks;
    
    std::string       _symbol;
    int16_t           _instid;
    int64_t           _seqNum;
    int64_t           _prevSeqNum;
    int64_t           _bookTime;
    int64_t           _delta;

    fastapp::callbacks::send_email_t      _sendEmail;
};   

} //itch

}
 // fastapp
#endif    /* QUOTE_H */

