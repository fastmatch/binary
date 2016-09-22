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

#include <src/connection.h>
#include <src/socketutils.h>
#include <assert.h>
#include <src/print.h> 

namespace fastapp
{
    

int64_t ConnectionBase::timeNow()
{
    return ::fastapp::utils::getTimeNow(); 
}

int64_t ConnectionBase::timeOfDay()
{
    return ::fastapp::utils::getTimeOfDay();
}


std::string& ConnectionBase::toString(Protocol protocol)
{
    switch(protocol)
    {
        case Protocol::UNKNOWN:
        {
            static std::string u("UNKNOWN");
            return u;
        }   

        case Protocol::ITCH:
        {
            static std::string u("ITCH");
            return u;
        }

        case Protocol::OUCH:
        {
            static std::string u("OUCH");
            return u;
        }

        default: 
        {
            static std::string u("UNKNOWN");
            return u;
        }
    }
}

void  ConnectionBase::setReadTimeout(uint32_t no_data_sec)
{
    _read_timeout_sec = no_data_sec;
}

uint32_t  ConnectionBase::getReadTimeout() const
{
    return _read_timeout_sec;
}

void  ConnectionBase::setReconnectTimeout(uint32_t recon_sec)
{
    _recon_sec = recon_sec;
}

uint32_t  ConnectionBase::getReconnectTimeout() const
{
    return _recon_sec;
}

void ConnectionBase::setReconnectioniFlag(bool val)
{
    _reconnect_on = val;
}

bool ConnectionBase::getReconnectionFlag() const
{
    return _reconnect_on;
}

void  ConnectionBase::setKeepAlive(bool kalive)
{
    _keep_alive = kalive;
}

bool  ConnectionBase::getKeepAlive()
{
    return _keep_alive;
}

int ConnectionBase::getWindowSize()
{
    return _window_size;
}

void ConnectionBase::setWindowSize(int bsize)
{
    _window_size = bsize;
}

ConnectionBase::~ConnectionBase ()
{
}

ConnectionBase::ConnectionBase()
: localport(0)
, print_flag(0)
//, hbc(udp_hbc.msg)
,_block_mode(true)
,_sleep(0)
,_status(false)
, update_flag(0)
,_enabledListReq(true)
,_read_buffer_size(0)
,_write_buffer_size(0)
,_window_size(0)
,_read_timeout_sec(0)
,_recon_sec(0)
,_keep_alive(0)
,_seq(0)
,sock(-1)
,_cookie(0)
{
    _buf.resize(24*1024);

    using namespace fastapp::utils;
    using namespace fastapp::itch;

    print_flag |= PrintFlags::Book;
//    hbc.size = hbc.csize;
//    hbc.type = hbc.tag;
    udp_hbc.msg.size = udp_hbc.msg.csize;
    udp_hbc.msg.type = udp_hbc.msg.tag;
    udp_hbc.seq = 0;
    
    //_sub.sessionID  = "";
    setSessionID("");
    setSeqNum(1);
    //_sub.seqNum     = "1";
    setUpdateType(eUpdateType::Book);
    //_sub.updateType = "1";
    _sub.requestID=std::to_string((int64_t)(ConnectionBase::timeOfDay()%0x7FFFFFFF) );
    //_sub.actionType = "1";
    setActionType(eActionType::Subscribe);
    //_sub.subType    = "1";
    setSubscriptionType(eSubType::Aggregated);
    _sub.depth      = 0;
    update_flag |= (uint64_t)UpdateFlags::Book;    
    
}

template <>
bool TmplConnection<::fastapp::sockets::TCP>::isTcp() const
{
    return true;
}

template <>
bool TmplConnection<sockets::TCP>::initSocket()
{
    sockets::TCP wfd;
    if (sockets::create(wfd) != 0) return false;
    sock = wfd.fd;
    
    
    if (!localip.empty())
    {
        std::string inet;
        int err = sockets::getInterface(localip, inet);
        if (err == 0)
        {    sockets::bind(sock, inet.c_str(), localport);    }
        else 
        {
            FMPRINT( KRED , "Failed to get interface by name : " , localip , KNRM , "\n");
        }
    }
    
    // ::fastapp::sockets::setNonBlockMode<tcp>(sock);
    sockets::setNodelay(sock);
    
    if (_write_buffer_size != 0)
    {
        sockets::setSendBuffer(sock, _write_buffer_size);
    }
    
    if (_read_buffer_size != 0)
    {
        sockets::setReceiveBuffer(sock, _read_buffer_size);
    }

    if (_keep_alive)
    {
        sockets::setKeepAlive(sock);
    }

    if (_window_size != 0)
    {
        sockets::setWindowSize(sock, _window_size);
    }

    if (_block_mode && _read_timeout_sec > 0)
    {
         sockets::setReadTimeOut(sock, _read_timeout_sec);
    }
    return true;
}

template <>
bool TmplConnection<sockets::TCP>::setup()
{
    if (!resolve(remote_endpoint))
    {
        return false;
    }

    // Remote
    
    if (!initSocket()) return false;
    
    prep();
    
    return true;
}

template <>
bool TmplConnection<sockets::UDP>::initSocket()
{
    sockets::UDP wfd;
    if (sockets::create(wfd) != 0) return false;
    sock = wfd.fd;
 
    
    if (!localip.empty())
    {
        in_addr addr;
        if (0 == sockets::getInterface(localip, addr) )
        {
            sockets::bind(sock, addr, localport);
        }
    }
    
    // ::fastapp::sockets::setNonBlockMode<tcp>(sock);
    // ::fastapp::sockets::setNodelay<udp>(sock);
    
    if (_write_buffer_size != 0)
    {
        sockets::setSendBuffer(sock, _write_buffer_size);
    }
    
    if (_read_buffer_size != 0)
    {
        sockets::setReceiveBuffer(sock, _read_buffer_size);
    }

    return true;
}

template <>
bool TmplConnection<sockets::UDP>::setup()
{
    if (!resolve(remote_endpoint))
    {
        return false;
    }
    // Remote
    
    if (!initSocket()) return false;
    
    prep();
    
    return true;
}

bool ConnectionBase::isConnected() const
{
    if (good() && sockets::is_open(sock) == 0)
    {    return true;    }
    else return false;
}

bool ConnectionBase::connect()
{
    if (IsTraceOn()) FMPRINT( KRED , "Make socket blocking to process connect in a sync way"  , KNRM ,"\n");
    sockets::setBlockMode(sock); 
    
    int error = sockets::connect(sock, remote_endpoint);
    if (error != 0)
    {
        FMPRINT( KRED , "Failed to connect  " , (!localip.empty() ? std::string("") : std::string("from '") + localip + std::string("'") ), remote_endpoint.to_string() , " with error : " , error , KNRM ,"\n");
        return false;
    }

    if (!_block_mode)
    {
        sockets::setNonBlockMode(sock); 
    }

    _status = true;

    if (_onConnect)
    {
        try
        {
           _onConnect(this);
        }
        catch(...)
        {
            FMPRINT( KRED , "OnConnect", "Failed", " with exception", KNRM);
        }
    }

    return true;
}

template <>
bool TmplConnection<sockets::TCP>::disconnect()
{
    bool connected = isConnected() || good();

    ::fastapp::sockets::closeSocket(sock);

    if (connected && _onDisconnect)
    {
        try
        {
           _onDisconnect(this);
        }catch(...)
        {
            FMPRINT( KRED , "OnDisconnect", "Failed", " with exception", KNRM);
        }
    }

    disable();

    return true;

}

template <>
bool TmplConnection<sockets::UDP>::disconnect()
{   bool connected = isConnected() || good();
    ::fastapp::sockets::shutdown(sock);
    ::fastapp::sockets::close(sock);

    disable();

    if (connected && _onDisconnect)
    {
        _onDisconnect(this);
    }

    return true;
}

template <>
bool TmplConnection<sockets::TCP>::run_once()
{
    if (!isInitialized()) 
    {
        FMPRINT( KRED , "processor is not set, exiting" , KNRM , "\n");
        return false;
    }
    
    if (!good())
    {
        return false;
    }

    static const size_t header_size = sizeof(fastapp::soupbin::Header);
    size_t consumed    = _buf.consumed();
    ssize_t len = 0;
    static ssize_t ctr = -1;
    ctr ++;
    tod = timeOfDay();  
    
    bool log = IsTraceOn() && IsLowLevelOn();
            
    if (log && ((tod - timeLastRcvd) % fivemill> fivemill - 1) && (ctr%512 == 0) )
    {
        FMPRINT( KGRN , Micro(tod) , " : "," : " , (tod - timeLastRcvd) / fivemill , "." , (tod - timeLastRcvd) % fivemill , ":" , fivemill - 15 , ":" , ctr , " Before checking for header consumed=", consumed ,", expected=" , _buf.expected_size() , KNRM , "\n");
    }      
    
    if (consumed < header_size)
    {
        if (consumed == 0) _buf.expected_size(header_size);

        if (log && ((tod - timeLastRcvd) % fivemill> fivemill - 1) && (ctr%512 == 0) )
        {
            FMPRINT( KGRN , Micro(tod) ,  " : " , ctr , " Before Header read ; trying to read=", _buf.expected_size()  , KNRM , "\n");
        }   
        
        if ( (len = ::fastapp::sockets::recv(sock, _buf.write_ptr(), _buf.expected_size(), IsLowLevelOn() )) != 0)
        {
            if (log && ((tod - timeLastRcvd) % fivemill> fivemill - 1) && (ctr%512 == 0) )
            {
                FMPRINT( KGRN , Micro(tod) ,  " : " , ctr , " ; trying to read=", _buf.expected_size()  , "; actually read=" , len , KNRM , "\n");
            }   
            
            if (len < 0) return true;
            tod = timeOfDay();            

            consumed = _buf.consumed(len);
            
            if (consumed < header_size  )
            {
                return true;
            }
            
            assert(consumed == header_size);
            if (consumed != header_size )
            {
                int64_t timeNow = TmplConnection::timeNow();
                
                FMPRINT( tod , " time:" , timeNow , "; unable to read packet size+type, exiting " , len , "!=" ,sizeof(fastapp::soupbin::Header) , "\n");
                return false;
            }
            
            fastapp::soupbin::Header * hdr = header_ptr<fastapp::soupbin::Header>();
            assert((void *)hdr == (void *)_buf.read_ptr() );
            assert(hdr->size == *(int16_t*)((void *)_buf.read_ptr()) );
            
            if (hdr->size <= 0)
            {
                FMPRINT( KRED , Micro(tod) ,  " : " , " ; expected=", _buf.expected_size()  , "; consumed=" , _buf.consumed()  , " ; hdr->size=" , hdr->size , " ; unsigned hdr->size=" , (unsigned)hdr->size , KNRM , "\n");
                using namespace fastapp::utils;
                print_incoming(tod, _buf.read_ptr(), consumed);
                return false;
            }
            assert(hdr->size > 0);
            _buf.expected_size(hdr->size - 1);
            timeLastRcvd = tod;
            
        } else return false;
    }
    
    if (log && ((tod - timeLastRcvd) % fivemill> fivemill - 1) && (ctr%512 == 0) )
    {
        FMPRINT( KGRN  , Micro(tod) , " : " , ctr , " Before READ/PROCESS BODY consumed=", consumed ,", expected=" , _buf.expected_size() , KNRM , "\n");
    }      

    
    if ( consumed >= header_size )
    {
        int64_t timeNow = TmplConnection::timeNow();
        tod = timeOfDay();        

        //FMPRINT( "tod=" , tod , "; timeNow - timeLastSent(" , timeNow-timeLastSent , "): delta=" ,  time_delta , "\n");
        
        if (tod - timeLastSent > time_delta)
        {
            using namespace fastapp::utils;
            
            send(udp_hbc.msg, tod);
            
            if ( (print_flag & PrintFlags::ClientHrtBeat) != 0) 
            {
                if (IsTraceOn()) print_outgoing(tod, udp_hbc.msg, " (ClientHB) '");
                FMPRINT( KGRN  , Micro(tod) , " local->" , remoteip , ":" , remoteport , "  ClientHB " , KNRM , "\n");
            }
        }
        
        fastapp::soupbin::Header * hdr = header_ptr<fastapp::soupbin::Header>();

        if (hdr->size > 1)
        {
            if (hdr->size > _buf.size())
            {
                _buf.resize(hdr->size + sizeof(int16_t));
                hdr = header_ptr<fastapp::soupbin::Header>();
                FMPRINT( KRED , tod ,  "Resizing buffer to [" , hdr->size + sizeof(int16_t) , "] " , KNRM , "\n");
                if (IsTraceOn() && IsLowLevelOn())
                {
                    FMPRINT( KRED , tod , "  time:" , timeNow , "; Packet size:" , hdr->size , "; type='" , hdr->type , "," , (int)hdr->type ,\
                    "'; trying to read=", sizeof(fastapp::soupbin::Header) , "; actual len=" , len , KNRM , "\n");
                }                    
            }
            
            if (log && ((tod - timeLastRcvd) % fivemill> fivemill - 1) && (ctr%512 == 0) )
            {
                FMPRINT( KGRN  , Micro(tod) , " : " , ctr , " Before reading BODY ; Packet size:" , hdr->size , "; type='" , hdr->type , "," , (int)hdr->type , \
                "'; trying to read=", _buf.expected_size()  , KNRM , "\n");
            }   
            
            if ((len = ::fastapp::sockets::recv(sock, _buf.write_ptr(), _buf.expected_size(), IsLowLevelOn() )) == 0) return false;
            
            if (log && ((tod - timeLastRcvd) % fivemill> fivemill - 1) && (ctr%512 == 0) )
            {
                FMPRINT( KGRN  , Micro(tod) , " : " , ctr , " ; Packet size:" , hdr->size , "; type='" , hdr->type , "," , (int)hdr->type , \
                "'; trying to read=", _buf.expected_size()  , "; actually read=" , len , KNRM , "\n");
            }                
            if (len < 0) return true;

            consumed = _buf.consumed(len);
            ssize_t expected = _buf.expected_size();
            
            if (expected == 0)
            {
                _buf.reset();
                ctr = -1;
                assert(consumed < (size_t)_buf.size());
                process(_buf.read_ptr(), consumed);
                timeLastRcvd = tod;
            }
            else if (expected < 0)
            {
                FMPRINT( KRED , tod , "  time:" , timeNow , "; Failed to read size:" , hdr->size - 1 , "; instead read=" , consumed , " stil expecting to read" ,  _buf.expected_size() , KNRM , "\n");
                exit (-1);
            }
        }
        else
        {
            _buf.reset();
            ctr = -1;
            assert(consumed < (size_t)_buf.size());
            process(_buf.read_ptr(), consumed);
            timeLastRcvd = tod;
        }

        
        {
            using namespace fastapp::utils;
            if (IsSrvHbtOn() && _buf.expected_size() == 0 && hdr->type == fastapp::soupbin::ServerHeartbeat::tag )
            {
                if (hdr->size + sizeof(hdr->size) != consumed)
                {    
                    bool cond = (hdr->size + sizeof(hdr->size) != consumed);
                    FMPRINT( (cond ? KRED:KNRM )\
                            , hdr->size + sizeof(hdr->size) \
                            , "==" \
                            , consumed \
                            , "="  \
                            , (cond ? "false" : "true" )\
                            , KNRM \
                            , "\n");    
                }
                
                assert(hdr->size + sizeof(hdr->size) == consumed);

                if (IsTraceOn()) print_incoming(tod, _buf.read_ptr(), consumed);
                FMPRINT( KGRN  , Micro(tod) , " " , remoteip , ":" , remoteport , "->local  SvrHB " , KNRM , "\n");

            }
            else if ( IsTraceOn() && _buf.expected_size() == 0)
            {
                bool cond = (hdr->size + sizeof(hdr->size) != consumed);
                FMPRINT( Micro(tod) , (cond ? KRED:KNRM) , " " , remoteip , ":" , remoteport , "->local : " , hdr->size + sizeof(hdr->size) , "==" , consumed , "=" , (cond ? "false" : "true") , KNRM , "\n");
                assert(hdr->size + sizeof(hdr->size) == consumed);
                print_incoming(tod, _buf.read_ptr(), consumed);
            }
        }


        
        if (tod - timeLastRcvd > 2 * onesec)
        {
            FMPRINT( KRED , tod , " " , remoteip , ":" , remoteport , " connection broken " , KNRM , "\n");
            return false;
        }
        
        return true;
    }       
    return false;
}

template <>
bool TmplConnection<sockets::UDP>::run_once()
{
    if (!isInitialized()) 
    {
        FMPRINT( KRED , "processor is not set, exiting" , KNRM , "\n");
        return false;
    }
    
    static const size_t header_size = sizeof(fastapp::soupbin::UdpHeader);
    size_t consumed    = 0;
    ssize_t len = 0;
    static ssize_t ctr = -1;
    ctr ++;
    tod = timeOfDay();  
    
    bool log = IsTraceOn() && IsLowLevelOn();
            
    
    if ((ssize_t)_buf.expected_size() < _buf.size()) _buf.expected_size(_buf.size());

    fastapp::soupbin::UdpHeader * udp_hdr = header_ptr<fastapp::soupbin::UdpHeader>();
    fastapp::soupbin::Header    * hdr = &(udp_hdr->hdr);

    if ((ssize_t)_buf.expected_size() > _buf.size())
    {
        _buf.resize(_buf.expected_size());

        FMPRINT( KRED , tod ,  "Resizing buffer to [" , _buf.expected_size() , "] " , KNRM , "\n");
    }

    if (log && ((tod - timeLastRcvd) % fivemill> fivemill - 1) && (ctr%512 == 0) )
    {
        FMPRINT( KGRN  , Micro(tod) , " : "," : " , (tod - timeLastRcvd) / fivemill , "." , (tod - timeLastRcvd) % fivemill , ":" , fivemill - 15 , ":" , ctr , " Before  rcvfrom, consumed=", consumed ,", expected=" , _buf.expected_size() , KNRM , "\n");
    }      
       
    if ( (len = ::fastapp::sockets::recvfrom(sock, _buf.write_ptr(), _buf.size(), remote_endpoint, IsLowLevelOn() )) != 0)
    {
        if (log && ((tod - timeLastRcvd) % fivemill> fivemill - 1) && (ctr%512 == 0) )
        {
            FMPRINT( KGRN  , Micro(tod) ,  " : " , ctr , " ; tried reading =", _buf.expected_size()  , "; actually read=" , len , KNRM , "\n");
        }   

        if (len < 0) return true;
        
        tod = timeOfDay();            

        consumed = _buf.consumed(len);

        if (consumed < header_size  )
        {
            if (log)
            {
                FMPRINT( KRED , Micro(tod) ,  " : " , " ; supposed to read at least '", header_size , "' but  actually read='" , len , "'" , KNRM , "\n");
            }

            return false;
        }

        assert((void *)udp_hdr == (void *)_buf.read_ptr() );
        assert(hdr->size == *(int16_t*)( (void *)(_buf.read_ptr() + sizeof(udp_hdr->seq))) );

        if (hdr->size <= 0)
        {
            FMPRINT( KRED  , Micro(tod) ,  " : " , " ; expected=", _buf.expected_size()  , "; consumed=" , _buf.consumed()  , " ; hdr->size=" , hdr->size , " ; unsigned hdr->size=" , (unsigned)hdr->size , KNRM , "\n");
            using namespace fastapp::utils;
            print_incoming(tod, _buf.read_ptr(), consumed);
            return false;
        }
        
        timeLastRcvd = tod;

    } else return false;
    
    if ( consumed >= header_size )
    {
        tod = timeOfDay();        

        if (tod - timeLastSent > time_delta)
        {
            using namespace fastapp::utils;
            
            if ( (print_flag & PrintFlags::ClientHrtBeat) != 0 && IsTraceOn() ) 
            {
                FMPRINT( Micro(tod) , ": Out type=<" , udp_hbc.msg.type , ">; HBC size:" , sizeof(udp_hbc) , "\n");
            }
            
            udp_hbc.seq = nextSequence();
            send(udp_hbc, tod);
            
            if ( (print_flag & PrintFlags::ClientHrtBeat) != 0) 
            {
                FMPRINT( Micro(tod) ); 
                if (IsTraceOn()) { print_outgoing(tod, udp_hbc, " (ClientHeartbeat) '");  }
                else FMPRINT( "   Out (ClientHeartbeat) " , "\n");
            }
        }

        
        _buf.reset();
        ctr = -1;
        assert(consumed < (size_t)_buf.size());
        process(_buf.read_ptr(), consumed);
        timeLastRcvd = tod;

        
        {
        using namespace fastapp::utils;
        if (IsSrvHbtOn() && consumed >= sizeof(udp_hdr) && hdr->type == fastapp::soupbin::ServerHeartbeat::tag )
        {
            bool cond = (hdr->size + sizeof(hdr->size) + sizeof(udp_hdr->seq) != consumed);
            if (cond)
            {    
                FMPRINT( (cond ? KRED:KNRM ) \
                        , "("\
                        , hdr->size + sizeof(hdr->size) \
                        , "==" \
                        , consumed \
                        , ")="  \
                        , (cond ? "false" : "true" )\
                        , KNRM \
                        , "\n");
            }

            assert(!cond);

            FMPRINT( KGRN ,  Micro(tod) , " " , remoteip , ":" , remoteport , "; seq=" , udp_hdr->seq ,  " In (SvrHB)" , KNRM , "\n");
            if (IsTraceOn()) print_incoming(tod, _buf.read_ptr(), consumed);
        }
        else if ( IsTraceOn() && consumed >= sizeof(udp_hdr))
        {
            bool cond = (hdr->size + sizeof(hdr->size) + sizeof(udp_hdr->seq) != consumed);
            FMPRINT( (cond ? KRED:KNRM)   , Micro(tod) , " " , remoteip , ":" , remoteport , "; seq=" , udp_hdr->seq , " : " , hdr->size + sizeof(hdr->size) + sizeof(udp_hdr->seq) , "==" , consumed , "=" , (cond ? "false" : "true") , KNRM , "\n");
            //assert(cond);
            print_incoming(tod, _buf.read_ptr(), consumed);
        }
        }


        
        if (tod - timeLastRcvd > 2 * onesec)
        {
            FMPRINT( KRED , tod , " " , remoteip , ":" , remoteport , " connection broken " , KNRM , "\n");
            return false;
        }
        
        return true;
    }       
    return false;
}


} // fastapp


