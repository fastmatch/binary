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

#ifndef ITCH_H
#define	ITCH_H

namespace fastapp
{
namespace itch
{

namespace
{
    constexpr int sym_size = 12;
}

#pragma pack(push, 1)

struct Header    
{
    int64_t timestamp;
    int8_t  streamid;
    int8_t  msgBlockCount;
};

struct BookUpdate
{
    static   constexpr char tag='B';
    char     type;
    int16_t  instrumentID;
    int8_t   updateMsgCount;
    int32_t  sequenceNumber;
};

enum class eSide : char
{
    Bid  ='1',
    Offer='2'
};

enum class eType : char
{
    Order='1',
    Queue='2'
};

enum class eUpdateType : char
{
    Book     = '1',
    Trade    = '2', 
    Midpoint = '3', 
    OnClose  = '4'
};



enum class eActionType : char
{
    Subscribe ='1', 
    Unsub     ='2'
};

enum class eSubType : char
{
    Aggregated = '1', 
    NonAggregated = '2'
};

enum class eStatus  : char 
{
    accepted = '1',
    rejected = '2'
};

struct PriceAdd
{
    static   constexpr char tag='P';
    
    char     type;
    int32_t  priceUpdateID;
    int64_t  qty;
    int64_t  minQty;
    int32_t  rate;
    char     side;
    int16_t  maxDelay;
};

struct PriceCancel
{
    static constexpr char tag='C';

    char     type;
    int32_t  priceUpdateID;
};

struct TradeUpdate
{
    static constexpr char tag='T';

    char     type;
    int16_t  instrumentID;
    int32_t  rate;
    int64_t  qty;
    int8_t   condition;
    int64_t  transTime;
};

struct Midpoint
{
    static constexpr char tag='M';

    char    type;
    int16_t instID;
    int32_t rate;
    
};

struct OnClose
{
    static constexpr char tag='O';

    char    type;
    int16_t instID;
    int32_t rate;
    
};

struct SubscriptionRequest
{
    static constexpr char tag='S';

    char    type;
    char    instSym[12];
    char    updateType; //'1' - book, '2'=Trade, '3' - Midpoint , '4' -MOC
    int32_t reqID;
    char    sessionID[10];
    char    actionType; //subscribe='1', unsub='2'
    char    subType; // '1' -aggregated, '2' - non-aggregated
    char    depth; //  0,1,...9,- 127
};


struct SubscriptionResponse
{
    static   constexpr char tag='R';
    char     type;
    char     instSym[12];
    int16_t  instID;
    int32_t  reqID;
    char     sessionID[10];
    char     status; // '1' -- accepted, '2'-rejected
    char     errorCode;
};

struct InstrumentListRequest
{
    static   constexpr char tag = 'N';
    char     type;
    char     instSym[12];
    char     sessionID[10];
};

struct InstrumentInfoMessage
{
    static   constexpr char tag='F';

    char     type;
    char     instSym[12];
    int16_t  instID;
    char     instType;
    int32_t  setlmentDate;
};

namespace
{
constexpr int rejectmsg_len = 20;
}
struct Reject
{
    static   constexpr char tag='J';

    char     type;
    char     messageType;
    int16_t  rejectCode;
    char     rejectMessage[rejectmsg_len];
};

enum class eErrorCode : char
{
    NoError                  = '0',
    InvalidMessageType       = '1',
    InvalidMessageLength     = '2',
    InvalidInstrument        = '3',
    OtherError               = 'Z',
    NotAuthorized            = 'A',
    UnsupportedInstrument    = 'S',
    UnsupportedVersion       = 'V',
    DuplicateRequestID       = 'D',
    SubscriptinAlreadyActive = 'P',
    ExchangeNotOpen          = 'E',
    InvalidActionType        = 'F',
    InvalidSubscriptionType  = 'G',
    InvalidDepth             = 'H',
    UnauthorizedSession      = 'J'
};

#pragma pack(pop)

}
}

#endif	/* ITCH_H */

