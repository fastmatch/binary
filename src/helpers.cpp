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

#include <string>
#include <iostream>

#include <src/helpers.h>
#include <src/socketutils.h>
#include <src/print.h>

namespace fastapp
{
    
    // helpers

int64_t timeNow()
{
    return ::fastapp::utils::getTimeNow(); 
}

int64_t timeOfDay()
{
    return ::fastapp::utils::getTimeOfDay();
}

int64_t ticksPerSecond()
{
    return ::fastapp::utils::ticksPerSecond();
}

void populateLogin(::fastapp::soupbin::Login & login, const std::string & sessionID, const std::string & seqnum, const std::string & username, const std::string & password)
{
    FMPRINT( "populateLogin", "\n");
    memset(&login, 0, sizeof(::fastapp::soupbin::Login));
    login.size = sizeof(login) - sizeof(int16_t);
    login.type = ::fastapp::soupbin::Login::tag;
    login.version = 1;
    
    ::memset(login.username, ' ', login.username_size);
    if (!username.empty())
    {
        ::strncpy(login.username, username.c_str(), login.username_size);
        if (username.size() < login.username_size)
        {
            ::memset(&login.username[username.size()], ' ', login.username_size - username.size());
        }
    }
    
    ::memset(login.password, ' ', login.password_size);
    if (!password.empty())
    {
        ::strncpy(login.password, password.c_str(), login.password_size);
        if (password.size() < login.password_size)
        {
            ::memset(&login.password[password.size()], ' ', login.password_size - password.size());
        }
    }
    
    ::strncpy(login.session , sessionID.c_str(), login.session_size);
    ::strncpy(login.seqnum  , seqnum.c_str()   , login.seqnum_size);    
    
    FMPRINT("user:",std::string(login.username, login.username_size) \
       ,"; pass:" , std::string(login.password, login.password_size) \
       ,"; session:", std::string(login.session, login.session) \
       ,"; seqnum:" , std::string(login.seqnum, login.seqnum_size) , ";" , "\n");
}

void populateLogin(UdpLogin & login, const std::string & sessionID, const std::string & seqnum, const std::string & username, const std::string & password)
{
    login.seq = 0;
    populateLogin(login.msg, sessionID, seqnum, username, password);
}


void populateHbc(::fastapp::soupbin::ClientHeartbeat & hbc)
{
    hbc.size = hbc.csize;
    hbc.type = hbc.tag;        
}

void populateHbc(UdpClientHeartbeat & hbc)
{
    populateHbc(hbc.msg);
}

::fastapp::itch::InstrumentListRequest & getInstrumentListRequest(InstReqData & seq_data)
{
    seq_data.type = ReqData::tag;
    seq_data.size = sizeof(seq_data ) - sizeof(seq_data.size);
            
    ::fastapp::itch::Header & req_hdr = *reinterpret_cast<::fastapp::itch::Header *>(seq_data.data);
    req_hdr.timestamp = ::fastapp::Connection::timeOfDay();
    req_hdr.streamid = 0;
    req_hdr.msgBlockCount = 1;
            
    return (*reinterpret_cast<::fastapp::itch::InstrumentListRequest *>(seq_data.data + sizeof(::fastapp::itch::Header) ) );   
}

void stampLogin(UdpLogin & req, int32_t seq)
{
    req.seq= seq;
}

void stampLogin(TcpLogin & req, int32_t seq)
{
}

void stampListRequest(InstReqData & seq_data, int32_t )
{
    ::fastapp::itch::Header & req_hdr = *reinterpret_cast<::fastapp::itch::Header *>(seq_data.data);
    req_hdr.timestamp = fastapp::Connection::timeOfDay();
}

void stampSubscriptionRequest(ReqData & seq_data, int32_t )
{
    ::fastapp::itch::Header & req_hdr = *reinterpret_cast<::fastapp::itch::Header *>(seq_data.data);
    req_hdr.timestamp = ::fastapp::Connection::timeOfDay();
}

void stampListRequest(UdpInstReqData & seq_data, int32_t seq)
{
    seq_data.seq = seq;
    stampListRequest(seq_data.msg, seq);
}

void stampSubscriptionRequest(UdpReqData & seq_data, int32_t seq)
{
    seq_data.seq = seq;
    stampSubscriptionRequest(seq_data.msg, seq);
}

void populateLogout(TcpLogout & logout)
{
    logout.type=logout.tag;
    logout.size=sizeof(TcpLogout) - sizeof(int16_t);    
}

void populateLogout(UdpLogout & req)
{
    populateLogout(req.msg);
}

void stampLogout(UdpLogout & req, int32_t seq)
{
    req.seq = seq;
}

void stampLogout(TcpLogout & req, int32_t seq)
{
}


void populateListRequest(::fastapp::itch::InstrumentListRequest & req, const std::string & session, const std::string & symbol)
{
    FMPRINT("populateInstrumentListRequest; ");
     ::memset(&req, 0, sizeof(::fastapp::itch::InstrumentListRequest));
    req.type = req.tag;
    ::strncpy(req.instSym, symbol.c_str(), ::fastapp::itch::sym_size);
    FMPRINT("type=", req.type , ", sym=", symbol);
     ::strncpy(req.sessionID , session.c_str(), ::fastapp::soupbin::session_size);
     
    FMPRINT(", session=" , session.c_str() , "\n");
}

::fastapp::ouch::NewOrder & castOrderRequest(NewReqData & seq_data)
{
    ::fastapp::ouch::Header & req_hdr = *reinterpret_cast<::fastapp::ouch::Header *>(seq_data.data);
    return (::fastapp::ouch::NewOrder &)req_hdr;
}

::fastapp::ouch::NewOrder & castNewOrderRequest(NewReqData & seq_data)
{
    ::fastapp::ouch::Header & req_hdr = *reinterpret_cast<::fastapp::ouch::Header *>(seq_data.data);
    return (::fastapp::ouch::NewOrder &)req_hdr;
}

::fastapp::ouch::ReplaceOrder & castOrderRequest(RplReqData & seq_data)
{
    ::fastapp::ouch::Header & req_hdr = *reinterpret_cast<::fastapp::ouch::Header *>(seq_data.data);
    return (::fastapp::ouch::ReplaceOrder &)req_hdr;
}

::fastapp::ouch::ReplaceOrder & castRplOrderRequest(RplReqData & seq_data)
{
    ::fastapp::ouch::Header & req_hdr = *reinterpret_cast<::fastapp::ouch::Header *>(seq_data.data);
    return (::fastapp::ouch::ReplaceOrder &)req_hdr;
}

::fastapp::ouch::NewOrder & getNewOrderRequest(NewReqData & seq_data)
{
    seq_data.type = NewReqData::tag;
    seq_data.size = sizeof(seq_data ) - sizeof(seq_data.size);
    ::fastapp::ouch::Header & req_hdr = *reinterpret_cast<::fastapp::ouch::Header *>(seq_data.data);
    req_hdr.timestamp = ::fastapp::Connection::timeOfDay();
    req_hdr.streamid = 0;
    req_hdr.type = ::fastapp::ouch::NewOrder::tag;
    return (::fastapp::ouch::NewOrder &)req_hdr;
}

::fastapp::ouch::CancelOrder & getCancelOrderRequest(CxlReqData & seq_data)
{
    seq_data.type = CxlReqData::tag;
    seq_data.size = sizeof(seq_data ) - sizeof(seq_data.size);
    ::fastapp::ouch::Header & req_hdr = *reinterpret_cast<::fastapp::ouch::Header *>(seq_data.data);
    req_hdr.timestamp = ::fastapp::Connection::timeOfDay();
    req_hdr.streamid = 0;
    req_hdr.type = ::fastapp::ouch::CancelOrder::tag;
    return (::fastapp::ouch::CancelOrder &)req_hdr;
}

::fastapp::ouch::ReplaceOrder & getReplaceRequest(RplReqData & seq_data)
{
    seq_data.type = RplReqData::tag;
    seq_data.size = sizeof(seq_data ) - sizeof(seq_data.size);
    ::fastapp::ouch::Header & req_hdr = *reinterpret_cast<::fastapp::ouch::Header *>(seq_data.data);
    req_hdr.timestamp = ::fastapp::Connection::timeOfDay();
    req_hdr.streamid = 0;
    req_hdr.type = ::fastapp::ouch::ReplaceOrder::tag;
    return (::fastapp::ouch::ReplaceOrder &)req_hdr;
}


::fastapp::itch::SubscriptionRequest & getSubscriptionRequest(ReqData & seq_data)
{
    seq_data.type = ReqData::tag;
    seq_data.size = sizeof(seq_data ) - sizeof(seq_data.size);
            
    ::fastapp::itch::Header & req_hdr = *reinterpret_cast<::fastapp::itch::Header *>(seq_data.data);
    req_hdr.timestamp = ::fastapp::Connection::timeOfDay();
    req_hdr.streamid = 0;
    req_hdr.msgBlockCount = 1;
            
    return (*reinterpret_cast<::fastapp::itch::SubscriptionRequest *>(seq_data.data + sizeof(::fastapp::itch::Header) ) );   
}

void populateSubscriptionRequest(::fastapp::itch::SubscriptionRequest & req, const std::string & session, const ::fastapp::Connection::Sub & sub, const std::string & symbol)
{
    FMPRINT("populateSubscriptionRequest; " );
     ::memset(&req, 0, sizeof(::fastapp::itch::SubscriptionRequest));
    req.type = ::fastapp::itch::SubscriptionRequest::tag;
    ::strncpy(req.instSym, symbol.c_str(), ::fastapp::itch::sym_size);
    
    static int reqid = 1;
    
    req.reqID      = reqid;  //1;
    FMPRINT("requestId=" , req.reqID);
    reqid ++;
    
    req.updateType = ::fastapp::utils::to_integral(sub.updateType); //'1';    
    ::strncpy(req.sessionID , session.c_str(), ::fastapp::soupbin::session_size);

    FMPRINT( "; session=" , std::string(req.sessionID, ::fastapp::soupbin::session_size) );

    req.actionType = ::fastapp::utils::to_integral(::fastapp::itch::eActionType::Subscribe); // '1'; //subscribe='1', unsub='2'
    req.subType    = ::fastapp::utils::to_integral(sub.subType); //'1'; // '1' -aggregated, '2' - non-aggregated
    FMPRINT("; sub.depth=" ,  (int)sub.depth);
    req.depth      = sub.depth; //  '0','1',...'9'    
    FMPRINT("\n");
}

::fastapp::itch::InstrumentListRequest & getInstrumentListRequest(UdpInstReqData & seq_data)
{
    seq_data.seq = 0;
    return getInstrumentListRequest(seq_data.msg);
}

::fastapp::itch::SubscriptionRequest & getSubscriptionRequest(UdpReqData & seq_data)
{
    seq_data.seq = 0;
    return getSubscriptionRequest(seq_data.msg);
}




}
