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

#ifndef ITCH_DISPATCHER_H__
#define ITCH_DISPATCHER_H__


#include <src/itch/itch.h>
#include <functional>
#include <src/defs.h>
#include <src/itch/book.h>

namespace fastapp
{
    
namespace itch
{ 
    
class InvalidData : public std::runtime_error
{
public:
    explicit InvalidData( const std::string& msg )
        :   std::runtime_error( msg )
    {}
};




typedef std::function< int8_t (const BookUpdate  &, uint64_t seq) > FuncBookUpdate;
typedef std::function< void (const PriceAdd    &, uint64_t seq, void * book) > FuncPriceAdd;
typedef std::function< void (const PriceCancel &, uint64_t seq, void * book) > FuncPriceCancel;
typedef std::function< void (const TradeUpdate &, uint64_t seq) > FuncTradeUpdate;
typedef std::function< void (const Midpoint    &, uint64_t seq) > FuncMidpoint;
typedef std::function< void (const OnClose     &, uint64_t seq) > FuncOnClose;
typedef std::function< void (const SubscriptionRequest &, uint64_t  seq) > FuncSubscriptionRequest;
typedef std::function< void (const SubscriptionResponse&, uint64_t  seq) > FuncSubscriptionResponse;
typedef std::function< void (const InstrumentListRequest&, uint64_t seq) > FuncInstrumentListRequest;
typedef std::function< void (const InstrumentInfoMessage&, uint64_t seq) > FuncInstrumentInfoMessage;
typedef std::function< void (const Reject      &, uint64_t seq) > FuncReject;
typedef std::function< void (void * book, const ::fastapp::itch::BookUpdate & msg) > FuncBookEvent; 
typedef std::function< void * (int64_t timeNow, const BookUpdate & bu, int32_t  size) > FuncBookUpdStartEvent; //returns pointer to book object


class Dispatcher
{
public:
    typedef ::fastapp::itch::FuncBookUpdate   FuncBookUpdate;
    typedef ::fastapp::itch::FuncPriceAdd     FuncPriceAdd;
    typedef ::fastapp::itch::FuncPriceCancel  FuncPriceCancel;
    typedef ::fastapp::itch::FuncTradeUpdate  FuncTradeUpdate;
    typedef ::fastapp::itch::FuncMidpoint     FuncMidpoint;
    typedef ::fastapp::itch::FuncOnClose      FuncOnClose;
    typedef ::fastapp::itch::FuncSubscriptionRequest    FuncSubscriptionRequest;
    typedef ::fastapp::itch::FuncSubscriptionResponse   FuncSubscriptionResponse;
    typedef ::fastapp::itch::FuncInstrumentListRequest  FuncInstrumentListRequest;
    typedef ::fastapp::itch::FuncInstrumentInfoMessage  FuncInstrumentInfoMessage;
    typedef ::fastapp::itch::FuncReject       FuncReject;
    typedef ::fastapp::itch::FuncBookEvent    FuncBookEvent;
    typedef ::fastapp::itch::FuncBookUpdStartEvent FuncBookUpdStartEvent; 
    #pragma pack(push, 1)
    struct Blocks
    {
        int32_t size;
        int8_t  block_count;
    };
    #pragma pack(pop)
    
public:
    
    void process (void * ptr, int32_t size, uint64_t seq);   
    Blocks dispatch(void * ptr, int32_t size, uint64_t seq);
    void processBookUpdate(const BookUpdate & bookUpdate, int32_t & size, uint64_t seq, int8_t & block_count);
    
    Dispatcher ()
    : IsTraceOn([](){return false;})
    , IsBookOn([](){return false;})
    , sendEmail([](const std::string &a, const std::string &b, const std::string &c){})
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
   
    void reg_trace(fastapp::callbacks::is_func_t trace)
    {    IsTraceOn = trace;    }

    void reg_book(fastapp::callbacks::is_func_t book)
    {    IsBookOn = book;    }

    void reg_email(fastapp::callbacks::send_email_t f)
    {    sendEmail = f;    }

    fastapp::callbacks::is_func_t  get_trace_callback()
    {    return IsTraceOn;    }

    fastapp::callbacks::is_func_t get_book_callback()
    {    return IsBookOn;    }

    fastapp::callbacks::send_email_t get_email_callback()
    {    return sendEmail;    }


    void sub(FuncBookUpdate f)
    {
        _funcBookUpdate = f;
    }

    void sub(FuncPriceAdd f)
    {    _funcPriceAdd = f;    }

    void get(FuncPriceAdd & f)
    {    f = _funcPriceAdd;    }


    void sub(FuncPriceCancel f)
    {    _funcPriceCancel = f;    }

    void get(FuncPriceCancel & f)
    {    f = _funcPriceCancel;    }


    
    void sub(FuncTradeUpdate  f)
    {    _funcTradeUpdate = f;   }
    void get(FuncTradeUpdate & f)
    {    f = _funcTradeUpdate;   }


    void sub(FuncMidpoint  f)
    {    _funcMidpoint = f; }

    void sub(FuncSubscriptionRequest  f)
    {    _funcSubReq = f;    }

    void sub(FuncSubscriptionResponse f)
    {    _funcSubResp = f;    }

    void sub(FuncInstrumentListRequest  f)
    {    _funcListRequest = f;    }

    void sub(FuncInstrumentInfoMessage  f)
    {    _funcInfoMsg = f;    }

    void sub(FuncReject  f)
    {    _funcReject = f;    }

    void sub(FuncOnClose  f)
    {    _funcOnClose = f;    }

    void sub(FuncBookEvent  f)
    {
        _book_event = f;    
    }

    void sub(FuncBookUpdStartEvent f)
    {
        _book_update_start   = f;    
    }

    void get(FuncMidpoint & f)
    {    f = _funcMidpoint; }

    void get(FuncSubscriptionRequest & f)
    {    f = _funcSubReq;    }

    void get(FuncSubscriptionResponse &f)
    {    f = _funcSubResp;    }

    void get(FuncInstrumentListRequest & f)
    {    f = _funcListRequest;    }

    void get(FuncInstrumentInfoMessage  &f)
    {    f = _funcInfoMsg;    }

    void get(FuncReject & f)
    {    f = _funcReject;    }

    void get(FuncOnClose & f)
    {    f = _funcOnClose;    }

    void get(FuncBookEvent & f)
    {
         f = _book_event;
    }

    void get(FuncBookUpdStartEvent & f)
    {
        f = _book_update_start;
    }


    fastapp::callbacks::is_func_t                  IsTraceOn;
    fastapp::callbacks::is_func_t                  IsBookOn;
    fastapp::callbacks::send_email_t               sendEmail;
    
protected:        
    FuncBookUpdate             _funcBookUpdate;
    FuncPriceAdd               _funcPriceAdd;
    FuncPriceCancel            _funcPriceCancel;
    FuncTradeUpdate            _funcTradeUpdate;
    FuncMidpoint               _funcMidpoint;
    FuncSubscriptionRequest    _funcSubReq;    
    FuncSubscriptionResponse   _funcSubResp;
    FuncInstrumentListRequest  _funcListRequest;
    FuncInstrumentInfoMessage  _funcInfoMsg;
    FuncReject                 _funcReject;
    FuncOnClose                _funcOnClose;
    FuncBookEvent              _book_event;
    FuncBookUpdStartEvent      _book_update_start; 
};
        
inline void Dispatcher::process(void * ptr, int32_t size, uint64_t seq)
{
    if (size == 0) 
    {
        FMPRINT( "skipping; size=0! ",  "\n");
        return;
    }
    
    Header * hdr = reinterpret_cast<Header *>(ptr);
    if (IsTraceOn ? IsTraceOn() : true)
    {
        FMPRINT("BlockCount='" , (int)hdr->msgBlockCount, "' : strmid='",  hdr->streamid , "' : times=" ,hdr->timestamp, "\n");
    }
    
    const char * strm = reinterpret_cast<const char *>(ptr) + sizeof(Header);
    const char * prev_strm=strm;
    int32_t total_size = size - sizeof(Header);
    int32_t prev_total_size = total_size;
    int8_t block_count = hdr->msgBlockCount;
    if (block_count == 0)
    {
        block_count = 1;
        FMPRINT( KRED , "!!!!Invalid, ITCH BlockCount is 0! setting it to 1 " , KNRM , "\n");
    }
        
    for (int8_t i = 0; i < block_count; i ++)
    {
        Blocks ctrs = dispatch((void *)strm, total_size, seq);
        
        if (ctrs.size <= 0)
        {    break;    }
        
        i += ctrs.block_count;
        prev_strm = strm;
        strm += ctrs.size;
        if (total_size < 0 || total_size < ctrs.size) 
        {
            std::stringstream ss;
            ss << KRED << "!!!!SequencedPacket:Invalid data size!" << std::endl << "Last Msg Block read #" << (int)i 
               << "; type: " << *strm
               << "; length read: "      << ctrs.size 
               << "; out of still available: " << total_size
               << "; msg sequence #"     << seq 
               << "; total number of blocks: " << (int)hdr->msgBlockCount 
               << KNRM;
           
            throw InvalidData(ss.str() );
        }
        prev_total_size = total_size;
        total_size -= ctrs.size;
    }
    
    if (total_size > 0) 
    {
        std::stringstream ss;
        ss << KRED << "!!!!SequencedPacket:Invalid data size!" << std::endl 
           << "Last Msg Block read #" << (int)hdr->msgBlockCount - 1 
           << "; prev_block_type: " << *prev_strm
           << "; size left before last block: " << prev_total_size               
           << "; type: " << *strm
           << "; size left: " << total_size
           << "; out of available: " << size 
           << "; msg sequence #"     << seq 
           << "; total number of blocks: " << (int)hdr->msgBlockCount 
           << "; length read: "      << size - total_size
           << "; data read: " << std::endl;
           ::fastapp::utils::hex(ss, reinterpret_cast<const char *>(ptr), size - total_size);
        ss << std::endl <<"; unread part size: " <<  total_size
           << "; data left: " << std::endl;
           ::fastapp::utils::hex(ss, reinterpret_cast<const char *>(strm), total_size);
        ss << KNRM;

        FMPRINT(ss.str() , "\n");
        //throw InvalidData(ss.str() );
    }
    
}
inline Dispatcher::Blocks Dispatcher::dispatch(void * ptr, int32_t size, uint64_t seq)
{
    if (size == 0) 
    {
        FMPRINT("skipping; size=0! ",  "\n");
        return {-1,0};
    }
    
    Blocks props;

    props.block_count = 0;
    
    switch( *(reinterpret_cast<const char *>(ptr)) )
    {
    case BookUpdate::tag:
        props.size  =  size;
        processBookUpdate(*reinterpret_cast<const BookUpdate *>(ptr), props.size , seq,  props.block_count);
        break;

    case TradeUpdate::tag:
        props.size = sizeof(TradeUpdate);
        if (_funcTradeUpdate) _funcTradeUpdate(*reinterpret_cast<const TradeUpdate *>(ptr), seq );
        break;

    case Midpoint::tag:
        props.size = sizeof(Midpoint);
        if (_funcMidpoint) _funcMidpoint(*reinterpret_cast<const Midpoint *>(ptr), seq );  
        break;
        
    case OnClose::tag:
        props.size = sizeof(OnClose);
        if (_funcOnClose) _funcOnClose(*reinterpret_cast<const OnClose *>(ptr), seq );  
        break;
        
    case SubscriptionRequest::tag:
    {
        props.size = sizeof(SubscriptionRequest);
        if (_funcSubReq) _funcSubReq(*reinterpret_cast<const SubscriptionRequest *>(ptr), seq);
    } break;
    
    case SubscriptionResponse::tag:
    {
        props.size = sizeof(SubscriptionResponse);
        const SubscriptionResponse & subResp = *reinterpret_cast<const SubscriptionResponse *>(ptr);

        if (_funcSubResp) _funcSubResp(subResp, seq );
    } break;
        
    case InstrumentListRequest::tag:
        props.size = sizeof(InstrumentListRequest);
        if (_funcListRequest)
        {
            const InstrumentListRequest & req = *reinterpret_cast<const InstrumentListRequest *>(ptr);
            _funcListRequest(req, seq );
        }
        break;

    case InstrumentInfoMessage::tag:
    {
        props.size = sizeof(InstrumentInfoMessage);
        const InstrumentInfoMessage & msg = *reinterpret_cast<const InstrumentInfoMessage *>(ptr);
        if (_funcInfoMsg) _funcInfoMsg(msg, seq);  
    } ; break;

    case Reject::tag:
    {            
        props.size = sizeof(Reject);
        if (_funcReject) _funcReject(*reinterpret_cast<const Reject *>(ptr), seq );  
    } ; break;
        
    default:
    {
         FMPRINT( "Itch:Unknown type: " , *reinterpret_cast<const char*>(ptr) , ";" , ::fastapp::Hex(ptr, size) , "\n");
    } return {-1,0};
    }
    
    return props;
}

inline void Dispatcher::processBookUpdate(const BookUpdate & bu, int32_t & size, uint64_t seq, int8_t & block_count)
{
    if (size == 0) 
    {
        FMPRINT("skipping BookUpdate; size=0! " , "\n");
        return;
    }
    
    if ( (IsBookOn ? IsBookOn() : true) && (IsTraceOn ? IsTraceOn() :  true ) )
    {
        FMPRINT( "Book Update type :'", bu.type , "' : instrumentId='" , bu.instrumentID , "' : blockCount='" , (int)bu.updateMsgCount , "' : seqNum=" , bu.sequenceNumber , "' : toatl_size=", size , "\n");
    }
    
    const char * strm = reinterpret_cast<const char *>(&bu) + sizeof(BookUpdate);
    int32_t total_size = size - sizeof(BookUpdate);
    int64_t timeNow = ::fastapp::utils::getTimeOfDay();

    void * _book = _book_update_start(timeNow, bu, size);

    for (int8_t i = 0; i < bu.updateMsgCount; i ++)
    {
        //Blocks blocks = desipatch((void *)strm, total_size, seq);
        Blocks blocks={0,0};
       
        switch( *strm )
        {        
        case PriceAdd::tag:
             blocks.size = sizeof(PriceAdd);
             if (_funcPriceAdd)
             {
                 _funcPriceAdd(*reinterpret_cast<const PriceAdd *>(strm), seq, _book );
             }
             break;

        case PriceCancel::tag:
            blocks.size = sizeof(PriceCancel);
            if (_funcPriceCancel)
            { 
                _funcPriceCancel(*reinterpret_cast<const PriceCancel *>(strm), seq, _book );
            }
            break;   
            default: break;
        }
        
        
        if (blocks.size <= 0)
        {    break;    }
        
        i += blocks.block_count;
        
        strm += blocks.size;
        if (total_size < blocks.size) 
        {
            std::stringstream ss;
            ss << KRED << "!!!!BookUpdate: Invalid data size! Last Msg Block read #" << i 
               << "; length read: "      << blocks.size 
               << "; out of available: " << total_size 
               << "; msg sequence #"     << seq 
               << "; total number of blocks: " << (int)bu.updateMsgCount << KNRM;
            
            throw InvalidData(ss.str() );
        }
        
        total_size -= blocks.size;
    }
    
    size = (size - total_size);
    
    block_count = bu.updateMsgCount;
    
    if (_book_event)
    {
        _book_event(_book, bu);
    }
}

} // soupbin
} // fastapp

#endif
