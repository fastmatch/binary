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


#ifndef SOUPBIN_MESSAGES_H
#define	SOUPBIN_MESSAGES_H

#include <cstdint>

namespace fastapp
{
    
namespace soupbin
{

namespace
{
    constexpr  int session_size  = 10;  // body[0] == type
    constexpr  int seqnum_size   = 20;  // body[0] == type
}

#pragma pack(push, 1)

struct Header
{
    int16_t size;
    char    type;
};

struct UdpHeader
{
    int32_t seq;
    Header  hdr;
};

struct SoupBinMessage
{
    int16_t size;
    char    body[1]; //body[0] == type]
};

struct UdpSoupBinMessage
{
    int32_t seq;
    SoupBinMessage msg;
};

template <typename SOUPMSG>
struct UdpSoupBin
{
    int32_t seq;
    SOUPMSG msg;
};

struct DebugMessageHeader
{
    static constexpr char tag = '+'; //body[0] == type]

    int16_t size;
    char    type;
};

struct DebugMessageHeaderRef
{
    static constexpr char tag = '+'; //body[0] == type]

    int16_t size;
    char    type;
    char    text[1];
};

template < int len>
struct DebugMessage
{
    static constexpr int16_t csize = len + 1;
    static constexpr char    tag = '+'; //body[0] == type]
    
    int16_t size;
    char    type;
    char    text[len];
};

struct LoginAccept
{
    static constexpr char tag = 'A'; //body[0] == type]

    int16_t size;
    char    type;
    char    session [10];  // alfa-numeric
    char    seqnum  [20];  // numeric left padded with spaces
};

struct LoginReject
{
    enum class eRejectReason : char
    {
        A='A', // Not Authorized. There was an invalid username and password combination in the Login Request Message. "S"

        S='S'  // Session not available. The Requested Session in the Login Request Packet was either invalid or not available.
    };
    
    static constexpr char tag = 'J'; //body[0] == type]
    
    int16_t        size;
    char           type;
    char           rejectReason;

};

struct SequencedDataHeader
{
    static constexpr  char tag = 'S'; //body[0] == type]   

    int16_t       size;
    char          type;
};

struct SequencedDataRef
{
    static constexpr  char tag = 'S'; //body[0] == type]

    int16_t    size;
    char       type;
    char       data[1];
};

template < int len>
struct SequencedData
{
    static constexpr  char tag = 'S'; //body[0] == type]

    int16_t    size;
    char       type;
    char       data[len];
};

struct ServerHeartbeat
{
    static constexpr  int16_t    csize = 1;
    static constexpr  char       tag = 'H'; //body[0] == type]
    int16_t    size;
    char       type; 
    
};

struct EndOfSession
{
    static constexpr  int16_t    csize = 1;
    static constexpr  char       tag = 'Z'; //body[0] == type]
    
    int16_t    size = 1;
    char       type = 'Z'; //body[0] == type]
    
};

struct Login
{
    static constexpr  int username_size = 6;   // body[0] == type
    static constexpr  int password_size = 10;  // body[0] == type
    static constexpr  int session_size  = fastapp::soupbin::session_size;  // body[0] == type
    static constexpr  int seqnum_size   = fastapp::soupbin::seqnum_size;   // body[0] == type

    //static constexpr  int16_t  csize     = sizeof(Login) - sizeof(int16_t);  // --
    static constexpr  char     tag       = 'L'; // body[0] == type
    static constexpr  char     cversion  = '1'; // body[0] == type
    
    int16_t    size;
    char       type; 
    int16_t    version;
    char       username[username_size];
    char       password[password_size];
    char       session [session_size ];
    char       seqnum  [seqnum_size  ];
};

struct UnsequencedDataHeader
{
    static constexpr  char tag = 'U'; //body[0] == type]
    
    int16_t    size;
    char       type;
    char       data[1];
};

struct UnsequencedDataRef
{
    static constexpr  char tag = 'U'; //body[0] == type]
    
    int16_t    size;
    char       type;
    char       data[1];
};

template <int len>
struct UnsequencedData
{
    static constexpr  char tag = 'U'; //body[0] == type]
    
    int16_t    size;
    char       type;
    char       data[len];
};


struct ClientHeartbeat
{
    static constexpr  int16_t    csize = 1;
    static constexpr  char       tag = 'R'; //body[0] == type]

    int16_t    size;
    char       type;
};


struct Logout
{
    static constexpr  int16_t  csize = 1;
    static constexpr  char     tag = 'O'; //body[0] == type]
    
    int16_t  size;
    char     type;
};


#pragma pack(pop)
} //soupbin

typedef ::fastapp::soupbin::Login            TcpLogin;
typedef ::fastapp::soupbin::ClientHeartbeat  TcpClientHeartbeat;
typedef ::fastapp::soupbin::Logout           TcpLogout;


typedef ::fastapp::soupbin::UdpSoupBin<::fastapp::soupbin::Login > UdpLogin;
typedef ::fastapp::soupbin::UdpSoupBin<::fastapp::soupbin::ClientHeartbeat > UdpClientHeartbeat;
typedef ::fastapp::soupbin::UdpSoupBin<::fastapp::soupbin::Logout > UdpLogout;
} //fastapp


#endif	/* MESSAGES_H */

