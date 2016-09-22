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


#ifndef SOUPBIN_DISPATCHER_H__
#define SOUPBIN_DISPATCHER_H__

#include <src/soupbin/soupbin.h>

#include <functional>

#include <src/print.h>

namespace fastapp
{
    
namespace soupbin
{ 
    
typedef std::function< void (const Login & login, int32_t size, int32_t seq) > FuncLogin;
typedef std::function< void (const LoginAccept &, int32_t size, uint64_t seq) > FuncAccept;
typedef std::function< void (const LoginReject &, int32_t size, int32_t seq) > FuncReject;
typedef std::function< void (const Logout      &, int32_t size, int32_t seq) > FuncLogout;


//Header
typedef std::function< void (const SoupBinMessage & , int32_t size, int32_t seq) > FuncSoup;
typedef std::function< void (const DebugMessageHeader & , int32_t size, int32_t seq) > FuncDebug;

//DebugMessageHeader
//DebugMessageHeaderRef

//SequencedDataHeader
typedef std::function< void (const SequencedDataHeader &, int32_t size, uint64_t seq) > FuncSeqData;
//SequencedDataRef
typedef std::function< void (const ServerHeartbeat &, int32_t size, int32_t seq) > FuncSvrHB;
typedef std::function< void (const EndOfSession    &, int32_t size, int32_t seq) > FuncEndoOfSession;

typedef std::function< void (const UnsequencedDataHeader &, int32_t size, int32_t seq) > FuncUnseq;
//UnsequencedDataRef
typedef std::function< void (const ClientHeartbeat &, int32_t size, int32_t seq) > FuncClientHB;


class Dispatcher
{
public:
    typedef ::fastapp::soupbin::FuncLogin         FuncLogin;
    typedef ::fastapp::soupbin::FuncAccept        FuncAccept;
    typedef ::fastapp::soupbin::FuncReject        FuncReject;
    typedef ::fastapp::soupbin::FuncLogout        FuncLogout;

    
//Header
    typedef ::fastapp::soupbin::FuncSoup          FuncSoup;
    typedef ::fastapp::soupbin::FuncDebug         FuncDebug;
    
//DebugMessageHeader
//DebugMessageHeaderRef

//SequencedDataHeader
    typedef ::fastapp::soupbin::FuncSeqData       FuncSeqData;
//SequencedDataRef
    typedef ::fastapp::soupbin::FuncSvrHB         FuncSvrHB;
    typedef ::fastapp::soupbin::FuncEndoOfSession FuncEndoOfSession;

    typedef ::fastapp::soupbin::FuncUnseq         FuncUnseq;
//UnsequencedDataRef
    typedef ::fastapp::soupbin::FuncClientHB      FuncClientHB;
    
public:
    
    void process(void * ptr, int32_t size);
    
    Dispatcher () : _sequence(1)
    {
    }
    Dispatcher (const Dispatcher & d) = default;
    Dispatcher (Dispatcher && d) = default;
    Dispatcher & operator = (Dispatcher && d) = default;
    Dispatcher & operator = (Dispatcher & d) = default;
    ~Dispatcher () = default;
    

public:

    void sub(FuncLogin  f) { _funcLogin  = f; }
    void sub(FuncAccept f) { _funcAccept = f; }
    void sub(FuncReject f) { _funcReject = f; }
    void sub(FuncLogout f) { _funcLogout = f; }
    void sub(FuncEndoOfSession  f) {_funcEOFSession = f; }

    void sub(FuncClientHB f) { _funcCliHB = f; }
    void sub(FuncSvrHB    f) { _funcSvrHB = f; }


    void sub(FuncDebug    f) { _funcDebug = f; }
    void sub(FuncSoup     f) { _funcSoup  = f; }
    void sub(FuncSeqData  f) { _funcSeq   = f; }
    void sub(FuncUnseq    f) { _funcUnseq = f; }


protected:
        
    FuncLogin    _funcLogin;;
    FuncAccept   _funcAccept;
    FuncReject   _funcReject;
    FuncLogout   _funcLogout;
    FuncEndoOfSession _funcEOFSession;
    
    FuncClientHB _funcCliHB;    
    FuncSvrHB    _funcSvrHB;


    FuncDebug    _funcDebug;
    FuncSoup     _funcSoup;
    FuncSeqData  _funcSeq;
    FuncUnseq    _funcUnseq;
    
protected:    
    uint64_t     _sequence;
};
        

inline void Dispatcher::process(void * vptr, int32_t size)
{
    if (size == 0) 
    {
        FMPRINT( "skipping; size=0! " , "\n");
        return;
    }
    const char *ptr = reinterpret_cast<const char *>(vptr);
    
    switch(reinterpret_cast<Header*>(vptr)->type)
    {
    case ServerHeartbeat::tag:
        if (_funcSvrHB) _funcSvrHB(*reinterpret_cast<const ServerHeartbeat *>(ptr), size, 0 );
        break;
        
    case ClientHeartbeat::tag:
         if (_funcCliHB) _funcCliHB(*reinterpret_cast<const ClientHeartbeat *>(ptr), size, 0 );       
        break;

        
    case DebugMessageHeader::tag:
        if (_funcDebug) _funcDebug(*reinterpret_cast<const DebugMessageHeader *>(ptr), size, 0 );
        break;

    case Login::tag:
        if (_funcLogin) _funcLogin(*reinterpret_cast<const Login *>(ptr), size, 0 );
        break;
    case Logout::tag:
        if (_funcLogout) _funcLogout(*reinterpret_cast<const Logout *>(ptr), size, 0 );  
        break;
        
    case LoginAccept::tag:
    {
        const LoginAccept * accept = reinterpret_cast<const LoginAccept *>(ptr);
        
        char buf[fastapp::soupbin::seqnum_size + 1];
        buf[fastapp::soupbin::seqnum_size] = (char)0;
        ::memcpy(buf, accept->seqnum, fastapp::soupbin::seqnum_size);
        
        int seq = atoi(buf);
        
        _sequence = seq;
        
        if (_funcAccept) _funcAccept(*reinterpret_cast<const LoginAccept *>(ptr), size, _sequence - 1);
    }    break;
    case LoginReject::tag:
        if (_funcReject) _funcReject(*reinterpret_cast<const LoginReject *>(ptr), size, 0 );
        break;
    case EndOfSession::tag:
        if (_funcEOFSession) _funcEOFSession(*reinterpret_cast<const EndOfSession *>(ptr), size, 0 );
        break;

    case SequencedDataHeader::tag:
    {
        
        if (_funcSeq) _funcSeq(*reinterpret_cast<const SequencedDataHeader *>(ptr), size, _sequence);  

        _sequence ++;
    } break;

    case UnsequencedDataHeader::tag:
    {    if (_funcUnseq) _funcUnseq(*reinterpret_cast<const UnsequencedDataHeader *>(ptr), size, 0 );  
    } break;
        
    default:
    {
         FMPRINT( "Soupbin:Unknown type: " , reinterpret_cast<Header*>(vptr)->type , ";", ::fastapp::Hex(ptr, size), "\n");
    } break;
    }
}

} // soupbin
} // fastapp

#endif

