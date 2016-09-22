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

#ifndef HELPRES_H
#define	HELPRES_H

#include <string>
#include <src/itch/itch.h>
#include <src/ouch/ouch.h>
#include <src/soupbin/soupbin.h>
#include <src/connection.h>

namespace fastapp
{
    
typedef ::fastapp::soupbin::UnsequencedData<sizeof(::fastapp::itch::Header) + sizeof(::fastapp::itch::SubscriptionRequest) > ReqData;
typedef ::fastapp::soupbin::UnsequencedData<sizeof(::fastapp::itch::Header) + sizeof(::fastapp::itch::InstrumentListRequest) > InstReqData;
typedef ::fastapp::soupbin::UdpSoupBin<ReqData > UdpReqData;
typedef ::fastapp::soupbin::UdpSoupBin<InstReqData > UdpInstReqData;

struct TcpDefs
{
    typedef ::fastapp::ReqData            ReqData;
    typedef ::fastapp::InstReqData        InstReqData;
    typedef TcpLogin           Login;
    typedef TcpClientHeartbeat ClientHeartbeat;
    typedef TcpLogout          Logout;
};
// ReqData, InstReqData, TcpLogin, TcpClientHeartbeat, TcpLogout
struct UdpDefs
{
    typedef UdpReqData         ReqData;
    typedef UdpInstReqData     InstReqData;
    typedef UdpLogin           Login;
    typedef UdpClientHeartbeat ClientHeartbeat;
    typedef UdpLogout          Logout;
};
// UdpReqData, UdpInstReqData, UdpLogin, UdpClientHeartbeat, UdpLogout

#define fastapp_max(left,right) (left > right ? left : right)

typedef ::fastapp::soupbin::UnsequencedData<sizeof(::fastapp::ouch::NewOrder) >     NewReqData;
typedef ::fastapp::soupbin::UnsequencedData<sizeof(::fastapp::ouch::ReplaceOrder) > RplReqData;
typedef ::fastapp::soupbin::UnsequencedData<sizeof(::fastapp::ouch::CancelOrder) >  CxlReqData;

typedef ::fastapp::soupbin::UnsequencedData< fastapp_max( sizeof(::fastapp::ouch::NewOrder), sizeof(::fastapp::ouch::ReplaceOrder) ) >  NRReqData;

void populateLogout(TcpLogout & logout);
void populateLogout(UdpLogout & req);
void stampLogout(UdpLogout & req, int32_t seq);
void stampLogout(TcpLogout & req, int32_t seq);

void stampLogin(UdpLogin & req, int32_t seq);
void stampLogin(TcpLogin & req, int32_t seq);
inline void stampHeartbeat(UdpClientHeartbeat & req, int32_t seq)
{
    req.seq = seq;
}
inline void stampHeartbeat(TcpClientHeartbeat & req, int32_t seq){}

void populateLogin(::fastapp::soupbin::Login & login, const std::string & sessionID, const std::string & seqnum, const std::string & username, const std::string & password);
void populateHbc(::fastapp::soupbin::ClientHeartbeat & hbc);

void populateLogin(UdpLogin & login, const std::string & sessionID, const std::string & seqnum, const std::string & username, const std::string & password);
void populateHbc(UdpClientHeartbeat & hbc);

::fastapp::itch::InstrumentListRequest & getInstrumentListRequest(InstReqData & seq_data);
void populateListRequest(::fastapp::itch::InstrumentListRequest & req, const std::string & session, const std::string & symbol);

::fastapp::itch::SubscriptionRequest & getSubscriptionRequest(ReqData & seq_data);
void populateSubscriptionRequest(::fastapp::itch::SubscriptionRequest & req, const std::string & session, const ::fastapp::Connection::Sub & sub, const std::string & symbol);

::fastapp::ouch::NewOrder & getNewOrderRequest(NewReqData & seq_data);

void stampListRequest(InstReqData & seq_data, int32_t seq);
void stampSubscriptionRequest(ReqData & seq_data, int32_t seq);

//void populateLogin(::fastapp::soupbin::Login & login, const std::string & sessionID, const std::string & seqnum, const std::string & username, const std::string & password);
//void populateHbc(::fastapp::soupbin::ClientHeartbeat & hbc);

::fastapp::itch::InstrumentListRequest & getInstrumentListRequest(UdpInstReqData & seq_data);
::fastapp::itch::SubscriptionRequest & getSubscriptionRequest(UdpReqData & seq_data);

void stampListRequest(UdpInstReqData & seq_data, int32_t seq);
void stampSubscriptionRequest(UdpReqData & seq_data, int32_t seq);


::fastapp::ouch::NewOrder & getNewOrderRequest(NewReqData & seq_data);
::fastapp::ouch::NewOrder & castNewOrderRequest(NewReqData & seq_data);
::fastapp::ouch::NewOrder & castOrderRequest(NewReqData  & seq_data);

::fastapp::ouch::ReplaceOrder & castOrderRequest(RplReqData & seq_data);
::fastapp::ouch::ReplaceOrder & castRplOrderRequest(RplReqData & seq_data);
::fastapp::ouch::ReplaceOrder & getReplaceRequest(RplReqData & seq_data);

::fastapp::ouch::CancelOrder & getCancelOrderRequest(CxlReqData & seq_data);

    // helpers
int64_t timeNow();
int64_t timeOfDay();
int64_t ticksPerSecond();

}

#endif	/* HELPRES_H */
