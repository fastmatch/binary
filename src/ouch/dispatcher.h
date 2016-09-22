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


#ifndef OUCH_DISPATCHER_H__
#define OUCH_DISPATCHER_H__


#include <src/ouch/ouch.h>

#include <functional>
#include <src/defs.h>
#include <src/helpers.h>

#include <src/print.h>

namespace fastapp
{
    
namespace ouch
{ 
    
class InvalidData : public std::runtime_error
{
public:
    explicit InvalidData( const std::string& msg )
        :   std::runtime_error( msg )
    {}
};

class Dispatcher
{
public:
    typedef std::function< void   (const NewOrderAck  &, uint64_t seq) >      FuncNewOrderAck;
    typedef std::function< void   (const CancelOrderReject &, uint64_t seq) > FuncCancelOrderReject;
    typedef std::function< void   (const ReplaceAck   &, uint64_t seq) >      FuncReplaceAck;
    typedef std::function< void   (const OutOrder     &, uint64_t seq) >      FuncOutOrder;
    typedef std::function< void   (const Trade        &, uint64_t seq) >      FuncTrade;
    typedef std::function< void   (const Reject       &, uint64_t seq) >      FuncReject;
    
    #pragma pack(push, 1)
    struct Blocks
    {
        int32_t size;
        int8_t  block_count;
    };
    #pragma pack(pop)
    
public:
    
    void process (void * ptr, int32_t size, uint64_t seq);   
    void dispatch(const Header * ptr, int32_t size, uint64_t seq);

    
    Dispatcher ()
    {
    }
    
    Dispatcher (const Dispatcher & d) = default;
    Dispatcher (Dispatcher && d) = default;
    Dispatcher & operator = (Dispatcher && d) = default;
    Dispatcher & operator = (Dispatcher & d) = default;
    ~Dispatcher () = default;
    
    
    void setEmailCallback(fastapp::callbacks::send_email_t email)
    {
        sendEmail = email;
    }    
    

    
    fastapp::callbacks::is_func_t                  IsTraceOn;
    fastapp::callbacks::is_func_t                  IsOuchOn;
    fastapp::callbacks::send_email_t               sendEmail;
    
public:

    FuncNewOrderAck            _funcNewOrderAck; 
    FuncCancelOrderReject      _funcCancelOrderReject;
    FuncReplaceAck             _funcReplaceAck;
    FuncOutOrder               _funcOutOrder;
    FuncTrade                  _funcTrade;
    FuncReject                 _funcReject;
    

};
        
inline void Dispatcher::process(void * ptr, int32_t size, uint64_t seq)
{
    if (size == 0) 
    {
        FMPRINT( "skipping; size=0! \n");
        return;
    }
    
    Header * hdr = reinterpret_cast<Header *>(ptr);
    if (IsTraceOn ? IsTraceOn() : true)
    {
        FMPRINT( (int)hdr->type , "' : msgType(char)='", hdr->type ,"' : timestamp=", \
               hdr->timestamp , "' : strmid='" , hdr->streamid , "' : ouch msg size=" , \
               size , "' : seq=" , seq , "'", "\n");
    }
        
     dispatch(hdr, size, seq);
    
}
inline void Dispatcher::dispatch(const Header * ptr, int32_t size, uint64_t seq)
{
    if (size == 0) 
    {
        return;
    }
    
    switch( ptr->type )
    {
    case NewOrderAck::tag:
        if (_funcNewOrderAck) _funcNewOrderAck(*reinterpret_cast<const NewOrderAck *>(ptr), seq );
        break;
    case CancelOrderReject::tag:
        if (_funcCancelOrderReject) _funcCancelOrderReject(*reinterpret_cast<const CancelOrderReject *>(ptr), seq );
        break;
    case ReplaceAck::tag:
        if (_funcReplaceAck) _funcReplaceAck(*reinterpret_cast<const ReplaceAck *>(ptr), seq );
        break;
    case OutOrder::tag:
        if (_funcOutOrder) _funcOutOrder(*reinterpret_cast<const OutOrder *>(ptr), seq );
        break;
    case Trade::tag:
        if (_funcTrade) _funcTrade(*reinterpret_cast<const Trade *>(ptr), seq );
        break;
    case Reject::tag:
    {            
        if (_funcReject) _funcReject(*reinterpret_cast<const Reject *>(ptr), seq );  
    } ; break;
        
    default:
    {
         FMPRINT( ";", ::fastapp::Hex(ptr, size), "\n");
    } ; break;
    }
    

}

} // soupbin
} // fastapp

#endif

