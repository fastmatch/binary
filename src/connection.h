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

#ifndef CONNECTION_H
#define	CONNECTION_H

//standard
#include <iostream>
#include <vector>
#include <queue>
#include <chrono>
#include <thread>
#include <string>

//my
#include <src/defs.h>
#include <src/socketutils.h>
#include <src/itch/itch.h>
#include <src/soupbin/soupbin.h>
//#include <src/soupbin/dispatcher.h>
//#include <src/itch/dispatcher.h>
// #include <src/handlers.h>
#include <src/buffer.h>
#include <src/print.h>

namespace fastapp
{

enum class UpdateFlags : uint64_t
{
    Unknown = 0,
    Book    = 1,
    Trade   = 2,
    Midpoint = 4,
    Onclose = 8
};


    
struct SubParams
{
    std::string                sessionID;
    std::string                seqNum;
    ::fastapp::itch::eUpdateType  updateType; //'1'...-'4'
    std::string                requestID;
    ::fastapp::itch::eActionType  actionType;
    ::fastapp::itch::eSubType     subType;
    int8_t                     depth;
};

class ConnectionBase
{
public:
    typedef SubParams Sub;
    typedef std::function< void (const TcpLogin & login) > FuncOnLoginRequest;
    typedef std::function< void (ConnectionBase *) > FuncOnConnect;
    typedef std::function< void (ConnectionBase *) > FuncOnDisconnect;
    
    enum class Protocol : int8_t 
    {
         UNKNOWN='U'
       , ITCH='I'
       , OUCH='O'
    };

    enum class Connectivity : int8_t 
    {
         UNKNOWN='U'
       , TCP='T'
       , UDP='U'
    };
    
    static std::string& toString(Protocol protocol);
    
    static constexpr int64_t onesec   = 1000000;
    static constexpr int64_t fivemill = 5000;
    static constexpr int64_t time_delta = onesec - fivemill;
    typedef std::function< void (void *ptr, int32_t size) > process_t;
    typedef std::function< int (const std::string & source, const std::string & subject, const std::string & text, const std::string & remote_ip, int remote_port) > send_mail_t;

    ConnectionBase ();
    ConnectionBase (const ConnectionBase & c) = default;
    ConnectionBase (ConnectionBase && c) = default;
    ConnectionBase & operator = (const ConnectionBase & c) = default;
    ConnectionBase & operator = (ConnectionBase && c) = default;
    virtual ~ConnectionBase ();

    
    // helpers
    static int64_t timeNow();
    static int64_t timeOfDay();

    // accessors
    bool IsTraceOn() const;
    bool IsSrvHbtOn() const;
    bool IsCliHbtOn() const;
    bool IsBookOn() const;
    bool IsLowLevelOn() const;
    bool IsOuchOn() const;
        
    bool isInitialized() const;
    
    
    const std::string & getUserName() const;
    const std::string & getPassword() const;
    
    bool good() const;
    void disable();
    
    void setDelay(int64_t delay);

    const std::string & getRemoteHost() const;
    const std::string & getRemoteIP() const;
    int getRemotePort() const;
    const std::string & getLocalHost() const;
    const std::string & getLocalIP() const;
    int getLocalPort() const;
 
    void setRemoteIP(const std::string & ip);
    void setRemotePort(int port);

    void setLocalIP(const std::string & ip);
    void setLocalPort(int port);
    
    const std::string & getProtocolStr() const;
    Protocol getProtocol() const;
    void setProtocol(Protocol protocol);
    
    void setUsername(const std::string user);
    void setPassword(const std::string pass);
    
    uint64_t getPrintFlag() const;
    void setPrintFlag(uint64_t flag);
    
    const std::string & getSessionID() const;
    void setSessionID(const std::string & sessionID);

    const std::string & getSeqNum() const;
    void setSeqNum(size_t seqnum) ;
    
    fastapp::itch::eUpdateType getUpdateType() const;
    void setUpdateType(fastapp::itch::eUpdateType utype);
    
    const std::string & getRequestID() const;
    void setRequestID(const std::string & reqid);
    
    fastapp::itch::eActionType  getActionType() const;
    void setActionType(fastapp::itch::eActionType atype);
    
    fastapp::itch::eSubType  getSubType() const;
    void setSubType(fastapp::itch::eSubType subtype);
    void setSubscriptionType(fastapp::itch::eSubType subtype);
    
    int8_t getDepth() const;
    void setDepth(int8_t depth);
    
    const Sub & getSubscription() const;
    
    bool listEnabled() const;
    void setListReqMod(bool mode);
    
    const std::string & getSymbols() const;
    void setSymbols(const std::string symbols);
    
    bool IsBookSub() const;
    bool IsTradeSub() const;
    bool IsMidpointSub() const;
    bool IsOnCloseSub() const;
            
    uint64_t getUpdateFlag() const;
    void setUpdateFlag(uint64_t flag);
    
    void setMailSender(send_mail_t send_email);
    
    int sendMail(const std::string & source, const std::string & subject, const std::string & text);
    
    void setReadBufferSize(size_t);
    void setWriteBufferSize(size_t);
    
    void setProcessor(process_t p);
    
    void setBlockingMode(bool bmode);

    int32_t nextSequence()
    {
        return _seq++;
    }

    int32_t lastSequence()
    {
        return _seq;
    }

    int32_t lastSeq()
    {
        return _seq;
    }

    void activateBlockingMode();
    bool resolve(::fastapp::sockets::Endpoint & remoteendpoint);
    void prep ();
    ssize_t send(auto & msg, int64_t cur_time)
    {
        return send_impl(msg, cur_time);
    }

    bool send_recv (auto & msg, int64_t cur_time);

    ssize_t send(TcpLogin & msg, int64_t cur_time)
    {
        if (_onLogin)
        {
            _onLogin(msg);
        }
        return send_impl(msg, cur_time);
    }


    ssize_t send(UdpLogin & msg, int64_t cur_time)
    {
        if (_onLogin)
        {
            _onLogin(msg.msg);
        }

        return send_impl(msg, cur_time);
    }

    bool connect ();
    bool isConnected() const;

    void setReadTimeout(uint32_t no_data_sec);
    uint32_t getReadTimeout() const; // sec

    void setReconnectTimeout(uint32_t recon_sec);
    uint32_t getReconnectTimeout() const;
    void setReconnectioniFlag(bool val);
    bool getReconnectionFlag() const;

    void setKeepAlive(bool kalive);
    bool getKeepAlive();
    int getWindowSize();
    void setWindowSize(int bsize);



    virtual bool setup() = 0;

    // network communication
    virtual bool disconnect() = 0;
    virtual void run () = 0;
    virtual bool run_once() = 0;

    virtual bool isTcp() const = 0;

    void attach(std::shared_ptr<void> p) { _cookie = p; }

    std::shared_ptr<void> getVoidProcessor() { return _cookie; }

    void registerOnSend(FuncOnLoginRequest onLogin) { _onLogin = onLogin; }
    void registerOnConnect(FuncOnConnect onConnect) { _onConnect = onConnect; }
    void registerOnDisconnect(FuncOnDisconnect onDisconnect) { _onDisconnect = onDisconnect; }
    int getSocket() const {return sock;}
protected:
    // setup
    virtual bool initSocket() = 0;
    ssize_t send_impl(auto & msg, int64_t cur_time);
protected:

    // Data memebers    
    std::string             localip;
    std::string             remoteip;
    int                     localport;
    int                     remoteport;    
    
    InputBuffer             _buf;
    
    process_t               process;

    uint64_t                print_flag;
    fastapp::soupbin::UdpSoupBin<fastapp::soupbin::ClientHeartbeat > udp_hbc;
//    fastapp::soupbin::ClientHeartbeat hbc;
    
    int64_t                 timeLastRcvd;
    int64_t                 tod;
    int64_t                 timeLastSent; // ::fastapp::utils::getTimeNow();
    
    std::string             _username;
    std::string             _password;
    bool                    _block_mode;
    bool                    _reconnect_on;
    int64_t                 _sleep;

    bool                    _status;
    Protocol                _protocol;
    
    uint64_t                 update_flag;
    Sub                     _sub;
    std::string             _symbol;    
    bool                    _enabledListReq;    
    size_t                  _read_buffer_size;
    size_t                  _write_buffer_size;
    ssize_t                 _window_size;
    uint32_t                _read_timeout_sec;
    uint32_t                _recon_sec;
    bool                    _keep_alive;


    send_mail_t             send_mail;
    int32_t                 _seq;
    int                     sock;
    ::fastapp::sockets::Endpoint          remote_endpoint;
    std::shared_ptr<void>  _cookie;
    FuncOnLoginRequest     _onLogin;
    FuncOnConnect          _onConnect;
    FuncOnDisconnect       _onDisconnect;
};

template <typename PROTOCOL>
class TmplConnection : public ConnectionBase
{
public:

    
//    static std::string& toString(Protocol protocol);
    
//    static constexpr int64_t onesec   = 1000000;
//    static constexpr int64_t fivemill = 5000;
//    static constexpr int64_t time_delta = onesec - fivemill;
//    typedef std::function< void (void *ptr, int32_t size) > process_t;
//    typedef std::function< int (const std::string & source, const std::string & subject, const std::string & text, const std::string & remote_ip, int remote_port) > send_mail_t;
    
    //TmplConnection(process_t p);
    TmplConnection () : ConnectionBase () {};
    TmplConnection (const TmplConnection & c) = default;
    TmplConnection (TmplConnection && c) = default;
    TmplConnection & operator = (const TmplConnection & c) = default;
    TmplConnection & operator = (TmplConnection && c) = default;
    ~TmplConnection () = default;
    
    // setup

    bool setup() override;
    
    // network communication
    bool disconnect() override;
    void run () override;
    bool run_once() override;

    bool isTcp() const override;

protected:    
    // setup
    bool initSocket() override;
    
    // buffer mgmt
    template <typename T>
    T* header_ptr()
    {
        T * ptr = reinterpret_cast<T *>(_buf.read_ptr());
        return ptr; 
    }
    
  
protected:
    // Data memebers    
};

inline ssize_t ConnectionBase::send_impl(auto & msg, int64_t cur_time)
{
    timeLastSent = cur_time;
    ssize_t size = sizeof( decltype(msg) ), rsize = size;
    char * ptr = (char *)((void *)&msg);
 
    do
    {    
        ssize_t ret = ::fastapp::sockets::send(sock, ptr, size, IsLowLevelOn() );
        if (ret <= 0)
        {    return ret;    }
        else // (ret > 0) 
        {
            size -= ret; 
            ptr  += ret;
        }
    }
    while (size > 0);

    if (size < 0)
    {
        throw ::fastapp::sockets::SocketError("sent more than requested", -1);
    }

    return rsize;
}

template <typename PROTOCOL>
bool TmplConnection<PROTOCOL>::isTcp () const
{
    return false;
}


inline bool ConnectionBase::send_recv (auto & msg, int64_t cur_time)
{
    ssize_t sret = send(msg, cur_time);
    bool ret = run_once();
    return (sret > 0) && ret;
}


inline void ConnectionBase::setReadBufferSize(size_t bsize)
{
    _read_buffer_size = bsize;
}

inline void ConnectionBase::setWriteBufferSize(size_t bsize)
{
    _write_buffer_size = bsize;
}


inline uint64_t ConnectionBase::getUpdateFlag() const
{
    return update_flag;
}


inline void ConnectionBase::setUpdateFlag(uint64_t flag)
{
    update_flag = flag;
}
    

inline void ConnectionBase::setMailSender(ConnectionBase::send_mail_t send_email)
{
    send_mail = send_email;
}


inline uint64_t ConnectionBase::getPrintFlag() const
{
    return print_flag;
}

inline void ConnectionBase::setPrintFlag(uint64_t flag)
{
    print_flag = flag;
}

inline const std::string & ConnectionBase::getRemoteHost() const
{
    return remoteip;
}

inline const std::string & ConnectionBase::getRemoteIP() const
{
    return remoteip;
}

inline int ConnectionBase::getRemotePort() const
{
    return remoteport;
}

inline const std::string & ConnectionBase::getLocalHost() const
{
    return localip;
}

inline const std::string & ConnectionBase::getLocalIP() const
{
    return localip;
}

inline int ConnectionBase::getLocalPort() const
{
    return localport;
}

inline void ConnectionBase::setProcessor(process_t p)
{
    process = p;
}

inline bool ConnectionBase::IsTraceOn() const
{
    using namespace fastapp::utils;
    return ((PrintFlags::Trace & print_flag) != 0);
}

inline bool ConnectionBase::IsSrvHbtOn() const
{
    using namespace fastapp::utils;
    return ((PrintFlags::ServerHrtBeat & print_flag) != 0);
}

inline bool ConnectionBase::IsCliHbtOn() const
{
    using namespace fastapp::utils;
    return ((PrintFlags::ClientHrtBeat & print_flag) != 0);
}

inline bool ConnectionBase::IsBookOn() const
{
    using namespace fastapp::utils;
    return ((PrintFlags::Book & print_flag) != 0);
}

inline bool ConnectionBase::IsLowLevelOn() const
{
    using namespace fastapp::utils;
    return ((PrintFlags::Lowlevel & print_flag) != 0);
}

inline bool ConnectionBase::IsOuchOn() const
{
    using namespace fastapp::utils;
    return ((PrintFlags::Ouch & print_flag) != 0);
}

inline const std::string & ConnectionBase::getUserName() const
{
    using namespace fastapp::utils;
    return _username;
}

inline const std::string & ConnectionBase::getPassword() const
{
    using namespace fastapp::utils;
    return _password;
}

inline bool ConnectionBase::good() const
{
    using namespace fastapp::utils;
    return _status ;
}

inline bool ConnectionBase::isInitialized() const
{
    return process.operator bool ();
}

inline void ConnectionBase::disable()
{
    _status = false;
    sock = -1;
}

inline void ConnectionBase::setDelay(int64_t delay) // time in millis
{
    _sleep = delay;
}

inline void ConnectionBase::setProtocol(ConnectionBase::Protocol protocol)
{
    _protocol = protocol;
}

inline typename ConnectionBase::Protocol ConnectionBase::getProtocol() const
{
    return _protocol;
}

inline const std::string & ConnectionBase::getProtocolStr() const
{
    return ConnectionBase::toString(_protocol);
}

inline void ConnectionBase::setRemoteIP(const std::string & ip)
{
    remoteip = ip;
}

inline void ConnectionBase::setRemotePort(int port)
{
    remoteport = port;    
}

inline void ConnectionBase::setLocalIP(const std::string & ip)
{
    localip = ip;
}

inline void ConnectionBase::setLocalPort(int port)
{
    localport = port;
}

inline void ConnectionBase::setUsername(const std::string user)
{
    _username = user;
}

inline void ConnectionBase::setPassword(const std::string pass)
{
    _password = pass;
}

inline const std::string & ConnectionBase::getSessionID() const
{
    return _sub.sessionID;
}

inline void ConnectionBase::setSessionID(const std::string & sessionID)
{
    if (sessionID.empty())
    {
        _sub.sessionID.resize(fastapp::soupbin::seqnum_size, ' ');
    }
    {
        _sub.sessionID = sessionID;
        if (_sub.sessionID.size() > fastapp::soupbin::seqnum_size)
        {
            _sub.sessionID.resize(fastapp::soupbin::seqnum_size);
        }
    }
    if (IsLowLevelOn()) { FMPRINT( "; setSessionID to [" , sessionID , "]" , "\n"); }
}

inline const std::string & ConnectionBase::getSeqNum() const
{
    return _sub.seqNum;
}

inline void ConnectionBase::setSeqNum(size_t seqnum)
{
    _sub.seqNum = std::to_string(seqnum);
    size_t len = _sub.seqNum.size();
    if (len < fastapp::soupbin::seqnum_size)
    {
        _sub.seqNum.insert(0, fastapp::soupbin::seqnum_size - len, ' ');
    }
    else if (len > fastapp::soupbin::seqnum_size)
    {
        _sub.seqNum.resize(fastapp::soupbin::seqnum_size);
    }
}

inline fastapp::itch::eUpdateType ConnectionBase::getUpdateType() const
{
    return _sub.updateType; //'1'...-'4'
} 

inline void ConnectionBase::setUpdateType(fastapp::itch::eUpdateType utype)
{
    _sub.updateType = utype; //'1'...-'4'
}

inline const std::string & ConnectionBase::getRequestID() const
{
   return _sub.requestID;
}

inline void ConnectionBase::setRequestID(const std::string & reqid)
{
    _sub.requestID = reqid;
}

inline fastapp::itch::eActionType ConnectionBase::getActionType() const
{
    return _sub.actionType;
}

inline void ConnectionBase::setActionType(fastapp::itch::eActionType atype)
{
    _sub.actionType = atype;
}

inline fastapp::itch::eSubType ConnectionBase::getSubType() const
{
    return _sub.subType;
}

inline void ConnectionBase::setSubType(fastapp::itch::eSubType subtype)
{
    _sub.subType = subtype;
}

inline void ConnectionBase::setSubscriptionType(fastapp::itch::eSubType subtype)
{
    _sub.subType = subtype;
}

inline int8_t ConnectionBase::getDepth() const
{
    return _sub.depth;
}

inline void ConnectionBase::setDepth(int8_t depth)
{
    _sub.depth = depth;
}

inline const ConnectionBase::Sub & ConnectionBase::getSubscription() const
{
   return _sub;
}

inline bool ConnectionBase::listEnabled() const
{
    return _enabledListReq;
}

inline void ConnectionBase::setListReqMod(bool mode)
{
    _enabledListReq = mode;
}

inline const std::string & ConnectionBase::getSymbols() const
{
    return _symbol;
}

inline void ConnectionBase::setSymbols(const std::string symbols)
{
    _symbol = symbols;
}

inline bool ConnectionBase::IsBookSub() const
{
    return (((uint64_t)UpdateFlags::Book & update_flag) != 0);
}

inline bool ConnectionBase::IsTradeSub() const
{
    return (((uint64_t)UpdateFlags::Trade & update_flag) != 0);
}

inline bool ConnectionBase::IsMidpointSub() const 
{
    return (((uint64_t)UpdateFlags::Midpoint & update_flag) != 0);
}

inline bool ConnectionBase::IsOnCloseSub() const 
{
    return (((uint64_t)UpdateFlags::Onclose & update_flag) != 0);
}

inline int ConnectionBase::sendMail(const std::string & source, const std::string & subject, const std::string & text)
{
    if (send_mail.operator bool())
    {
        return send_mail(source, subject, text, remoteip, remoteport);
    }
    
    return -1;
}

inline void ConnectionBase::setBlockingMode(bool bmod)
{
    _block_mode = bmod;
}

inline void ConnectionBase::activateBlockingMode()
{
    if (_block_mode) ::fastapp::sockets::setBlockMode(sock);
}

inline bool ConnectionBase::resolve(::fastapp::sockets::Endpoint & remoteendpoint)
{
     
    remote_endpoint.address().sin_family      = AF_INET;
    remote_endpoint.address().sin_port        = htons(remoteport);
 
    bool ret = ::fastapp::sockets::resolve(remoteip, remoteendpoint.address().sin_addr);
    if (!ret)
    {
        FMPRINT( "Failed to resolve " , remoteip , "\n");
    }
    else
    {
        if (IsLowLevelOn()) 
        { 
            FMPRINT( "Resolved " , remoteip , ":" , remoteport , " to end_point: " , remoteendpoint.to_string() , "\n");
        }
    }
    return ret;
}

inline void ConnectionBase::prep ()
{
    //hdr = header_ptr<fastapp::soupbin::Header>();
    timeLastRcvd = 0;
    tod = timeOfDay();
    timeLastSent = tod; // ::fastapp::utils:::getTimeNow();
}

template <typename PROTOCOL>
void TmplConnection<PROTOCOL>::run ()
{
    activateBlockingMode();

    printf("sleep in millis: %d\n", (int)_sleep);
    //int64_t last = timeOfDay();
    while(run_once())
    {
        if (_sleep > 0)
        {
            std::this_thread::sleep_for( std::chrono::milliseconds(_sleep));;
            //sleep(_sleep/1000);
        }
    }
}

typedef TmplConnection<::fastapp::sockets::TCP> Connection;
typedef TmplConnection<::fastapp::sockets::TCP> TcpConnection;
typedef TmplConnection<::fastapp::sockets::UDP> UdpConnection;

} //fastapp

#endif	/* CONNECTION_H */

