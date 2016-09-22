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

#ifndef SOUPBIN_HDRS_H
#define	SOUPBIN_HDRS_H

#include <src/defs.h>

#include <src/soupbin/dispatcher.h>

#include <mutex>

namespace fastapp
{
class ConnectionBase;

namespace soupbin
{

namespace callbacks
{
    
class Handlers
{

public:

    Handlers (::fastapp::soupbin::Dispatcher & sbd)
    : _sbd(sbd)
    , _override_seq(false)
    , _session(nullptr)
    {
    }

    Handlers (::fastapp::soupbin::Dispatcher & sbd
    , ::fastapp::ConnectionBase * session)
    : _sbd(sbd)
    , _override_seq(false)
    , _session(session)
    {
    }

    Handlers (::fastapp::soupbin::Dispatcher & sbd
    , ::fastapp::ConnectionBase * session
    , bool override_seq_callback)
    : _sbd(sbd)
    , _override_seq(override_seq_callback)
    , _session(session)
    {
    }

    void registerCallbacks ()
    {
        registerSoupbinCallbacks () ;
    }

    void registerSoupbinCallbacks () ;

    void sub(auto f) {  _sbd.sub(f); }

    void set(const Login & login)
    {
        std::lock_guard<std::mutex> lock(_l_mutex);
        strncpy(_login.session, (char*)(login.session), session_size);
        strncpy(_login.seqnum , (char*)(login.seqnum) , seqnum_size);
    }

    const Login getLogin()
    {
        std::lock_guard<std::mutex> lock(_l_mutex);
        return _login;
    }
/* 
    void regstr(::fastapp::soupbin::Dispatcher::FuncLogin         f) {  _sbd.sub(f); } //_funcLogin = f;}
    void regstr(::fastapp::soupbin::Dispatcher::FuncAccept        f) {  _sbd.sub(f); } // _funcAccept= f;}
    void regstr(::fastapp::soupbin::Dispatcher::FuncReject        f) {  _sbd.sub(f); } // _funcReject= f;}
    void regstr(::fastapp::soupbin::Dispatcher::FuncLogout        f) {  _sbd.sub(f); } // _funcLogout= f;}
    void regstr(::fastapp::soupbin::Dispatcher::FuncEndoOfSession f) {  _sbd.sub(f); } //_funcEOFSession = f;}

    void regstr(::fastapp::soupbin::Dispatcher::FuncClientHB      f) {  _sbd.sub(f); } // _funcCliHB = f;}
    void regstr(::fastapp::soupbin::Dispatcher::FuncSvrHB         f) {  _sbd.sub(f); } // _funcSvrHB = f;}


    void regstr(::fastapp::soupbin::Dispatcher::FuncDebug         f) {  _sbd.sub(f); } // _funcDebug = f;}
    void regstr(::fastapp::soupbin::Dispatcher::FuncSoup          f) {  _sbd.sub(f); } // _funcSoup  = f;}
    void regstr(::fastapp::soupbin::Dispatcher::FuncSeqData       f) {  _sbd.sub(f); } // _funcSeq   = f;}  
    void regstr(::fastapp::soupbin::Dispatcher::FuncUnseq         f) {  _sbd.sub(f); } // _funcUnseq = f;}
*/
    void reg(::fastapp::callbacks::set_id_t f)    { setSessionID = f; }

    void reg_trace(::fastapp::callbacks::is_func_t  f)  { IsTraceOn     = f; }
    void reg_clihb(::fastapp::callbacks::is_func_t  f)  { IsCliHrtbtOn  = f; }
    void reg_svrhb(::fastapp::callbacks::is_func_t  f)  { IsSrvHrtbtOn  = f; }

    ::fastapp::callbacks::is_func_t                       IsTraceOn;
    ::fastapp::callbacks::is_func_t                       IsCliHrtbtOn;
    ::fastapp::callbacks::is_func_t                       IsSrvHrtbtOn;

protected:

    ::fastapp::callbacks::set_id_t                        setSessionID;

    ::fastapp::soupbin::Dispatcher &                      _sbd;

    bool                                                  _override_seq;

    std::mutex                                            _l_mutex;

    Login                                                 _login;

    ::fastapp::ConnectionBase      *                      _session;
};

}

}

}

#endif	/* SOUPBIN_HDRS_H */
