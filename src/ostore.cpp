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

#include <src/defs.h>
#include <src/helpers.h>
#include <src/ostore.h>
#include <cassert>

namespace fastapp
{
    
namespace ouch
{
  
int Slot::i  = 0;
int Slot::j  = 0;
int Slot::c  = 0;
int Slot::cc = 0;

bool validSell(::fastapp::ouch::Slot & slot)
{
//    if (!(buy.qty > 0 && sell.qty > 0) ) return false;
    if (slot.sell.clordid != 0)
    {
        if (slot.sell.qty != slot.sell.cum_qty)
        {
            return true;
        }
    }
    return false;
}

bool validBuy(::fastapp::ouch::Slot & slot)
{
    if (slot.buy.clordid != 0)
    {
        if (slot.buy.qty != slot.buy.cum_qty)
        {
            return true;
        }
    }
    return false;
}

Ostore::Ostore()
: _books(new std::vector<Slot>() )
, _delta( 60 * ::fastapp::ticksPerSecond() )
, _print_delta ( 60 * ::fastapp::ticksPerSecond() )
, reject_count(0)
{
    _books->resize(512);
    _all.sym="ALL";
}

Ostore::~Ostore()
{
    FMPRINT( KRED , "Final count: " , KNRM , "\n");

    int64_t now = ::fastapp::timeOfDay() ;
    for (size_t index = 0; index < _books->size() ; ++index)
    {
        const Slot & slot = _books->at(index);
        if (!(slot.sym.empty()))
        {   
//            FMPRINT( "#" , index , " "; 
            print_slot (now, slot);    
            print_aggr (now, slot);
        }
//        index ++;
    }

    print_aggr(now);
}

void Ostore::setDelta(int64_t delta)
{
    _delta = delta  * (::fastapp::ticksPerSecond()/1000 );
}

void Ostore::setPrintDelta(int64_t delta)
{
    _print_delta = delta * (::fastapp::ticksPerSecond()/1000);
}

bool Ostore::time2print(int32_t instid) const
{
    int64_t now = ::fastapp::timeOfDay() ;

    return time2print(now, instid);
}

bool Ostore::time2print_aggr() const
{
    int64_t now = ::fastapp::timeOfDay() ;

    return time2print_aggr(now);
}

bool Ostore::time2print_aggr(int64_t now) const
{
    if (now - _all.last_print > (ssize_t)_books->size() * _print_delta)
    {
        return true;
    }

    return false;
}


bool Ostore::time2print(int64_t now, int32_t instid) const 
{
    if (instid > (ssize_t)_books->size()) return true;
    
    const Slot & slot = _books->at(instid);
    
    return time2print(now, slot);    
}

bool Ostore::time2print(int64_t now, const Slot & slot) const 
{
    // FMPRINT( KYEL , "Time 2 Print :" , now - slot.last_print , ">?"  , _print_delta , KNRM , "\n");
    if (now - slot.last_print > _print_delta) return true;
    return false;    
}
    
bool Ostore::time2submit(int32_t instid) const
{
    int64_t now = ::fastapp::timeOfDay() ;

    return time2submit(now, instid);
}

bool Ostore::time2submit(int64_t now, int32_t instid) const
{
    if (instid > (ssize_t)_books->size()) return true;
    
    const Slot & slot = _books->at(instid);
    
    return time2submit(now, slot);
}

bool Ostore::time2submit(int64_t now, const Slot & slot) const
{
    //FMPRINT( KYEL , "Time 2 SuBmit :" , now - slot.last , ">?"  , _delta , KNRM , "\n");
    
    if (now - slot.last > _delta) return true;
    return false;
}    

Order * Ostore::find_order(clord_id_t clordid)
{
     Slot  * slot = 0;
     return find_order(clordid, &slot);
}

Order * Ostore::find_order(clord_id_t clordid, clord_id_t origclordid)
{
     Slot  * slot = 0;
     return find_order(clordid, origclordid, &slot);
}


Slot *  Ostore::find_slot_by_instid(int32_t instid)
{
    if ((ssize_t)instid > (ssize_t)_books->size()) return 0;

    Slot & slot = _books->at(instid);
    return &slot;
}

Slot * Ostore::find_slot_by_clordid(clord_id_t clordid)
{
    OrderID * id_ptr = reinterpret_cast<OrderID *>((void *)&clordid);
    
    if ((ssize_t)id_ptr->book > (ssize_t)_books->size()) return 0;
    
    Slot & slot = _books->at(id_ptr->book);
    return &slot;
}


Order * Ostore::find_order(clord_id_t clordid, Slot ** slot)
{
    OrderID * id_ptr = reinterpret_cast<OrderID *>((void *)&clordid);
    
    if ((ssize_t)id_ptr->book > (ssize_t)_books->size()) return 0;
    
    *slot = &_books->at(id_ptr->book);
    
    Order * order = 0;
    if ((*slot)->buy.clordid == clordid)
    {
        order = &( (*slot)->buy);
    } 
    else if ( (*slot)->sell.clordid == clordid)
    {
        order = &((*slot)->sell);
    }
    return order;
}

Order * Ostore::find_order(clord_id_t clordid, clord_id_t origclordid, Slot ** slot)
{
    OrderID * id_ptr = reinterpret_cast<OrderID *>((void *)&clordid);

    if ((ssize_t)id_ptr->book > (ssize_t)_books->size()) return 0;

    *slot = &_books->at(id_ptr->book);

    Order * order = 0;
    OrderID * pid = reinterpret_cast<OrderID *>((void *)&clordid);
    Order * pord = &((*slot)->pordrs[pid->oid % (*slot)->pordrs.size()]);
    if ((*slot)->buy.clordid == clordid
        || (*slot)->buy.clordid == origclordid
        || (pord->side == eSide::Buy && pord->clordid == clordid)
    )
    {
        order = &( (*slot)->buy);
    }
    else if ( (*slot)->sell.clordid == clordid
        || (*slot)->sell.clordid == origclordid
        || (pord->side == eSide::Sell && pord->clordid == clordid)
    )
    {
        order = &((*slot)->sell);
    }
    return order;
}

bool Ostore::accept_order(clord_id_t clordid)
{
//    int64_t now = ::fastapp::timeOfDay() ;
    
    Slot * slot = 0;
    Order * order = find_order(clordid, &slot);
    
    if (slot != 0)
    {
        slot->accept_count ++;
        _all.accept_count ++;
    }
    return (order != 0);
}


bool Ostore::cancel_replace(clord_id_t clordid, clord_id_t origclordid)
{
    Slot * slot = 0;
    Order * order = find_order(clordid, origclordid, &slot);

    if (order != 0)
    {
        order->qty = 0;
        order->rate = 0;
        order->clordid = 0;
        order->cum_qty = 0;
    }
    else
    {
        FMPRINT( KYEL , "Err-CR!!! Order already closed clorid: " , clordid , " orig.clordid:" , origclordid , KNRM , "\n");
    }

    if (slot != 0)
    {
        slot->reject_replace_count ++;

        OrderID * oid = reinterpret_cast<OrderID *>(&clordid);

        Order * newOrd = &(slot->pordrs[oid->oid % slot->pordrs.size()]);
        // set newOrd->clordid to zero in case pending order is already overwritten
        if (newOrd->clordid == clordid) newOrd->clordid = 0;
    }

    _all.reject_replace_count ++;


    return (order != 0);
}

bool Ostore::accept_replace(clord_id_t clordid, clord_id_t origclordid)
{
//    int64_t now = ::fastapp::timeOfDay();
    Slot * slot = 0;
    Order * newOrd = 0;
    Order * order = find_order(clordid, origclordid, &slot);

    if (slot != 0)
    {
        slot->accept_replace_count ++;
        _all.accept_replace_count ++;

        OrderID * oid = reinterpret_cast<OrderID *>(&clordid);

        newOrd = &(slot->pordrs[oid->oid % slot->pordrs.size()]);
        // set newOrd->clordid to zero in case pending order is already overwritten
        if (newOrd->clordid != clordid) newOrd->clordid = 0;
    }



    if (order != 0)
    {
//        assert(order->clordid == origclordid);
//        assert(order->pending_clordid == clordid);

        if (newOrd != 0)
        {
            if (order->side == eSide::Sell)
            {
                slot->total_sellqty    += newOrd->qty;
                slot->total_sellqty    -= order->qty;
            }
            else
            {
                slot->total_buyqty    += newOrd->qty;
                slot->total_buyqty    -= order->qty;
            }
            *order = *newOrd;
            newOrd->clordid = 0;
        }

        order->clordid = clordid;
    }
    else
    {
        FMPRINT( KRED , "Failed to find order: " , clordid , KNRM , "\n");
    }


    return (order != 0);
}

bool Ostore::reject_replace(clord_id_t clordid, clord_id_t origclordid)
{
//    int64_t now = ::fastapp::timeOfDay() ;

    Slot * slot = 0;
    Order * order = find_order(clordid, origclordid, &slot);

    if (slot != 0)
    {
        slot->reject_replace_count ++;
//        slot->partial_count --;
//        slot->filled_count  ++;

    }
    _all.reject_replace_count ++;
//    _all.partial_count  --;
//    _all.filled_count  ++;

    OrderID * oid = reinterpret_cast<OrderID *>(&clordid);
    Order * newOrd = &(slot->pordrs[oid->oid % slot->pordrs.size()]);
    // set newOrd->clordid to zero in case pending order is already overwritten
    if (newOrd->clordid == clordid)
    {
        newOrd->clordid = 0;
    }

    return (order != 0);
}

bool Ostore::reject_order(clord_id_t clordid)
{
    //int64_t now = ::fastapp::timeOfDay() ;
    
    Slot * slot = 0;
    Order * order = find_order(clordid, &slot);
    
    if (order != 0)
    {
        order->qty = 0;
        order->rate = 0;
        order->clordid = 0;
        order->cum_qty = 0;
        slot->reject_count ++;
        _all.reject_count ++;
    }
    else
    {
        reject_count ++;
        _all.reject_count ++;

        FMPRINT( KRED , "Err!!! REJ_ORD Order already closed clorid: " , clordid , KNRM , "\n");
    }
    
    return (order != 0);
}

bool Ostore::process_trade(const Trade & trade, ::fastapp::TmplConnection<::fastapp::sockets::TCP> * trade_con)
{
    
    Slot * slot = 0;
    Order * order = find_order(trade.clordid, &slot);
    
    if (slot != 0)
    {
//        FMPRINT( KGRN , "process_trade :" , trade.clordid , " Slot=" , slot , " sym='" , slot->sym , "'" , KNRM , "\n");
        slot->fill_count ++;
        _all.fill_count ++;

        if (trade.leaves > 0)
        { 
             if (order != 0 && (order->cum_qty == 0 )) 
             {
                 slot->partial_count ++;
                 _all.partial_count ++;
             }
        }
                
        if (order != 0) order->cum_qty += trade.fillqty;
        
        if (trade.leaves == 0 || (order != 0 && order->cum_qty == order->qty) )
        {
            if (order !=0)
            {
                slot->partial_count --;
                _all.partial_count  --;
                slot->filled_count  ++;

                if (trade.leaves != order->qty - order->cum_qty)
                {
                    FMPRINT( KRED , "Invalid order state for "\
                    , "sym (" , std::string((const char *)((void *)&(trade.ccypair)), 7) , ") clorid: "\
                    , trade.clordid \
                    , " execid:" , std::string(trade.execid, execid_size) \
                    , "leaves (" , trade.leaves , ") != " \
                    , " qty(" , order->qty , ") - cum_qty(" , order->cum_qty , ")" \
                    , KNRM , "\n");
                }

                if (trade.leaves == 0)
                { 
                    for (Order & o : slot->pordrs)
                    {
                        if (o.clordid != 0)
                        {
                           FMPRINT( KRED \
                           , "sym (" , std::string((const char *)((void *)&(trade.ccypair)), 7) \
                           , ") clordid: '" , o.clordid , "' is not ACKed " \
                           , KNRM , "\n");

                           CxlReqData creq;
                           CancelOrder & co = getCancelOrderRequest(creq);

                           co.ccypair = trade.ccypair;
                           co.clordid = submit_cancelorder(slot);
                           co.origclordid = o.clordid;
                           uint64_t tod = ::fastapp::utils::getTimeOfDay() ;
                           trade_con->send(creq, tod);
                           o.clordid = 0;
                        }
                    }
                }

                order->clordid = 0;
                order->qty     = 0;
                order->cum_qty = 0;
                order->rate    = 0;

                _all.filled_count ++;
            }
            else
            {
                FMPRINT( KRED , "Err-TRD!!! Order already closed clorid: " \
                , trade.clordid \
                , " execid:" , std::string(trade.execid, execid_size) \
                , "sym (" , std::string((const char *)((void *)&(trade.ccypair)), 7) , ") " \
                , KNRM , "\n");
            }

        }
                
        switch(trade.side)
        {
            case eSide::Buy:
                slot->fill_buyqty += trade.fillqty;
                if (trade.leaves == 0)
                {
                    slot->buy.clordid = 0; 
                    slot->buy.qty     = 0;
                    slot->buy.cum_qty = 0;
                    slot->buy.rate    = 0;
                }
                break;
            case eSide::Sell:
                slot->fill_sellqty += trade.fillqty;
                if (trade.leaves == 0)
                {
                    slot->sell.clordid = 0;
                    slot->sell.qty     = 0;
                    slot->sell.cum_qty = 0;
                    slot->sell.rate    = 0;
                }
                break;
            default: 
                break;
        }
    }
    else
    {
        FMPRINT( KRED , "missed order :" , trade.clordid , KNRM , "\n");
    }
    
    return (order != 0);
}

bool Ostore::peek(Slot * slot, int64_t now, const Quote * & q)
{
    if (slot->queue.empty()) return false;
    q = &slot->queue.front();
    
    if (now - q->time > (::fastapp::ticksPerSecond()/5000) )
    { // complain in 5000 micros
        return true;
    }
    return false;
}

bool Ostore::pop (Slot * slot, const Quote *q)
{
    if (slot->queue.empty()) return false;
    slot->queue.pop();
    return true;
}

bool Ostore::postpone(Slot * slot, const Quote & q)
{
    if (slot->queue.empty()) return false;
    slot->queue.push(q);
    return true;
}

bool Ostore::cancel_order(clord_id_t clordid)
{
    int64_t now = ::fastapp::timeOfDay() ;

    Slot * slot = 0;    
    Order * order = find_order(clordid, &slot);
    
    if (order != 0)
    {
        if (order->cum_qty == 0)
        {
            Quote q;
            q.qty    = order->qty;
            q.rate   = order->rate;
            q.side   = order->side;
            q.instid = slot->instid;
            q.time   = now;

            postpone(slot, q);
        }

        slot->cancel_count ++;
        _all.cancel_count ++;

        order->qty = 0;
        order->rate = 0;
        order->clordid = 0;
        order->cum_qty = 0;
    }
    else
    {
        FMPRINT( KRED , "Err-CXL!!! Order already closed clorid: " \
        , clordid \
        , KNRM , "\n");
    }
    
    return (order != 0);
}

void Ostore::print(int64_t now, int32_t instid)
{
    if (instid > (ssize_t)_books->size()) return;
    
    const Slot & slot = _books->at(instid);
    print(now, slot);
}

void Ostore::print_aggr(int64_t now, const Slot & slot)
{
    using namespace ::fastapp::utils;

    int64_t inflight = slot.accept_count - (slot.filled_count + slot.cancel_count);
    bool ack  = (inflight < 2);
    bool ords = (slot.reject_count + slot.accept_count ==  slot.total_count);
    bool cxld = (slot.partial_count == slot.cancel_count);
    bool rpls = (slot.total_replace_count == slot.reject_replace_count + slot.accept_replace_count);

    if (&slot == &_all || !ack || !ords || !cxld || !rpls)
    {
    FMPRINT( KCYN,  ::fastapp::Micro(now) , \
    " Sym: "          , slot.sym , \
    ", Rejected:"     , slot.reject_count , \
    ", Accepted:"     , slot.accept_count , \
    ", Canceled:"     , slot.cancel_count , \
    ", Orders  :"     , slot.total_count  , \
    ", Fills   :"     , slot.fill_count   , \
    ", Partials:"     , slot.partial_count , \
    ", Filled  :"     , slot.filled_count  , \
    ", UnkRejects:"   , reject_count       , \
    ", RplRejected:"  , slot.reject_replace_count , \
    ", RplAccepted:"  , slot.accept_replace_count , \
    ", Replaces:"     , slot.total_replace_count  , \
    ", In-flight: "    , inflight , KNRM , "\n");

    if (!ack || !ords || !cxld || !rpls)
    {
         FMPRINT( KCYN ,  " Sym: " , slot.sym , (ords ? KGRN :KRED) ," (Accepted<" , slot.accept_count , "> + Rejected <" \
         , slot.reject_count , "> == Orders<", slot.total_count , ">)= " \
         , (ords ? "OK " : "ERROR! "  ) , KNRM ,  (ack ? KGRN : KRED) , " (in-flight<" , \
         inflight , "> < 2 " \
         , ")=" , (ack ? "OK " : "ERROR ") \
         , (cxld ? KGRN :KRED) , " (PartiallyFilled< ", slot.partial_count \
         , "> == Cancelled<" , slot.cancel_count , "> )=" \
         , (cxld ? "OK " : "ERROR ") \
         , (rpls ? KGRN :KRED) , " (Replaces< ", slot.total_replace_count \
         , "> == Accepted<" , slot.accept_replace_count , "> + Rejected<" \
         , slot.reject_replace_count , ">)=" \
         , (rpls ? "OK " : "ERROR ") ); 

         if (slot.buy.clordid != 0 || slot.sell.clordid != 0)
         {
             FMPRINT( " ; suspicious clordids: {");
         }
         if (slot.buy.clordid != 0)
         {
             FMPRINT( " <" , slot.buy.clordid , "> ");
         }
         if (slot.sell.clordid != 0)
         {
             FMPRINT( " <" , slot.sell.clordid , "> ");
         }

         if (slot.buy.clordid != 0 || slot.sell.clordid != 0)
         {
             FMPRINT( "}");
         }

         FMPRINT( KNRM , "\n");
    }
    }
    slot.last_print = now;
}

void Ostore::print_aggr(int64_t now)
{
    print_aggr(now,_all);

    if (reject_count > 0)
    {
        FMPRINT( KRED , "UnkRejects:"   , reject_count , " !!!! Error ", KNRM , "\n");
    }
}

void print_slot(const Slot & slot)
{
    int64_t inflight = slot.accept_count - (slot.filled_count + slot.cancel_count);

    FMPRINT( \
    " Sym: " , slot.sym ,\
    ", Traded sell:"  , slot.fill_sellqty ,\
    ", Traded buy :"  , slot.fill_buyqty  ,\
    ", Total  sell :" , slot.total_sellqty ,\
    ", Total  buy  :" , slot.total_buyqty  ,\
    ", Total  sell%:" , (slot.total_sellqty != 0 ? slot.fill_sellqty*100/slot.total_sellqty : 0.0d) ,\
    ", Total  buy% :" , (slot.total_buyqty  != 0 ? slot.fill_buyqty*100/slot.total_buyqty   : 0.0d) ,\
    ", Rejected:"     , slot.reject_count ,\
    ", Accepted:"     , slot.accept_count ,\
    ", Canceled:"     , slot.cancel_count ,\
    ", Orders  :"     , slot.total_count  ,\
    ", Fills   :"     , slot.fill_count   ,\
    ", Partials:"     , slot.partial_count ,\
    ", Filled  :"     , slot.filled_count  ,\
    ", In-flight: "    , inflight);

}

void Ostore::print_slot(int64_t now, const Slot & slot)
{
    using namespace ::fastapp::utils;

    FMPRINT( KYEL , ::fastapp::Micro(now));

    ::fastapp::ouch::print_slot(slot);

    FMPRINT( ", UnkRejects:"   , reject_count , KNRM , "\n");
    slot.last_print = now;
}


void Ostore::print(int64_t now, const Slot & slot)
{
    if(!time2print(now, slot)) return;
    print_slot(now, slot);           
}


Slot * Ostore::allocate(int16_t  instid, const std::string & sym)
{
    if ((ssize_t)_books->size() < instid)
    {
        _books->resize(instid + 1);
    }

    Slot & slot = _books->at(instid);
    slot.instid = instid;
    slot.sym    = sym;

    return &slot;
}

int32_t  Ostore::submit_cancelorder(Slot * slot)
{
    OrderID oid;
    oid.book = slot->instid;
    oid.oid  = slot->total_count + slot->total_replace_count + slot->total_cxl_count;

    slot->total_cxl_count ++;
    _all.total_cxl_count ++;

    return *reinterpret_cast<int32_t *>(&oid);
}

bool Ostore::submit_order(int16_t  instid,  NewOrder & buy, NewOrder & sell)
{
    static_assert( sizeof(OrderID) == sizeof(::fastapp::ouch::clord_id_t), "OrderID != clordid");
    assert(instid > 0);        
    
    int64_t now = ::fastapp::timeOfDay() ;
    
    //FMPRINT( KRED , " ; book.size:" , (ssize_t)_books->size() , " ; instid:", instid , KNRM , "\n");
    if ((ssize_t)_books->size() < instid) 
    { 
        _books->resize(instid + 1); 
    }

    //FMPRINT( KRED , " ; _book.at" , KNRM , "\n");
    Slot & slot = _books->at(instid);
    if (slot.instid != instid) 
    {
        slot.instid = instid;
        //FMPRINT( KRED , " ; clear" , KNRM , "\n");
        slot.sym.clear();
        //FMPRINT( KRED , " ; assign" , KNRM , "\n");
        slot.sym = std::string((const char *)((void *)&(buy.ccypair)), 7);
    }

    //FMPRINT( KRED , " ; time2submit" , KNRM , "\n");
    if (!time2submit(now, slot)) return false;

    //FMPRINT( KRED , " ; qty negative or 0=" , (!(slot.buy.qty > 0 && slot.sell.qty > 0)), KNRM , "\n");
    if (!(buy.qty > 0 && sell.qty > 0) ) return false;
   
    if (slot.buy.clordid != 0)
    {
        if (slot.buy.qty != slot.buy.cum_qty)
        {
            return false;
        }
    }

    if (slot.sell.clordid != 0)
    {
        if (slot.sell.qty != slot.sell.cum_qty)
        {
            return false;
        }
    }
    //FMPRINT( KRED , " prep ords" , KNRM , "\n");
    
    OrderID oid;
    oid.book = instid;
    oid.oid  = slot.total_count;
    //FMPRINT( KRED , " ; before buy" , KNRM , "\n");
    slot.total_count ++;
    _all.total_count ++;

    slot.buy.qty     = buy.qty;
    slot.buy.clordid = *reinterpret_cast<int32_t *>(&oid);
    buy.clordid      = slot.buy.clordid;
    slot.buy.rate    = buy.rate;
    slot.buy.cum_qty = 0;
    slot.buy.side    = buy.side;        
    slot.total_buyqty    += buy.qty;

    oid.oid           = slot.total_count;
    slot.total_count ++;
    _all.total_count ++;
    //FMPRINT( KRED , " ; before sell" , KNRM , "\n");
    slot.sell.qty     = sell.qty;
    slot.sell.clordid = *reinterpret_cast<int32_t *>(&oid);
    sell.clordid      = slot.sell.clordid;
    slot.sell.rate    = sell.rate;
    slot.sell.cum_qty = 0;
    slot.sell.side    = sell.side;     

    slot.total_sellqty    += sell.qty;
    
    slot.last = now;
    
    return true;
}

Slot & Ostore::getValidSlot(int16_t  instid,  ccypair_int_t ccypair)
{
    //FMPRINT( KRED , " ; book.size:" , (ssize_t)_books->size() , " ; instid:", instid , KNRM , "\n");
    if ((ssize_t)_books->size() < instid)
    {
        _books->resize(instid + 1);
    }

    //FMPRINT( KRED , " ; _book.at" , KNRM , "\n");
    Slot & slot = _books->at(instid);
    if (slot.instid != instid)
    {
        slot.instid = instid;
        //FMPRINT( KRED , " ; clear" , KNRM , "\n");
        slot.sym.clear();
        //FMPRINT( KRED , " ; assign" , KNRM , "\n");
        slot.sym = std::string((const char *)((void *)&(ccypair)), 7);
    }
    return slot;
}

bool Ostore::submit_order(int16_t  instid,  NewOrder & ord)
{
    static_assert( sizeof(OrderID) == sizeof(::fastapp::ouch::clord_id_t), "OrderID != clordid");
    assert(instid > 0);

    int64_t now = ::fastapp::timeOfDay() ;

    Slot & slot = getValidSlot(instid, ord.ccypair);

//    if (!time2submit(now, slot)) return false;

     Order & o = ord.side == eSide::Sell ? slot.sell : slot.buy;
    //FMPRINT( KRED , " ; (qty > 0)=" , (o.qty > 0 ), KNRM , "\n");
    //FMPRINT( KRED , " ; clordid=" , o.clordid , " qty=" , o.qty , " cum_qty=" , o.cum_qty , KNRM , "\n");

    if (o.clordid != 0)
    {
        if (o.qty > 0 && o.qty < o.cum_qty)
        {
            return false;
        }
    }
    //FMPRINT( KRED , " prep ords" , KNRM , "\n");

    OrderID oid;
    oid.book = instid;
    oid.oid  = slot.total_count + slot.total_replace_count + slot.total_cxl_count;

    slot.total_count ++;
    _all.total_count ++;
    //FMPRINT( KRED , " ; before sell" , KNRM , "\n");
    o.qty     = ord.qty;
    o.clordid = *reinterpret_cast<int32_t *>(&oid);
    ord.clordid      = o.clordid;
    o.rate    = ord.rate;
    o.cum_qty = 0;
    o.side    = ord.side;

    if (ord.side == eSide::Sell)
    {
        slot.total_sellqty    += ord.qty;
    }
    else
    {
        slot.total_buyqty    += ord.qty;
    }

    slot.last = now;

    return true;
}

bool Ostore::submit_replace(int16_t  instid,  ReplaceOrder & req,  ::fastapp::ouch::eSide side)
{
    static_assert( sizeof(OrderID) == sizeof(::fastapp::ouch::clord_id_t), "OrderID != clordid");
    assert(instid > 0);

    int64_t now = ::fastapp::timeOfDay() ;

    Slot & slot = getValidSlot(instid, req.ccypair);

    //if (!time2submit(now, slot)) return false;

     Order & ord = side == eSide::Sell ? slot.sell : slot.buy;
//     FMPRINT( KRED , "side: ", (side == eSide::Sell ? " Sell " : " Buy ")
//             , " ; qty ="    , o.qty     , " ; clordid=" , o.clordid
//             , " ; cum_qty=" , o.cum_qty , KNRM , "\n");

    //FMPRINT( KRED , " ; qty negative or 0=" , (!(slot.buy.qty > 0 && slot.sell.qty > 0)), KNRM , "\n");
    if (ord.clordid == 0) return false;
//    if (o.clordid != 0)
//    {
//        if (o.qty <= o.cum_qty)
//        {
//            return false;
//        }
//    }
    // if (o.qty > 0 ) return false;



    //FMPRINT( KRED , " prep ords" , KNRM , "\n");

    OrderID oid;
    oid.book = instid;
    oid.oid  = slot.total_count + slot.total_replace_count + slot.total_cxl_count;
    slot.total_replace_count ++;
    _all.total_replace_count ++;
    //FMPRINT( KRED , " ; before sell" , KNRM , "\n");


    Order & o = slot.pordrs[oid.oid % slot.pordrs.size()];
    o.qty     = req.qty;
    o.clordid = *reinterpret_cast<int32_t *>(&oid);
    o.rate    = req.rate;
    o.cum_qty = 0;
    o.side    = side;

    req.clordid = o.clordid;

    slot.last = now;

    return true;
}
} //namespace 

} //namespace
