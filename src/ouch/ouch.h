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


/* 
 * File:   itch.h
 * Author: casinovs
 *
 * Created on July 31, 2015, 3:23 PM
 */

#ifndef FAST_OUCH_H__
#define	FAST_OUCH_H__

#include <cstdint>
#include <type_traits>

namespace fastapp
{
namespace ouch
{

namespace
{
    static const int sym_size = 8;
}

#pragma pack(push, 1)

struct Header    
{
    char     type;
    int32_t  timestamp;
    int8_t   streamid;
};

enum class eOrderType : char
{
     Market = '1' 
   , Limit  = '2'         
};

enum class eSide : char
{
     Buy    = '1' 
   , Sell   = '2'         
};

enum class eTimeInForce : char
{
     Day    = '1'
   , Ioc    = '2'
   , Fok    = '3'
};



enum class eErrorCode : char
{
    NoError            = '0',
    DuplicateOrderId   = '1',
    ExchangeNotOpen    = '2',
    InvalidQty         = '3',
    InvalidRate        = '4',
    UnknownStreamId    = '5',
    InvalidAccount     = '6',
    UnknownSymbol      = '7',
    InvalidSide        = '8',
    InvalidMinQty      = '9',
    UnsupportedTIF     = 'A',
    InvalidMaxShow     = 'B',
    InvalidType        = 'E',
    OrderNotFound      = 'F',
    TooLateToCancel    = 'G',
    SymbolDisabled     = 'H',
    TooManyOpenOrders  = 'I',
    TradingSuspended   = 'J',
    OtherError         = 'Z'
};

enum class eRejectReasonCode
{
    NotAuthorized      = 'A',
    SessionNotAvailable= 'S',
    InvalidVersion     = 'V'
};

typedef int32_t         clord_id_t;
typedef char            ccypair_t[sym_size]  ;
typedef int64_t         ccypair_int_t;
typedef int64_t         orderid_t;

struct NewOrderBase : public Header
{
    clord_id_t    clordid;
    ccypair_int_t ccypair;
    eOrderType    orderType;
    eSide         side;
    int64_t       qty;
    int64_t       minqty;
    int32_t       rate;
    eTimeInForce  tif;
};

struct NewOrder    : public NewOrderBase
{
    static        constexpr char tag='D';
};

template <int T>
struct S;

////S<sizeof(NewOrder) > a1;

static_assert( sizeof(NewOrder) == 41, "s(NewOrder) != 41" );

struct NewOrderExt : public NewOrderBase
{
    static        constexpr char tag='E';
    int64_t       maxShow;
    int64_t       account;
};

////S<sizeof(NewOrderExt) > a2;

static_assert( sizeof(NewOrderExt) == 57 , "s(NewExt) != 57");

enum class eAckStatus : char 
{
    Accept = '1'
  , Reject = '2'
};

struct NewOrderAck : public Header
{
    static        constexpr char tag='A';
    
    clord_id_t      clordid;
    ccypair_int_t   ccypair;
    orderid_t       ordid;
    eAckStatus      status;
    eErrorCode      err;
};

static_assert(sizeof(NewOrderAck) == 28, "size(Ack) != 28");

struct CancelOrder : public Header
{
    static        constexpr char tag='F';
    
    clord_id_t     clordid;
    clord_id_t     origclordid;
    ccypair_int_t  ccypair;
};
static_assert(sizeof(CancelOrder) == 22, "size(CancelOrder) != 22");

struct CancelOrderReject : public Header
{
    static        constexpr char tag='R';
    
    clord_id_t     clordid;
    clord_id_t     origclordid;
    eErrorCode     err;
};
static_assert(sizeof(CancelOrderReject) == 15, "sizeof(CancelOrderReject) != 15");

struct ReplaceOrder : public Header
{
    static        constexpr char tag='G';
    
    clord_id_t     clordid;
    clord_id_t     origclordid;
    ccypair_int_t  ccypair;
    int64_t        qty;
    int32_t        rate;
};

static_assert(sizeof(ReplaceOrder) == 34, "sizeof(ReplaceOrder) != 34");

enum class eReplaceStatus : char
{
      Cancelled = '1'   
    , Replaced  = '2'
    , Rejected  = '3'
};

struct ReplaceAck : public Header
{
    static        constexpr char tag='P';
    clord_id_t      clordid;
    clord_id_t      origclordid;
    eReplaceStatus  status;
    eErrorCode      err;
};

//S<sizeof(ReplaceAck)> a;
static_assert(sizeof(ReplaceAck) == 16, "sizeof(ReplaceAck) != 16");

enum class eOutStatus : char
{
      User = '1'   
    , Ecn  = '2'
};

struct OutOrder   : public Header
{
    static        constexpr char tag='C';
    clord_id_t     clordid;
    orderid_t      ordid;
    eOutStatus     status;
};

static_assert(sizeof(OutOrder) == 19, "sizeof(OutOrder) != 19");


namespace
{
    static const int execid_size = 20;
    static const int acct_size   = 8;
}

typedef char execid_t[execid_size];

enum class eLiquidInd : char
{
    AddedvsOrder  ='A',
    AddedvsQuote  ='B',
    RemovedvsOrder='R',
    RemovedVsQuote='S'
};


struct Trade   : public Header
{
    static         constexpr char tag='T';
    clord_id_t     clordid;
    ccypair_int_t  ccypair;
    int64_t        fillqty;
    int32_t        fillrate;
    eSide          side;
    execid_t       execid;
    int64_t        leaves;
    int64_t        acct;
    eLiquidInd     liquidInd;
    int16_t        contracliId;
    int32_t        commission;
    int64_t        transacttime;
    int32_t        settldate;
    int32_t        tradedate;  
    int32_t        contrabroker;
};
// S<sizeof(Trade)> t;
static_assert(sizeof(Trade) == 94, "sizeof(Trade) != 94");

namespace
{
constexpr int rejectmsg_len = 20;
}

struct Reject : public Header
{
    static   constexpr char tag='J';

    char     reftype;
    int16_t  rejectcode;
    char     rejectmessage[rejectmsg_len];
};

//S<sizeof(Reject)> r;

static_assert(sizeof(Reject) == 29, "sizeof(Reject) != 29");



#pragma pack(pop)

}
}

#endif	/* FAST_OUCH_H */

