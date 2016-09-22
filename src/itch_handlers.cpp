
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

#include <iostream>

#include <src/defs.h>
#include <src/socketutils.h>
#include <src/itch_handlers.h>
#include <src/itch/book.h>
#include <src/connection.h>
#include <src/print.h>

namespace fastapp
{
 
namespace itch
{
    
namespace callbacks
{



void printErrorCode(int16_t errorCode)
{
    using namespace ::fastapp::itch;
    switch((eErrorCode)errorCode)
    {
        case eErrorCode::NoError:
           FMPRINT( "NoError");
           break;
        case eErrorCode::InvalidMessageType:
           FMPRINT( "Invalid MessageType");
           break;
        case eErrorCode::InvalidMessageLength:
           FMPRINT( "Invalid MessageLength");
           break;
        case eErrorCode::InvalidInstrument:
           FMPRINT( "Invalid Instrument" );
           break;
        case eErrorCode::OtherError:
           FMPRINT( "Other Error" );
           break;
        case eErrorCode::NotAuthorized:
           FMPRINT("Unauthorized");
           break;
        case eErrorCode::UnsupportedInstrument:
           FMPRINT("Instrument not supported");
           break;
        case eErrorCode::UnsupportedVersion:
           FMPRINT("Version not supported");
           break;
        case eErrorCode::DuplicateRequestID:
           FMPRINT("Duplicate request ID ");
           break;
        case eErrorCode::SubscriptinAlreadyActive:
           FMPRINT("Subscription already active");
           break;
        case eErrorCode::ExchangeNotOpen:
           FMPRINT("Exchange is closed");
           break;
        case eErrorCode::InvalidActionType:
           FMPRINT( "Invalid action type");
           break;
        case eErrorCode::InvalidSubscriptionType:
           FMPRINT("Invalid subscription type");
           break;
        case eErrorCode::InvalidDepth:
           FMPRINT( "Invalid depth type");
           break;
        case eErrorCode::UnauthorizedSession:
            FMPRINT( "unauthorized session");
        default:
            FMPRINT( "unknown error code");
            break;
    }    
}

void Handlers::registerItchCallbacks()
{
    Handlers::registerItchCallbacks(_itchd, _sbd, _session);
}

void Handlers::registerItchCallbacks(::fastapp::itch::Dispatcher & itchd, ::fastapp::soupbin::Dispatcher & sbd, ConnectionBase * session)
{

//    itchd._funcBookUpdate = [&] (const ::fastapp::itch::BookUpdate & msg, uint64_t seq)
//    {
//        std::cerr , "BookUpdate:" , msg.tag , "\n";
//    };

      FMPRINT( KNRM , "Register Itch callbacks" , KNRM , "\n");
/*    d._funcSeq   = [&] (const ::fastapp::soupbin::SequencedDataHeader & data, int32_t size, uint64_t seq)
    {
        bool trace = IsTraceOn ? IsTraceOn() : false;
        if (trace)
        {
            uint64_t tod = ::fastapp::utils::getTimeOfDay() ;
            ::fastapp::utils::formatTime(tod, std::cerr);  
            std::cerr , " :Itchd SequencedDataHeader seq: " , seq << " : size ("<< size << ") data.size: (" << data.size << ")";
        }
        
        if (size > (int32_t)sizeof(::fastapp::soupbin::SequencedDataHeader) && size >= data.size)
        {
            if (trace) std::cerr , " : Itchd SequencedData : size (" , size - (int32_t)sizeof(::fastapp::soupbin::SequencedDataHeader) << ") : ";
            
            itchd.process
            (
                (void *)(reinterpret_cast<const char *>(&data) + sizeof(::fastapp::soupbin::SequencedDataHeader)) ,
                size - (int32_t)sizeof(::fastapp::soupbin::SequencedDataHeader), seq
            );
        }
        
        if (trace) std::cerr << "\n";
    };
*/
  
    using namespace ::fastapp::itch;

    // register callback to store sessionID in current connection
    // call is executed on accept
    if (session != nullptr)
    {
        reg ( [session] (const std::string & sessionID) { session->setSessionID(sessionID); } );
    }
   
    itchd.sub
    ( [&] (int64_t timeNow, const BookUpdate & bu, int32_t size) //_book_update_start =
    {
        if (bu.instrumentID > 512)
        {
            std::stringstream ss;
            ss << KRED << "Invalid instrument ID; BookUpdate [type :'"  << bu.type
            << "' : instrumentId='"  << bu.instrumentID
            << "' : blockCount='"    << (int)bu.updateMsgCount
            << "' :"  << " seqNum=(" << bu.sequenceNumber
            << ") + 1" <<" : toatl_size="     << size
            << "]" << KNRM << "\n";
            const std::string && s = ss.str();
            FMPRINT( s);
            return (fastapp::itch::Book*)nullptr;
        }

        Book & _book = _books.at(bu.instrumentID);

        if (!_book.update(timeNow, bu.sequenceNumber, bu.instrumentID) )
        {
            std::stringstream ss;
            ss << "Invalid Sequence number for " << _book.getSymbol() <<  " BookUpdate [type :'"  << bu.type
            << "' : instrumentId='"  << bu.instrumentID
            << "' : blockCount='"    << (int)bu.updateMsgCount
            << "' :" << KRED << " seqNum=("         << bu.sequenceNumber
            << ") != prevSeqNum("     << _book.getPrevSeqNum()
            << ") + 1"<< KNRM <<" : toatl_size="     << size
            << "]; clear book!!!"  << "\n";
            const std::string && s = ss.str();
            FMPRINT( s);

            sendEmail("sequencegap", std::string("sequence gap") , s);

            _book.clear();
        }
        return &_book;
    }
    );

    ::fastapp::itch::Dispatcher::FuncBookEvent prevBook;
    itchd.get(prevBook);// chain book handlers

    itchd.sub( [this, prevBook] (void * book, const ::fastapp::itch::BookUpdate & bu)
    {
        if (book == 0) return;

        ::fastapp::itch::Book & _book = *(::fastapp::itch::Book *)book;
        _book.allign_and_check();

        if ( (IsBookOn ? IsBookOn() : true) )
        {
            _book.print_and_check(bu.instrumentID);
        }

        if (prevBook) prevBook(book, bu);
    });

    itchd.sub([&] (const ::fastapp::itch::PriceAdd   & msg, uint64_t seq, void * book_ptr)
    {

        if (book_ptr == 0) return;

        ::fastapp::itch::Book & book = *(::fastapp::itch::Book *)book_ptr;

        if (IsBookOn() && IsTraceOn())
        {
            using namespace fastapp;
            uint64_t tod = ::fastapp::utils::getTimeOfDay() ;
                    
            FMPRINT(Micro(tod) ," - PriceAdd: " , msg.tag , " updateId:" , msg.priceUpdateID , ", qty:" , msg.qty/100 , ", minQty:" , msg.minQty/100 , ", rate:" , msg.rate/100000 , "." ,  msg.rate%100000 , ", side:");

            switch(msg.side)
            {
                case '1': FMPRINT( 'B');
                break;
                case '2': FMPRINT( 'S');
                break;
                default:
                          FMPRINT( 'U');
                break;
            }

            FMPRINT( ", maxDelay:" , msg.maxDelay  , "\n");
        }

        book.add(msg);
    });
        
    itchd.sub( [&] (const ::fastapp::itch::PriceCancel & msg, uint64_t seq, void * book_ptr)
    {
        if (book_ptr == 0) return;

        ::fastapp::itch::Book & book = *(::fastapp::itch::Book *)book_ptr;

        if (IsBookOn() && IsTraceOn())
        {
            uint64_t tod = ::fastapp::utils::getTimeOfDay() ;
            using namespace fastapp;
            FMPRINT( Micro(tod), " - PriceCancel: ", msg.tag , " updateId:" , msg.priceUpdateID, "\n");
        }

        book.remove(msg);

    });
    
    itchd.sub( [&] (const ::fastapp::itch::TradeUpdate & msg, uint64_t seq)
    {
        uint64_t tod = ::fastapp::utils::getTimeOfDay() ;
        using namespace fastapp;        
        FMPRINT(KYEL,  Micro(tod), " - TradeUpdate:" , msg.tag , " sym:" , getSymbol(msg.instrumentID) , " instrumentId: " , msg.instrumentID , ", qty:" , msg.qty/100 , ", rate:" , msg.rate/100000 ,"." ,  msg.rate%100000 , ", transTime:" ,msg.transTime , KNRM , "\n");
    }); 
    
    itchd.sub( [&] (const ::fastapp::itch::Midpoint & msg, uint64_t seq)
    {
        uint64_t tod = ::fastapp::utils::getTimeOfDay() ;

        using namespace fastapp;
        FMPRINT(KCYN, Micro(tod), " - Midpoint:" , msg.tag , " sym:" , getSymbol(msg.instID) , " instrumentId:" , msg.instID , ", rate:" , msg.rate/100000 ,"." ,  msg.rate%100000 , KNRM , "\n");
    });
    
    itchd.sub( [&] (const ::fastapp::itch::OnClose & msg, uint64_t seq)
    {
        uint64_t tod = ::fastapp::utils::getTimeOfDay() ;
        using namespace fastapp; 
        FMPRINT( Micro(tod) , " - OnClose:" , msg.tag , " sym:" , getSymbol(msg.instID)  , " instrumentId:"  , msg.instID , ", rate:" , msg.rate/100000 ,"." ,  msg.rate%100000 , "\n");
    });
    
    itchd.sub( [&] (const ::fastapp::itch::SubscriptionRequest & msg, uint64_t seq)
    {
        uint64_t tod = ::fastapp::utils::getTimeOfDay() ;
        using namespace fastapp;
        FMPRINT(\
        Micro(tod), " - SubscriptionRequest: '" \
        , msg.type  , "' : sym='" \
        , msg.instSym \
        , "', updateType='" \
        , msg.updateType ,  "', reqID='" \
        , msg.reqID      , "', sessionID='" \
        , std::string(msg.sessionID, ::fastapp::soupbin::session_size)  , "', actionTyoe='" \
        , msg.actionType , "', subType='" \
        , msg.subType    , "'" );
    });
    
    itchd.sub([&] (const ::fastapp::itch::SubscriptionResponse & msg, uint64_t seq)
    {
        uint64_t tod = ::fastapp::utils::getTimeOfDay() ;
        using namespace fastapp;
        FMPRINT( Micro(tod)\
        , " - SubscriptionResponse: '" \
        , msg.type       , "' sym='"\
        , std::string(msg.instSym, ::fastapp::itch::sym_size)    , "', instrumentID='"\
        , msg.instID     , "', reqID='"\
        , msg.reqID      , "', sessionID='"\
        , std::string(msg.sessionID, ::fastapp::soupbin::session_size)  , "', status='" \
        , msg.status     , "', errorCode='"\
        , msg.errorCode  , "'"\
        , "\n");
        
        
        if (getSymbol(msg.instID).empty())
        {
            setSymbol(msg.instID, std::string(msg.instSym, ::fastapp::itch::sym_size) );
        }
        else if (strncmp(msg.instSym, getSymbol(msg.instID).c_str(), ::fastapp::itch::sym_size) != 0)
        {
            FMPRINT("\n" , KRED , "<invalid symbol:'" , std::string(msg.instSym, ::fastapp::itch::sym_size) , "'!='" , getSymbol(msg.instID) , "'>" , KNRM);
        }
        
        if (msg.errorCode != '0')
        {
            using namespace ::fastapp::itch;
            FMPRINT("\n" , KRED , "<invalid subscription:'" , msg.errorCode , "," );
            
            printErrorCode((int16_t)msg.errorCode);
            
            FMPRINT(">" , KNRM , "\n");
        }


       // create book
        auto & subResp = msg;
        if ((uint32_t)subResp.instID < _books.size())
        {
            std::string sym = std::string(subResp.instSym, fastapp::itch::sym_size);

            if (!sym.empty())
            {
                ssize_t len = ::strlen(sym.c_str());
                if (len < fastapp::itch::sym_size)
                {
                    sym.resize(len);
                }
            }

            if (subResp.errorCode == '0' && !sym.empty())
            {
                setSym(subResp.instID,  sym);
            }
            else
            {
                clearBook(subResp.instID);
            }
        }
       // end-of-book code
    });
    
    ::fastapp::itch::Dispatcher::FuncInstrumentInfoMessage prevInfo;
    itchd.get(prevInfo); // chain info message handlers
    
    itchd.sub( [this, prevInfo ] (const ::fastapp::itch::InstrumentInfoMessage & msg, uint64_t seq)
    {
        using namespace fastapp;
        int64_t tod = ::fastapp::utils::getTimeOfDay() ;
        FMPRINT( Micro(tod)\
        , " - InstrumentInfoMessage:" \
        , msg.type , ", sym='"\
        , std::string(msg.instSym, ::fastapp::itch::sym_size)       , "', instrumentID='" \
        , msg.instID , "', instrumentType='"\
        , msg.instType      , "', setlementDate='"\
        , msg.setlmentDate  , "'" , "\n");
        
        // symbols[0]

        if ((size_t)msg.instID < _books.size())
        {
            std::string sym = std::string(msg.instSym, fastapp::itch::sym_size);
            if (!sym.empty())
            {
                ssize_t len = ::strlen(sym.c_str());
                if (len < fastapp::itch::sym_size)
                {
                    sym.resize(len);
                }

                setSym(msg.instID,  sym);
            } // will clear book
        }
        
        if (prevInfo) prevInfo(msg, seq); 
    });
    
    itchd.sub( [&] (const ::fastapp::itch::InstrumentListRequest & msg, uint64_t seq)
    {
        uint64_t tod = ::fastapp::utils::getTimeOfDay() ;
        using namespace fastapp;        
        FMPRINT( Micro(tod), " - InstrumentListRequest:"\
        , msg.type , ", sym=' "\
        , std::string(msg.instSym, ::fastapp::itch::sym_size)    , "', sessionID='" \
        , std::string(msg.sessionID, ::fastapp::soupbin::session_size)  , "'" , "\n");
    });
    
    itchd.sub( [&] (const ::fastapp::itch::Reject & msg, uint64_t seq)
    {
        uint64_t tod = ::fastapp::utils::getTimeOfDay() ;

        FMPRINT( KRED, Micro(tod)," - Reject:"\
        , msg.type , "' , messageType="\
        , msg.messageType , "', rejectCode=" \
        , msg.rejectCode  , "', rejectMessage='"\
        , std::string(msg.rejectMessage, ::fastapp::itch::rejectmsg_len)  , "', ");
        printErrorCode(msg.rejectCode);
        FMPRINT( KNRM ,"\n");
    });
}

} // callbacks
}
} //test
