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
#include <src/ouch_handlers.h>
#include <src/connection.h>
#include <src/helpers.h>
#include <src/print.h>

namespace fastapp
{
 
namespace ouch
{
    
namespace callbacks
{



void printRejectCode(eRejectReasonCode code)    
{
    using namespace ::fastapp::ouch;
    switch( code)
    {
        case eRejectReasonCode::NotAuthorized:
            FMPRINT( "Access is not authorized for used credentials" ,"\n");
            break;
        case eRejectReasonCode::SessionNotAvailable:
            FMPRINT( "Session is not available" , "\n");
            break;
            
        case eRejectReasonCode::InvalidVersion:
            FMPRINT( "Invalid version number"   , "\n");
            break;
            
        default:
            FMPRINT( "Unknown reject code: (int) '" , (int)code , "' ; (char) '" , (char)code , "'"   , "\n");
            break;
    }
}

void printErrorCode(eErrorCode errorCode)
{
    using namespace ::fastapp::ouch;
    switch(errorCode)
    {
        case eErrorCode::NoError:
            FMPRINT( "NoError"); 
            break;
        case eErrorCode::DuplicateOrderId:
            FMPRINT( "Duplicate ClOrdId from client");
            break;
        case eErrorCode::ExchangeNotOpen:
            FMPRINT( "Exchange not accepting orders temporarily");   
            break;
        case eErrorCode::InvalidQty:
            FMPRINT( "Invalid Quantity"); 
            break;
        case eErrorCode::InvalidRate:
            FMPRINT( "Invalid Rate");            
            break;
        case eErrorCode::UnknownStreamId:
            FMPRINT( "StreamId cannot be mapped to internal clientId");            
            break;
        case eErrorCode::InvalidAccount:
            FMPRINT( "Invalid account field");            
            break;
        case eErrorCode::UnknownSymbol:
            FMPRINT( "Symbol not supported");            
            break;
        case eErrorCode::InvalidSide:
            FMPRINT( "Invalid Side");            
            break;
        case eErrorCode::InvalidMinQty:
            FMPRINT( "Invalid MinQty (wither < 0 or > OrderQty)");            
            break;
        case eErrorCode::UnsupportedTIF:
            FMPRINT( "Invalid time in force");            
            break;
        case eErrorCode::InvalidMaxShow:
            FMPRINT( "Invalid max show (< 0 or > OrderQty)");            
            break;
        case eErrorCode::InvalidType:
            FMPRINT( "Invalid order type");            
            break;
        case eErrorCode::OrderNotFound:
            FMPRINT( "Order not found by ClOrdId");            
            break;
        case eErrorCode::TooLateToCancel:
            FMPRINT( "Order already filled or cancelled");            
            break;
        case eErrorCode::SymbolDisabled:
            FMPRINT( "Symbol temporarily disabled for trading");            
            break;
        case eErrorCode::TooManyOpenOrders:
            FMPRINT( "Client has too many open orders");            
            break;
        case eErrorCode::TradingSuspended:
            FMPRINT( "Client session is suspended");            
            break;
        case eErrorCode::OtherError:
            FMPRINT( "Any other error");            
            break;
        default:
            FMPRINT( "unknown error code: (int)=" , (int)errorCode , "' ; (char)='" , (char)errorCode , "'");
            break;
    }    
}



void Handlers::registerOuchCallbacks()
{
    /*
    _sbd.sub( [&] (const ::fastapp::soupbin::SequencedDataHeader & data, int32_t size, uint64_t seq)
    {
        bool trace = IsTraceOn ? IsTraceOn() : false;
        bool ouch  = IsOuchOn  ? IsOuchOn()  : false;
        if (trace && ouch )
        {
            int64_t tim = ::fastapp::utils::getTimeOfDay();
            FMPRINT( tim , " : SequencedDataHeader seq: " , seq , " : size (", size , ")");
        }
        
        if (size > (int32_t)sizeof(::fastapp::soupbin::SequencedDataHeader) && size >= data.size)
        {
            if (trace && ouch ) { FMPRINT( " : SequencedData : size (" , size - (int32_t)sizeof(::fastapp::soupbin::SequencedDataHeader) , ") : "); }
            
            ouchd.process
            (
                (void *)(reinterpret_cast<const char *>(&data) + sizeof(::fastapp::soupbin::SequencedDataHeader)) ,
                size - (int32_t)sizeof(::fastapp::soupbin::SequencedDataHeader), seq
            );
        }
        
        if (trace && ouch )
        {    FMPRINT( "\n");    }
    });
    */

    using namespace ::fastapp::ouch;

    ouchd._funcNewOrderAck  =  [&] (const ::fastapp::ouch::NewOrderAck   & msg, uint64_t seq)
    {
        uint64_t tod = ::fastapp::utils::getTimeOfDay() ;        

//        bool trace = IsTraceOn ? IsTraceOn() : false;
        bool ouch  = IsOuchOn  ? IsOuchOn()  : false;
        
        if (ouch)
        {
            //FMPRINT( ::fastapp::sockets::getTimeOfDay() 

            FMPRINT( Micro(tod) ,  (msg.status == eAckStatus::Reject ? KRED : KYEL)
            , " - NewOrderAck: '" , msg.type , "' clOrdID:" , msg.clordid
            , ", sym:" , std::string(sym_cstr(msg.ccypair), 7) , ", ordid:"
            , msg.ordid , ", ack.status:");
        }

        switch(msg.status)
        {
            case eAckStatus::Accept: 
            {
                if (ouch)
                {                    
                    FMPRINT( " 'Accept'");
                }

                if (_store != 0)
                {
                    _store->accept_order(msg.clordid);
                }                
            }
            break;
            case eAckStatus::Reject: 
            {
                if (ouch)
                {                    
                    FMPRINT( " 'Reject'");
                }

                if (_store != 0)
                {
                    _store->reject_order(msg.clordid);
                }                
            }
            break;
            default:
            {
                if (ouch)
                {                    
                    FMPRINT( "'Unknown'");
                }
            }
            break;
        }
            
        if (ouch)
        {                    
            FMPRINT( ", timestamp:" , msg.timestamp , ", strmid:'" , msg.streamid , "' err: ");
            printErrorCode(msg.err);
            FMPRINT( "'  " , KNRM , "\n");
        }
        
        if (_store != 0)
        {
            ::fastapp::ouch::Slot *slot = _store->find_slot_by_clordid(msg.clordid);
            if (slot != 0 && _store->time2print(tod, *slot) )
            {
                _store->print(tod, *slot);
            }
            
            _store->print_aggr_ext(tod);

            if (msg.status == eAckStatus::Reject && msg.err == eErrorCode::DuplicateOrderId)
            {
                CxlReqData creq;
                CancelOrder & co = getCancelOrderRequest(creq);

                co.ccypair = msg.ccypair;
                co.clordid = _store->submit_cancelorder(slot);
                co.origclordid = msg.clordid;

                _trade_con->send(creq, tod);
            }
        }


    };
        
    ouchd._funcCancelOrderReject = [&] (const ::fastapp::ouch::CancelOrderReject & msg, uint64_t seq)
    {
        uint64_t tod = ::fastapp::utils::getTimeOfDay() ;
//        bool trace = IsTraceOn ? IsTraceOn() : false;
        bool ouch  = IsOuchOn  ? IsOuchOn()  : false;
        if (ouch)
        {
            //FMPRINT( ::fastapp::utils::getTimeOfDay() 
            FMPRINT( Micro(tod) , KRED , " - CxlReject: '" , msg.type , "' clordid:'"\
                    , msg.clordid , "' orig.clordid:'" , msg.origclordid \
                    , "', timestamp:" , msg.timestamp , ", strmid:'" \
                    , msg.streamid , "'");
            printErrorCode (msg.err);
            FMPRINT( KNRM , "\n");
        }
        
//        if (_store != 0)
//        {
//            ::fastapp::ouch::Slot *slot = _store->find_slot(msg.clordid);
//            if (slot != 0 && _store->time2print(tod, *slot) )
//            {
//                _store->print(tod, *slot);
//            }
//
//            _store->time2print_aggr_ext(tod);
//        }


    };

    ouchd._funcReplaceAck = [&] (const ::fastapp::ouch::ReplaceAck & msg, uint64_t seq)
    {
//        bool trace = IsTraceOn ? IsTraceOn() : false;
        uint64_t tod = ::fastapp::utils::getTimeOfDay() ;

        ::fastapp::ouch::Slot *slot = 0;
        if (_store != 0)
        {
            slot = _store->find_slot_by_clordid(msg.clordid);
        }

        bool ouch  = IsOuchOn  ? IsOuchOn()  : false;        
        if (ouch)
        {
            FMPRINT( (msg.status != eReplaceStatus::Replaced ? KRED : KYEL) , Micro(tod) ,  " - ReplaceAck:'" \
            , msg.type , "' clordid:'" , msg.clordid , "' orig.clordid: '" , msg.origclordid , "', ");

            if (slot != 0)
            {
                FMPRINT( "sym=" , slot->sym , ", ");
            }
            FMPRINT( "status=" );
        }
        switch(msg.status)
        {
        case eReplaceStatus::Cancelled:
            if (ouch) FMPRINT( " 'Cancelled' ");
            if (_store != 0)
            {
                //_store->reject_replace(msg.clordid, msg.origclordid);
                _store->cancel_replace(msg.clordid, msg.origclordid);
            }
            break;

        case eReplaceStatus::Replaced:
            if (ouch) FMPRINT( " 'Replaced' ");
            if (_store != 0)
            {
                _store->accept_replace(msg.clordid, msg.origclordid);

            }
            break;

        case eReplaceStatus::Rejected:
            if (ouch) FMPRINT( " 'Reject' ");
            if (_store != 0)
            {
                _store->reject_replace(msg.clordid, msg.origclordid);
            }
            break;
        default:
            if (ouch) FMPRINT( "'Unknown' ");
            break;
        }

        if (ouch)
        {
            FMPRINT( ", timestamp:" , msg.timestamp , ", strmid:'" , msg.streamid , "' err: ");
            printErrorCode(msg.err);
            FMPRINT( "'  " , KNRM , "\n");
        }

        if (_store != 0)
        {
            if (slot != 0 && _store->time2print(tod, *slot) )
            {
                _store->print(tod, *slot);
            }

            _store->print_aggr_ext(tod);
        }


    }; 
    
    ouchd._funcOutOrder    = [&] (const ::fastapp::ouch::OutOrder & msg, uint64_t seq)
    {
        uint64_t tod = ::fastapp::utils::getTimeOfDay() ;        
//        bool trace = IsTraceOn ? IsTraceOn() : false;
        bool ouch  = IsOuchOn  ? IsOuchOn()  : false;        
        if (ouch)
        {
            FMPRINT( KYEL , Micro(tod)  ,  " - OutOrder:'" , msg.type , "' clordid:'" , msg.clordid , "' ordid: '" , msg.ordid , "', status='" );
            switch(msg.status)
            {
                case eOutStatus::User: FMPRINT( " 'Cancelled by User' ");
                break;
                case eOutStatus::Ecn: FMPRINT( " 'Unsolicited ECN Cancel' ");
                break;
                default:
                FMPRINT( "'Unknown' ");
                break;
            }        

            FMPRINT( ", timestamp:" , msg.timestamp , ", strmid:'" , msg.streamid , "'" , KNRM , "\n");
        }
        
        if (_store != 0)
        {
            _store->cancel_order(msg.clordid);
            
            ::fastapp::ouch::Slot *slot = _store->find_slot_by_clordid(msg.clordid);
            if (slot != 0 && _store->time2print(tod, *slot) )
            {
                _store->print(tod, *slot);
            }
            _store->print_aggr_ext(tod);
        }        
        
    };
    
    ouchd._funcTrade    = [&] (const ::fastapp::ouch::Trade & msg, uint64_t seq)
    {
        using namespace fastapp;
//        FMPRINT( ::fastapp::sockets::getTimeOfDay() 
        uint64_t tod = ::fastapp::utils::getTimeOfDay() ;
        
//        bool trace = IsTraceOn ? IsTraceOn() : false;
        bool ouch  = IsOuchOn  ? IsOuchOn()  : false;        
        if (ouch)
        {
            FMPRINT( KCYN ,Micro(tod)," - Trade:'" , msg.type , "' clordid:'" , msg.clordid, "', sym:'" , std::string( sym_cstr(msg.ccypair), 7) , "', fillqty:'" , \
            Qty(msg.fillqty),  "', fillrate: '" , Rate(msg.fillrate), "', side: '" );

            switch(msg.side)
            {
                case eSide::Sell:
                    FMPRINT( " 'Sell ");
                    break;
                case eSide::Buy:
                    FMPRINT( " 'Buy ");
                    break;
                default: 
                    FMPRINT( " 'Unknown");
                    break;
            }
            FMPRINT( "', execid: '" , std::string(msg.execid, execid_size), "', leaves: '" , Qty(msg.leaves), "', acct.: '" , msg.acct, "', lqdtyInd.: '" ) ;
            switch(msg.liquidInd)
            {
                case eLiquidInd::AddedvsOrder:
                    FMPRINT( "'AddedVsOrd'");
                    break;
                case eLiquidInd::AddedvsQuote:
                    FMPRINT( "'AddedVsQuote'");
                    break;
                case eLiquidInd::RemovedvsOrder:
                    FMPRINT( "'RemovedVsOrder'");
                    break;
                case eLiquidInd::RemovedVsQuote:
                    FMPRINT( "'RemovedVsQuote'");
                    break;
                default:;       
                    break;
            }

            FMPRINT( "', contracliId:" , msg.contracliId, "', commission:"  , msg.commission, \
             "', transacttime:", msg.transacttime, "', settldate:"   , msg.settldate, \
             "', tradedate:"   , msg.tradedate, \
             "', contrabroker:" ,msg.contrabroker, \
             "', timestamp:" , msg.timestamp , ", strmid:'" , msg.streamid , "'" , KNRM , "\n");  
        }
        
        if (_store != 0)
        {
            _store->process_trade(msg, _trade_con);
            ::fastapp::ouch::Slot *slot = _store->find_slot_by_clordid(msg.clordid);
            if (slot != 0 && _store->time2print(tod, *slot) )
            {
                _store->print(tod, *slot);
            }
            _store->print_aggr_ext(tod);

        }
        else
        {
            FMPRINT( "store=0 sym=" , std::string( sym_cstr(msg.ccypair), 7) , "\n");
        }
    };
    
    ouchd._funcReject      = [&] (const ::fastapp::ouch::Reject & msg, uint64_t seq)
    {
        uint64_t tod = ::fastapp::utils::getTimeOfDay() ;

//        bool trace = IsTraceOn ? IsTraceOn() : false;
        bool ouch  = IsOuchOn  ? IsOuchOn()  : false;        
        if (ouch)
        {
            FMPRINT( KRED , Micro(tod) , " - Reject:" \
            , msg.type , "' , refMessageType=" \
            , msg.reftype , "', rejectCode=" \
            , msg.rejectcode  , "', rejectMessage='" \
            , std::string(msg.rejectmessage, ::fastapp::ouch::rejectmsg_len) \
            , "', rejectcode='" , msg.rejectcode,  "', timestamp:" , msg.timestamp , "', strmid:'" , msg.streamid , "'", KNRM , "\n"); 
        }
        
        if (_store != 0)
        {
            _store->reject_order(-1); // unknown order id

            _store->print_aggr_ext(tod);
        }           
    };
}

} // callbacks
}
} //test
