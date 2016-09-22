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

#include <src/socketutils.h>

#include <ifaddrs.h>
#include <arpa/inet.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/ip.h> /* superset of previous */

#include <stdio.h>
#include <sys/time.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/time.h>
#include <netinet/tcp.h>
#include <sys/prctl.h>

#define fast_min(a, b) ((a) < (b)) ? (a) : (b)

namespace fastapp
{

namespace sockets
{

int getInterface(const std::string & interface, in_addr & addr)
{   struct in_addr inp;
    int ret = inet_aton(interface.c_str(), &inp);
    if (ret != 0 )
    {   addr = inp;
        return 0;
    }

    ifaddrs* begin = 0;
    getifaddrs(&begin);

    ifaddrs* current = 0;

    for(current = begin; current != 0; current = current->ifa_next)
    {   if(current->ifa_addr->sa_family == AF_INET
                && std::strncmp(current->ifa_name, interface.c_str(), interface.length()) == 0)
        {   addr =  reinterpret_cast<sockaddr_in*>(current->ifa_addr)->sin_addr;
            freeifaddrs(begin);
            return 0;
        }
    }


    if(begin != 0)
    {   freeifaddrs(begin);
    }

    return -1;
}

int getInterface(const std::string & interface, std::string & ip)
{
    in_addr addr;
    int err = getInterface(interface, addr);
    if (err < 0) return errno;
    ip.resize(INET6_ADDRSTRLEN);
    inet_ntop(AF_INET, &addr, (char *)ip.data(), INET6_ADDRSTRLEN);
    ip.resize( fast_min(strlen(ip.c_str()), INET6_ADDRSTRLEN) );

    return 0;
}

bool resolve(const std::string & remoteip, in_addr & addr)
{

    addrinfo hints, *servinfo_list = 0, *current = 0;
    int retv(0);

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC; // use AF_INET6 to force IPv6
    hints.ai_socktype = SOCK_STREAM;

    if ((retv = getaddrinfo(remoteip.c_str(), NULL, &hints, &servinfo_list)) != 0)
    {   //fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return false;
    }

// loop through all the results and connect to the first we can
    for (current = servinfo_list; current != NULL; current = current->ai_next)
    {   // get the pointer to the address itself,
        // different fields in IPv4 and IPv6:
        if (current->ai_family == AF_INET)   // IPv4
        {   struct sockaddr_in *ipv4 = (struct sockaddr_in *)current->ai_addr;
            addr = ipv4->sin_addr;
            freeaddrinfo(servinfo_list);
            return true;
        }

    }

    freeaddrinfo(servinfo_list); // free the linked list

    return false;
}

void sockaddr_to_ip_port(struct sockaddr_in *ipv4, std::string & ip, int & port) 
{
    ip.resize(INET6_ADDRSTRLEN);
    inet_ntop(AF_INET, &ipv4->sin_addr, (char *)ip.data(), INET6_ADDRSTRLEN);
    ip.resize( fast_min(strlen(ip.c_str()), INET6_ADDRSTRLEN) );

    port = ntohs(ipv4->sin_port);
}

bool resolve(const std::string & remoteip, std::string & ip)
{
    in_addr addr; 
    memset(&addr, 0, sizeof(addr));
    if ( resolve(remoteip,addr) )
    {
        ip.resize(INET6_ADDRSTRLEN);
        inet_ntop(AF_INET, &addr, (char *)ip.data(), INET6_ADDRSTRLEN);
        ip.resize( fast_min(strlen(ip.c_str()), INET6_ADDRSTRLEN) );
        return true;
    }
    return false;
}

// Returns current time in usecs from Jan 1, 1970
int64_t getTimeNow()
{   struct timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);

    return t.tv_sec * 1000000LL + t.tv_nsec/1000LL;
}

int64_t getTimeOfDay()
{   struct timeval t;
    gettimeofday( &t, NULL );

    return t.tv_sec * 1000000LL + t.tv_usec;
}


// Sets non-blocking mode on a socket
int setNonBlockMode(int sock)
{   int flags = fcntl( sock , F_GETFL, 0 );
    int err = fcntl( sock, F_SETFL, flags | O_NONBLOCK );
    if (err < 0)
    {   FMPRINT( KRED , "Failed to make UNblocking socket: " , err ,KNRM ,"\n");
    }

    flags = fcntl( sock , F_GETFL, 0 );
    if (  (flags & O_NONBLOCK) != 0 )
        FMPRINT( KGRN , "Non blocking socket" ,KNRM ,"\n");
    else
        FMPRINT( KRED , "Blocking socket" ,KNRM ,"\n");
    return err;
}

int setBlockMode(int sock)
{   int err = 0;
    int flags = fcntl( sock, F_GETFL, 0 );
    if (  (flags & O_NONBLOCK) != 0 )
    {   err = fcntl( sock, F_SETFL , flags ^ O_NONBLOCK );
        if (err < 0)
        {   FMPRINT( KRED , "Failed to make blocking socket: " , err ,KNRM ,"\n");
        }
    }

    flags = fcntl( sock , F_GETFL, 0 );

    if ( (flags & O_NONBLOCK) == 0 )
        FMPRINT( KGRN , "Blocking socket" ,KNRM ,"\n");
    else
        FMPRINT( KRED , "None Blocking socket" ,KNRM ,"\n");
    return err;
}

int setReceiveBuffer(int sock, size_t size)
{   int err = 0;
    if (setsockopt(sock, SOL_SOCKET, SO_RCVBUF, &size, sizeof(size)) == -1)
    {   // deal with failure, or ignore if you can live with the default sizeA
        err = errno;
        FMPRINT( KRED , "failed to set recv_socket buffer: " , size , "; error: " , err , KNRM , "\n");
    }

    socklen_t len = sizeof(size);
    socklen_t sz = 0;
    if (getsockopt(sock, SOL_SOCKET, SO_RCVBUF, &sz, &len) == 0)
    {   FMPRINT( KGRN , "recv_socket buffer: " , sz , KNRM , "\n");
    }

    return err;
}

int setSendBuffer(int sock, size_t size)
{   int err = 0;
    if (setsockopt(sock, SOL_SOCKET, SO_RCVBUF, &size, sizeof(size)) == -1)
    {   // deal with failure, or ignore if you can live with the default sizeA
        err = errno;
        FMPRINT( KRED , "failed to set send_socket buffer: " , size , "; error: " , err , KNRM , "\n");
    }

    socklen_t len = sizeof(size);
    socklen_t sz = 0;
    if (getsockopt(sock, SOL_SOCKET, SO_RCVBUF, &sz, &len) == 0)
    {   FMPRINT( KGRN , "send_socket buffer: " , sz , KNRM , "\n");
    }

    return err;
}

int setWindowSize(int sock, int window_size)
{
    int err = 0;
    if (setsockopt(sock, IPPROTO_TCP /*SOL_SOCKET*/, TCP_WINDOW_CLAMP, (char *)&window_size, sizeof(window_size))  < 0) {
        err = errno;
        FMPRINT( KRED , "Failed to set inbound tcp window_size :" , errno , KNRM ,"\n");;
    }

    int optval = 0;
    socklen_t optlen = sizeof(int);
    if (getsockopt(sock, IPPROTO_TCP /*SOL_SOCKET*/, TCP_WINDOW_CLAMP, &optval, &optlen) == 0)
    {
         if (window_size == optval)
         {    FMPRINT( KGRN , "inbound tcp window_size is : '" , optval , "'" , KNRM , "\n"); }
         else
         {    FMPRINT( KRED , "inbound tcp window_size is not set correctly reported '" , optval , "' !=  desired '" , window_size , "', errno: " , errno , KNRM , "\n"); }
    }
    else { FMPRINT( KRED , "Failed to set inbound tcp window_size, errno: " , errno , KNRM  , "\n"); }
    return err;
}

int getWindowSize(int sock)
{
    int optval = 0;
    socklen_t optlen = sizeof(int);
    if (getsockopt(sock, IPPROTO_TCP/* SOL_SOCKET */, TCP_WINDOW_CLAMP, &optval, &optlen) == 0) {
        return optval;
    }
    else return 0;
}


int setReadTimeOut(int sock, size_t val) //in seconds
{
    int err = 0;
    struct timeval tout;
    tout.tv_sec  = val;
    tout.tv_usec = 0;

    if (setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tout, sizeof(tout)) < 0)
    {
        FMPRINT( KRED , "Failed to set RCVTIMEOUT with errno= " , errno , "\n");
        err = errno; 
    }

    socklen_t ntrcv;
    if (getsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tout, &ntrcv) < 0)
    {
         FMPRINT( KRED , "Failed to get RCVTIMEOUT with errno= " , errno , KNRM , "\n");
    }
    else
    {
         FMPRINT( KGRN , "RCVTIMEOUT is " , tout.tv_sec , "." , std::setfill('0') , std::setw(6) , tout.tv_usec , " sec" , KNRM  , "\n");
    }

    return err;
}


void toggleNonBlockMode(int sock)
{   int flags = fcntl( sock, F_GETFL, 0 );
    if (  (flags & O_NONBLOCK) == 0 )
    {   setNonBlockMode(sock);
    }
    else
    {   setBlockMode(sock);
    }
}


int setKeepAlive(int sock)
{
    bool is_set = false;
    int err = 0;
    int keepalive = 1;
    if (setsockopt(sock, SOL_SOCKET, SO_KEEPALIVE, &keepalive , sizeof(keepalive)) < 0)
    {
        err = errno;
    }

    keepalive = 0;
    socklen_t optlen = sizeof(int);
    getsockopt(sock, SOL_SOCKET, SO_KEEPALIVE, &keepalive , &optlen);
    is_set = (keepalive == 1);

    if (is_set)
    {    FMPRINT( KGRN , "keep_allive is on" , KNRM  , "\n"); }
    else
    {    FMPRINT( KRED , "keep alive is off" , KNRM , "\n"); }
    return err;
}

// Sets reusable socket address flag
int setReuseAddr( int sock )
{   int yes = 1;
    if( setsockopt( sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) < 0 )
    {   int err = errno;
        FMPRINT( KRED , " Failed to set reuse option; " , err , KNRM , "\n");
        return err;
    }

    return 0;
}


// Sets NODELAY mode
int setNodelay( int sock)
{   int err = 0;
    int yes = 1;
    if( setsockopt( sock, IPPROTO_TCP, TCP_NODELAY, &yes, sizeof(yes) ) < 0 )
    {   err = errno;
        FMPRINT( KRED , "Error setting non-nodelay mode; " , err , KNRM , "\n");
    }

    socklen_t len = 0;
    if( getsockopt( sock, IPPROTO_TCP, TCP_NODELAY, &yes, &len ) < 0 )
    {   err = errno;
        FMPRINT( KRED , "Error getting non-nodelay mode; " , err , KNRM , "\n");
    }
    else
    {   FMPRINT( KGRN , "tcp_nodelay : "  , (yes > 0 ? "yes" : "no" ) , KNRM , "\n");
    }

    return err;
}


int open (::fastapp::sockets::TCP & tcp)
{
    int err = 0;
    if (tcp.fd != 0) close(tcp.fd);

    tcp.fd = socket( AF_INET, SOCK_STREAM, 0 );
    if (tcp.fd < 0)
    {    err = errno; }

    return err;
}

int open(::fastapp::sockets::UDP & udp)
{
    int err = 0;

    if (udp.fd != 0) close(udp.fd);

    udp.fd = socket( AF_INET, SOCK_DGRAM, 0 );
    if (udp.fd < 0)
    {err = errno;}

    return err;
}

//void create(auto & proto)
//{   open(ptoto);
//}


// Binds socket to localAddr
int bind( int sock,  in_addr localadr, int port )
{
    int err = 0;
    setReuseAddr( sock );

    struct sockaddr_in sa_loc;
    memset(&sa_loc, 0, sizeof(struct sockaddr_in));
    sa_loc.sin_family = AF_INET;
    sa_loc.sin_port = htons(port);
    sa_loc.sin_addr = localadr;
    int ret = ::bind(sock, (struct sockaddr *)&sa_loc, sizeof(struct sockaddr));
    if (ret != 0)
    {
        err = errno;
        char ip[INET6_ADDRSTRLEN];
        ip[INET6_ADDRSTRLEN - 1] = 0;
        inet_ntop(AF_INET, &localadr, ip, INET6_ADDRSTRLEN);        
        FMPRINT( KRED , "Error binding socket <" , ip , ":" ,  port ,   ">  err: " , err , KNRM , "\n");
    }
    else
    {
        Address local;
        if (0 == getLocalAddr( sock, local) )
        {
            FMPRINT( KGRN , "Socket " , sock , " is bound to local interface <" , local.ip , ":" ,  local.port ,   "> " , KNRM , "\n");
        }
        else
        {
            char ip[INET6_ADDRSTRLEN];
            ip[INET6_ADDRSTRLEN - 1] = 0;
            inet_ntop(AF_INET, &localadr, ip, INET6_ADDRSTRLEN);
            FMPRINT( KGRN , "Socket " , sock , " is bound to local interface <" , ip , ":" ,  port ,   ">  failed getting local address err: " , errno , KNRM , "\n");
        }
    }

    return err;

}

int bind( int sock, const char* ip, int port )
{   in_addr localaddr;
    localaddr.s_addr = inet_addr(ip);
    return bind(sock, localaddr, port);
}

int accept(int acceptfd, int & sockfd)
{   struct sockaddr_in cli_addr;

    socklen_t clilen = sizeof(cli_addr);

    /* Accept actual connection from the client */
    sockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);
    if (sockfd < 0)
    {   int err = errno;
        FMPRINT( KRED , "Error accepting connection: " , err , KNRM , "\n");
        return err;
    }

    return 0;
}

// Starts listening on socket
int listen( int sock)
{   return ::listen( sock, 10 );
}

int connect( int sock, struct in_addr addr, int port )
{   if (is_open(sock) ) close(sock);

    struct sockaddr_in sa_dst;
    // Remote
    memset(&sa_dst, 0, sizeof(struct sockaddr_in) );
    sa_dst.sin_family      = AF_INET;
    sa_dst.sin_port        = htons(port);
    sa_dst.sin_addr        = addr;

    int ret = connect(sock, (struct sockaddr *)&sa_dst, sizeof(struct sockaddr));

    if (ret < 0)
    {   int err = errno;
        FMPRINT( KRED , "Error connecting: " , err , KNRM , "\n");
        return err;
    }

    return 0;
}

// Connects to remote ip:port. Assumes async socket
int connect( int sock, const char * ip, int port )
{   struct in_addr inp;
    inp.s_addr = inet_addr(ip);


    return connect( sock, inp, port);
}

int connect( int sock, Endpoint & endpoint)
{
    int ret = connect(sock, (struct sockaddr *)&(endpoint.address()), sizeof(struct sockaddr));

    if (ret < 0)
    {   int err = errno;
        FMPRINT( KRED , "Error connecting: " , err , KNRM , "\n");
        return err;
    }

    return 0;
}

// Fills sockAddr with socket address obtained from socket descriptor
inline int getTmplAddr( int sock, Address & remote, auto getAddr)
{   socklen_t len;
    struct sockaddr_storage addr;
    int err = 0;
    len = sizeof addr;
    if (getAddr(sock, (struct sockaddr*)&addr, &len) == 0)
    {

        if (addr.ss_family == AF_INET)
        {   struct sockaddr_in *s = (struct sockaddr_in *)&addr;
            remote.port = ntohs(s->sin_port);
            inet_ntop(AF_INET, &s->sin_addr, remote.ip, INET6_ADDRSTRLEN);

        }
        else     // AF_INET6
        {   struct sockaddr_in6 *s = (struct sockaddr_in6 *)&addr;
            remote.port = ntohs(s->sin6_port);
            inet_ntop(AF_INET6, &s->sin6_addr, remote.ip, INET6_ADDRSTRLEN);

        }
    }
    else
    {   err = errno;
        FMPRINT( KRED ,  "Error obtaining socket address from descriptor: " ,  err , KNRM , "\n");
    }

    return err;
}


// int getsockname(int sockfd, struct sockaddr *addr, socklen_t *addrlen);

int getLocalAddr( int sock, Address & local)
{
    return getTmplAddr(sock, local, ::getsockname);
}

int getSockAddr( int sock, Address & remote)
{

    return getTmplAddr(sock, remote, ::getpeername);
}

// Fills sockAddr with socket address obtained from socket descriptor
int getRemoteAddr( int sock, Address & remote)
{
    return getTmplAddr(sock, remote, ::getpeername);
}

// Resolving interface name to IP and fills sockAddr object.
bool resolveInterfaceAddr( const char* intfName, Address & address )
{   std::string interface(intfName);
    in_addr addr;
    if ( getInterface(interface, addr) == 0)
    {   inet_ntop(AF_INET, &addr, address.ip, INET6_ADDRSTRLEN);
        FMPRINT( KRED , "Intf <" , intfName , "> resolved into <" , address.ip , ">" , KNRM , "\n");
        return true;
    }
    return false;
}

// Receive on async TCP socket
int recv( int sock, void* buf, size_t len, bool log)
{   int ec = 0;
    int ret =  ::recv(sock, buf, len, 0);
    if (ret < 0) ec = errno;
    if (log && ret > 0 && !(ec == EAGAIN   || ec == EWOULDBLOCK) )
    {   Address local, remote;
        getLocalAddr(sock, local);
        getRemoteAddr(sock, remote);

        FMPRINT( "In packet: (" , local.ip , ":" , local.port , "->" , remote.ip , ":" , remote.port , ") '",::fastapp::Hex(buf, ret), "' ; ec=" , ec , "; len=" , ret , "\n");
    }

    if( ec != 0 )
    {   if( ec == EAGAIN   || ec == EWOULDBLOCK  )
        {   return -1;
        }
        else
        {   FMPRINT( KRED , "Error receiving data from socket  ; ec=" , ec , "; len=" , ret , KNRM, "\n");
            throw SocketError( "Error receiving data from socket", ec );
        }
    }
    else if( ret == 0 )
    {   FMPRINT( KRED , "' ; ec=" , ec , "; len=" , ret , "\n", "Connection reset by peer" , KNRM , "\n");
    }

    return ret;
}

// Async receive of UDP datagram
int recvfrom(
    int   socket
    , char* buf
    , size_t len
    , Endpoint & remote
    , bool log)
{   int ec (0);
    socklen_t addrlen = sizeof(remote.addr);
    int ret = ::recvfrom(socket, buf, len, 0, (struct sockaddr *)&(remote.address()), &addrlen);
    ec = errno;
    if (log && ret > 0 && !(ec == EAGAIN  || ec == EWOULDBLOCK ))
    {   FMPRINT( "Incoming packet:'", ::fastapp::Hex(buf, ret), "' ; ec=" , ec , "; len=" , ret , "\n");
    }

    if( ret < 0 && ec != 0)
    {   // if( errno == EAGAIN || errno == EWOULDBLOCK )
        if (ec == EAGAIN  || ec == EWOULDBLOCK)
        {   return -1;
        }
        else
        {   FMPRINT( KRED , "Error receiving data from UDP socket  ; ec=" , ec , "; len=" , ret , KNRM, "\n");
            throw SocketError( "Error receiving data from UDP socket", ec );
        }
    }
    else if( ret == 0 )
    {   // Acceptable. Means zero-size UDP packet is received.
        return -1;
    }

    return ret;
}

// Sends data over async TCP socket. Returns -1 if socket full, number of
// bytes sent otherwise.
ssize_t send( int socket, void * buf, size_t len , bool log)
{   //const char * buf = reinterpret_cast<const char *>(bptr);
    int ec = 0;


    ssize_t ret = (ssize_t)::send(socket, buf, len, 0);
    if (ec < 0)
    {   ec = errno;
    }

//        int64_t tod = getTimeOfDay();
//        FMPRINT( tod , " : Outgoing packet:'";
//        ::utils::hex(FMPRINT( (const char *)buf, ret);
//        FMPRINT( "' ; ec=" , ec , "; len=" , ret , "\n");
    if (ret <= 0 || log )
    {   int64_t tod = getTimeOfDay();

        FMPRINT( (ret <= 0 ? KRED: KNRM) , tod , ::fastapp::Micro(tod) , " : Outgoing :'", ::fastapp::Hex(buf, ret),"' ; ec=" , ec , "; len=" , ret , KNRM , "\n");
    }

    if (ec != 0)
    {   // if( errno == EAGAIN || errno == EWOULDBLOCK )

        if (ec == EAGAIN || ec == EWOULDBLOCK)
        {   if (log)
            {   int64_t tod = getTimeOfDay();
                FMPRINT( KRED , ::fastapp::Micro(tod), "Error sending data. " , (ec == EAGAIN ?  "resource unavailable! " : "ewouldblock. ") , KNRM , "\n");
            }

            return -1;
        }
        else
        {   if (log)
            {   int64_t tod = getTimeOfDay();
                FMPRINT( KRED , ::fastapp::Micro(tod) , "Error sending data" , KNRM , "\n");
            }

            throw SocketError( "Error sending data", ec );
        }
    }
    else
    {   if (ret == 0) return -1;
        return ret;
    }
}

// Sends datagram to addr destination.
ssize_t sendto( int sock, const char* buf, size_t len, const Endpoint & remote, bool log )
{   int ec(0);
    socklen_t addrlen = sizeof(remote.addr);
    ssize_t ret = ::sendto( sock, buf, len, 0, (const struct sockaddr *)(&(remote.address())), addrlen );
    if (ret < 0) ec = errno;

    if (ret <= 0 || log)
    {   
        FMPRINT( (ret <= 0 ? KRED: KNRM) , "Outgoing packet:'", ::fastapp::Hex(buf, ret), "' ; ec=" , ec , "; len=" , len , KNRM, "\n");
    }

    if( ec  != 0)
    {   if (ec == EAGAIN || ec == EWOULDBLOCK)
        {   return -1;
        }
        else
        {   throw SocketError( "Error sending datagram", ec );
        }
    }
    else
    {   if (ret == 0) return -1;
        return ret;
    }
}

// Shuts down socket. Note that data may still arrive after this call.
// Therefore, correct shutdown sequence is shutdown, followed by receive
// (untile recv returns zero), followed by close.
 int shutdown( int socket )
{   int ec = 0;
    if (::shutdown ( socket, SHUT_RDWR ) < 0)
    {   ec = errno;
    }
    return ec;
}

// Closed socket. Note shutdown should be called prior to close. 'recv'
// should be invoked prior to close until it returns zero (no more data).
int close( int socket )
{   int ec = 0;
    int ret = ::close( socket );
    if (ret < 0)
    {   ec = errno;
    }
    return ec;
}

int is_open(int sock)
{   int error = 0;
    socklen_t len = sizeof (error);
    int retval = getsockopt (sock, SOL_SOCKET, SO_ERROR, &error, &len);
    if (retval != 0)
    {   return -1;
    }
    if (error != 0) FMPRINT( KRED , "is_open error=" , error , KNRM , "\n");
    //if (error != 0) => error
    return error;
}

void closeSocket(int sock)
{
    if ( is_open(sock) == 0)
    {   Address local, remote;

        getLocalAddr (sock, local);
        getRemoteAddr(sock, remote);

        int  ec = shutdown(sock);

        if (ec != 0)  
        {   if (ec != ENOTCONN && ec != EINVAL) 
            {   char discard[64];
                int ret = 0;
                int n = 0;

                while( (ret = recv(sock, discard, sizeof(discard), 0) ) > 0)
                {   if (n > 20) break;
                    n++;
                };

                if ( ret <= 0)
                {   FMPRINT( "n=" , n , ", socket shutdown" , "\n");
                }
                else
                {   FMPRINT( KRED , "failed to flush socket" , "\n");
                }
            }
            else
            {   FMPRINT( KRED , "socket shutdown failed" , KNRM , "\n");
            }
        }

        ec = ::close(sock);
        if (ec) // finally call close()
        {   FMPRINT( KRED , "failed to close socket, errno=" , errno , KNRM , "\n");
            FMPRINT( KRED , "...additional info, failed to close socket " , local.ip , ":" , local.port\
            , "->"   , remote.ip ,":" , remote.port , " ec=" , ec  , KNRM, "\n");
        }
        else
        {   FMPRINT( KGRN , "socket " , local.ip , ":" , local.port
                      , "->"   , remote.ip , ":" , remote.port ," closed" KNRM , "\n");
        }
    }
}

void nameThread (const char * name)
{
    prctl(PR_SET_NAME, name, 0,0,0);
}

void initOnloadStack( const char* stackName )
{
#ifdef EBL_ONLOAD
    onload_set_stackname( ONLOAD_THIS_THREAD, ONLOAD_SCOPE_PROCESS, stackName );

    if( onload_thread_set_spin( ONLOAD_SPIN_ALL, 1 ) != 0 )
    {   FMPRINT( KRED , "Error enabling onload spinning in client thread" , KNRM , "\n");
    }
#endif
}

void spinOnloadThread()
{
#ifdef EBL_ONLOAD
    if( onload_thread_set_spin( ONLOAD_SPIN_TCP_RECV, 1 ) != 0 )
    {   FMPRINT( KRED , "Error enabling onload tcp rcv spinning in client thread" , KNRM , "\n");
    }
    if( onload_thread_set_spin( ONLOAD_SPIN_TCP_SEND, 1 ) != 0 )
    {   FMPRINT( KRED , "Error enabling onload tcp send spinning in client thread" , KNRM , "\n");
    }
    if( onload_thread_set_spin( ONLOAD_SPIN_UDP_RECV, 1 ) != 0 )
    {   FMPRINT( KRED , "Error enabling onload udp rcv spinning in client thread" , KNRM , "\n");
    }
    if( onload_thread_set_spin( ONLOAD_SPIN_UDP_SEND, 1 ) != 0 )
    {   FMPRINT( KRED , "Error enabling onload udp send spinning in client thread" , KNRM , "\n");
    }

#endif
}


// Adds mcast membeship (joining mcast group)
int addMcastMembership( int sock, const Mcast & mcast )
{   int err = 0;
    struct ip_mreq mreq;
    mreq.imr_multiaddr.s_addr = mcast.remote.address().sin_addr.s_addr;
    mreq.imr_interface.s_addr = mcast.local.address().sin_addr.s_addr;

    if( setsockopt(sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) < 0 )
    {   err = errno;
        FMPRINT( KRED , "Error joining mcast group : "  , err , KNRM , "\n");
    }

    return err;
}

// Drops mcast membership
int dropMcastMembership( int sock, const Mcast & mcast)
{   struct ip_mreq mreq;
    mreq.imr_multiaddr.s_addr = mcast.remote.address().sin_addr.s_addr;
    mreq.imr_interface.s_addr = mcast.local.address().sin_addr.s_addr;
    int err = 0;

    if( setsockopt(sock, IPPROTO_IP, IP_DROP_MEMBERSHIP, &mreq, sizeof(mreq)) < 0 )
    {   err = errno;
        FMPRINT( KRED , "Error dropping mcast group subscription" , err , KNRM , "\n");
    }

    return err;
}

// Sets mcast socket time to live, 2 is minimum required for internal mcast network
int setMcastTTL( int sock, int value)
{   unsigned char ttl = value;
    int err = 0;
    if ( setsockopt( sock, IPPROTO_IP, IP_MULTICAST_TTL, &ttl, sizeof(ttl) ) < 0 )
    {   err = errno;
        FMPRINT( KRED , "Error in setsockop IP_MULTICAST_TTL" , KNRM , "\n");
    }
    return err;
}

// Sets interface for outgoing mcast socket
int setOutgoingMcastIntf( int sock, const char* local_ip )
{   int err = 0;
    struct in_addr        localInterface;

    localInterface.s_addr = inet_addr(local_ip);

    if (setsockopt(sock, IPPROTO_IP, IP_MULTICAST_IF, (char *)&localInterface, sizeof(localInterface)) < 0)
    {   err = errno;
        FMPRINT( KRED , "Error setting mcast outgoing interface : " , err , KNRM , "\n");
    }

    return err;
}

// Sets interface for outgoing mcast socket
void setOutgoingMcastIntf( int sock, const Endpoint & local )
{   int err = 0;

    struct in_addr        localInterface;

    localInterface = local.address().sin_addr;

    if (setsockopt(sock, IPPROTO_IP, IP_MULTICAST_IF, (char *)&localInterface, sizeof(localInterface)) < 0)
    {   err = errno;
        FMPRINT( KRED , "Error setting mcast outgoing interface : " , err , KNRM , "\n");
    }
}


} // End of sockets
} // End of fastapps namespace


