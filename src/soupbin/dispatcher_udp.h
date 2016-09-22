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


#ifndef SOUPBIN_DISPATCHER_UDP_H__
#define SOUPBIN_DISPATCHER_UDP_H__

#include <src/soupbin/soupbin.h>
#include <src/soupbin/dispatcher.h>
#include <functional>
#include <src/print.h>

namespace fastapp
{
    
namespace soupbin_udp
{ 
    

class Dispatcher : public fastapp::soupbin::Dispatcher
{
public:
    
    void process(void * ptr, int32_t size);
    
    Dispatcher () : fastapp::soupbin::Dispatcher()
    {
    }
    Dispatcher (const Dispatcher & d) = default;
    Dispatcher (Dispatcher && d) = default;
    Dispatcher & operator = (Dispatcher && d) = default;
    Dispatcher & operator = (Dispatcher & d) = default;
    ~Dispatcher () = default;
    
};
        

inline void Dispatcher::process(void * vptr, int32_t size)
{
    if (size == 0) 
    {
        std::cerr << "skipping; size=0! "  << std::endl;
        return;
    }
    using namespace fastapp::soupbin;
    UdpHeader *ptr = reinterpret_cast<UdpHeader *>(vptr);
    
    switch(ptr->hdr.type)
    {
    case ServerHeartbeat::tag:
        if (_funcSvrHB) _funcSvrHB(*reinterpret_cast<const ServerHeartbeat *>(&ptr->hdr), size - sizeof(ptr->seq) , ptr->seq);
        break;
        
    case ClientHeartbeat::tag:
         if (_funcCliHB) _funcCliHB(*reinterpret_cast<const ClientHeartbeat *>(&ptr->hdr), size - sizeof(ptr->seq), ptr->seq );       
        break;

        
    case DebugMessageHeader::tag:
        if (_funcDebug) _funcDebug(*reinterpret_cast<const DebugMessageHeader *>(&ptr->hdr), size - sizeof(ptr->seq), ptr->seq );
        break;

    case Login::tag:
        if (_funcLogin) _funcLogin(*reinterpret_cast<const Login *>(&ptr->hdr), size - sizeof(ptr->seq), ptr->seq );
        break;
    case Logout::tag:
        if (_funcLogout) _funcLogout(*reinterpret_cast<const Logout *>(&ptr->hdr), size - sizeof(ptr->seq), ptr->seq );  
        break;
        
    case LoginAccept::tag:
    {
        const LoginAccept * accept = reinterpret_cast<const LoginAccept *>(&ptr->hdr);
        
        char buf[fastapp::soupbin::seqnum_size + 1];
        buf[fastapp::soupbin::seqnum_size] = (char)0;
        ::memcpy(buf, accept->seqnum, fastapp::soupbin::seqnum_size);
        
        int seq = atoi(buf);
        
        _sequence = seq - 1;
        if (_sequence == 0) _sequence = ptr->seq;
        if (_funcAccept) _funcAccept(*reinterpret_cast<const LoginAccept *>(&ptr->hdr), size - sizeof(ptr->seq), ptr->seq);
    }    break;
    
    case LoginReject::tag:
        if (_funcReject) _funcReject(*reinterpret_cast<const LoginReject *>(&ptr->hdr), size - sizeof(ptr->seq), ptr->seq );
        break;
    case EndOfSession::tag:
        if (_funcEOFSession) _funcEOFSession(*reinterpret_cast<const EndOfSession *>(&ptr->hdr), size - sizeof(ptr->seq), ptr->seq );
        break;

    case SequencedDataHeader::tag:
    {
        
        if (_funcSeq) _funcSeq(*reinterpret_cast<const SequencedDataHeader *>(&ptr->hdr), size - sizeof(ptr->seq), ptr->seq);  
    } break;

    case UnsequencedDataHeader::tag:
    {    if (_funcUnseq) _funcUnseq(*reinterpret_cast<const UnsequencedDataHeader *>(&ptr->hdr), size - sizeof(ptr->seq), ptr->seq );  
    } break;
        
    default:
    {
         std::cerr << "Soupbin:Unknown type: "  << ptr->hdr.type << ";";
         ::fastapp::utils::hex(std::cerr, reinterpret_cast<char *>(ptr), size);
         std::cerr << std::endl;
    } break;
    
    }
    
    
    if ((size_t)ptr->seq != _sequence)
    {
        std::cerr << KRED;
        int64_t tod = ::fastapp::utils::getTimeOfDay();  
        ::fastapp::utils::formatTime(tod, std::cerr);                  
        std::cerr << " Detected udp sequence gap: expected:'" << _sequence << "' received:'" << ptr->seq << "' msgType:" << ptr->hdr.type << "; ";
        ::fastapp::utils::hex(std::cerr, reinterpret_cast<char *>(ptr), size);
        std::cerr << KNRM << std::endl;
        
        if (ptr->seq != 0)
        {    _sequence = ptr->seq;    }
    }
    
    _sequence ++;
}

} // soupbin
} // fastapp

#endif

