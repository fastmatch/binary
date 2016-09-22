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

#ifndef FAST_OSTORE_H
#define	FAST_OSTORE_H

#include <deque>
#include <functional>
#include <src/ouch/ouch.h>
#include <src/helpers.h>

namespace fastapp
{

template <typename PROTOCOL>
class TmplConnection;

namespace ouch
{
    
#pragma pack(push, 1)
    
struct Fill
{
    int64_t        fillqty;
    int32_t        fillrate;
    execid_t       execid;
};

struct Quote
{
    int64_t       qty;
    int32_t       rate;
    int32_t       instid;
    int64_t       time;
    eSide         side;

    Quote() = default;
    ~Quote() = default;
    Quote(const Quote & ) = default;
    Quote & operator = (const Quote &);
};

struct Order
{
    int64_t       qty;
    clord_id_t    clordid;
    //clord_id_t    pending_clordid;
    int32_t       rate;
    int64_t       cum_qty;
    eSide         side;

    Order() 
    : qty(0)
    , clordid(0)
    , rate(0)
    , cum_qty(0)
    {
    }
    Order(const Order &) = default;
    Order & operator = (const Order &) = default;
    ~Order() = default;
};

struct Slot;
void print_slot(const Slot &);

struct Slot
{
    Order    buy;
    Order    sell;
    std::vector<Order> pordrs;
    
    int64_t  fill_sellqty  ;
    int64_t  fill_buyqty   ;
    int64_t  total_sellqty ;
    int64_t  total_buyqty  ;
    int64_t  last          ;
    mutable int64_t  last_print;
    int64_t  reject_count  ;
    int64_t  accept_count  ;
    int64_t  cancel_count  ;
    int64_t  accept_replace_count  ;
    int64_t  reject_replace_count;
    int64_t  total_replace_count  ;
    int64_t  total_cxl_count;
    int64_t  total_count   ;
    int64_t  fill_count    ;
    int64_t  partial_count ;
    int64_t  filled_count  ;    
    
    int32_t  instid        ;
    std::string sym;
    std::queue<Quote>  queue;

    static int i ;
    static int j ;
    static int c ;
    static int cc;

    Slot() :
    fill_sellqty  (0),
    fill_buyqty   (0),
    total_sellqty (0),
    total_buyqty  (0),
    last          (0),
    last_print    (0),
    reject_count  (0),
    accept_count  (0),
    cancel_count  (0),
    accept_replace_count(0),
    reject_replace_count(0),
    total_replace_count(0),
    total_cxl_count(0),
    total_count   (0),
    fill_count    (0),
    partial_count (0), 
    filled_count  (0),
    instid        (0)
    {
        pordrs.resize(50);
//        std::cerr << KRED << i << " SLOT '" << this << "'" << KNRM << std::endl;
//        i ++;
    }

    ~Slot()
    {
//        if (!sym.empty() )
//        {
//            std::cerr << KCYN << j << "~SLOT '" << this << "' ";
//            print_slot(*this);
//            std::cerr <<  KNRM << std::endl;
//        }
        j ++;
    }

    Slot & operator =(const Slot & s) 
    {
//        std::cerr << KYEL << c << "SLOT= '" << this << "'" << KNRM << std::endl;
//        c ++;
 

        fill_sellqty  = s.fill_sellqty;
        fill_buyqty   = s.fill_buyqty;
        total_sellqty = s.total_sellqty;
        total_buyqty  = s.total_buyqty;
        last          = s.last;
        last_print    = s.last_print;
        reject_count  = s.reject_count;
        accept_count  = s.accept_count;
        cancel_count  = s.cancel_count;
        total_cxl_count = s.total_cxl_count;
        total_count   = s.total_count;
        fill_count    = s.fill_count;
        partial_count = s.partial_count;
        filled_count  = s.filled_count;
        instid        = s.instid;
        sym           = s.sym;
        buy           = s.buy;
        sell          = s.sell;
        accept_replace_count  = s.accept_replace_count;
        reject_replace_count  = s.reject_replace_count;
        total_replace_count   = s.total_replace_count;

        return *this;
    }

    Slot(const Slot & s)
    :   buy           ( s.buy)
    ,   sell          ( s.sell)
    ,   fill_sellqty  ( s.fill_sellqty)
    ,   fill_buyqty   ( s.fill_buyqty)
    ,   total_sellqty ( s.total_sellqty)
    ,   total_buyqty  ( s.total_buyqty)
    ,   last          ( s.last)
    ,   last_print    ( s.last_print)
    ,   reject_count  ( s.reject_count)
    ,   accept_count  ( s.accept_count)
    ,   cancel_count  ( s.cancel_count)
    ,   accept_replace_count(s.accept_replace_count)
    ,   reject_replace_count(s.reject_replace_count)
    ,   total_replace_count(s.total_replace_count)
    ,   total_cxl_count(s.total_cxl_count)
    ,   total_count   ( s.total_count)
    ,   fill_count    ( s.fill_count)
    ,   partial_count ( s.partial_count)
    ,   filled_count  ( s.filled_count)
    ,   instid        ( s.instid)
    ,   sym           ( s.sym)
    {
//        std::cerr << KRED << cc << "SLOT(=) '" << this << "'" << KNRM << std::endl;
//        cc ++;

    }
};

                   
struct OrderID
{
    uint16_t oid;
    int16_t  book;
};

#pragma pack(pop)


class Ostore
{
public:
    typedef std::function<void (const std::string & sym, int64_t time, const Quote &) > process_delayed_quote_t;
    ~Ostore();
    bool time2submit(int32_t instid) const;
    bool time2submit(int64_t now, int32_t instid) const;
    bool time2submit(int64_t now, const Slot & slot) const;
    
    bool time2print(int32_t instid) const;
    bool time2print(int64_t now, int32_t instid) const;
    bool time2print(int64_t now, const Slot & slot) const;
        
    Ostore();
    Order * find_order(clord_id_t clordid);
    Order * find_order(clord_id_t clordid, Slot ** slot);

    Order * find_order(clord_id_t clordid, clord_id_t origclordid);
    Order * find_order(clord_id_t clordid, clord_id_t origclordid, Slot ** slot);

    Slot *  find_slot_by_clordid(clord_id_t clordid);
    Slot *  find_slot_by_instid(int32_t instid);
    bool accept_order(clord_id_t clordid);
    bool reject_order(clord_id_t clordid);
    bool process_trade(const Trade & trade, ::fastapp::TmplConnection<::fastapp::sockets::TCP> * trade_con);
    bool cancel_order(clord_id_t clordid);

    bool cancel_replace(clord_id_t clordid, clord_id_t origclordid);
    bool accept_replace(clord_id_t clordid, clord_id_t origclordid);
    bool reject_replace(clord_id_t clordid, clord_id_t origclordid);

    void print(int64_t now, const Slot & slot);
    void print(int64_t now, int32_t instid);
    void print_slot(int64_t now, const Slot & slot);

    bool time2print_aggr() const;
    bool time2print_aggr(int64_t now) const;
    void print_aggr(int64_t now);
    void print_aggr(int64_t now, const Slot & slot);
    void print_aggr_ext(int64_t now)
    {
        if (time2print_aggr(now))
        {
            print_aggr(now);
        }
    }

    bool submit_order(int16_t  instid,  NewOrder & buy, NewOrder & sell);

    Slot & getValidSlot(int16_t  instid,  ccypair_int_t ccypair);

    bool submit_order(int16_t  instid,  NewOrder & ord);
    bool submit_replace(int16_t  instid,  ReplaceOrder & ord,  ::fastapp::ouch::eSide side);
    int32_t  submit_cancelorder(Slot * slot);

    void setDelta(int64_t delta);
    void setPrintDelta(int64_t delta);

    process_delayed_quote_t     process_delayed;    
public:
    bool peek(Slot * slot, int64_t now, const Quote * & q);
    bool pop (Slot * slot, const Quote *q);
    Slot * allocate(int16_t  instid, const std::string & sym);

protected:    
    bool postpone(Slot * slot, const Quote & q);
    
protected:    
    std::unique_ptr<std::vector<Slot> > _books;
    int64_t          _delta;
    int64_t          _print_delta;
    int64_t          reject_count;
    Slot             _all;
};

bool validSell(::fastapp::ouch::Slot & slot);
bool validBuy (::fastapp::ouch::Slot & slot);

}
}


#endif	/* OSTORE_H */

