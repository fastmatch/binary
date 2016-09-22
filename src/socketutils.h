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

#ifndef __BIN_UTILS_SOCKETS_H__
#define __BIN_UTILS_SOCKETS_H__

//
#include <string>
#include <cstring>


// Onload Includes
#ifdef EBL_ONLOAD
#include <onload/extensions.h>
#include <onload/extensions_zc.h>
#endif

#include <src/defs.h>

#include <ifaddrs.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#include <system_error>
#include <src/utils.h>

namespace fastapp
{
namespace  sockets
{

class SocketDisconnect : public std::runtime_error
{
    int  err;
public:
    explicit SocketDisconnect( const std::string& msg ,  const int & ec)
        :   std::runtime_error( msg )
        ,  err(ec)
    {}
    int error_code() const { return err; }
};

class SocketError : public std::runtime_error
{
    int  err;
public:
    explicit SocketError( const std::string& msg,  const int & ec)
        :   std::runtime_error( msg )
        , err(ec)
    {}
        
    int error_code() const { return err; }
};

struct TCP
{   int fd;
    TCP() : fd(0) {}
};

struct UDP
{   int fd;
    UDP() : fd(0) {}
};

struct Address
{   int port;
    char ip[INET6_ADDRSTRLEN];
    Address() : port(-1)
    {   ip[0] = 0;
    }
};

void sockaddr_to_ip_port(struct sockaddr_in *ipv4, std::string & ip, int & port);

struct Endpoint
{
    struct sockaddr_in addr;

    Endpoint()
    {   memset(&addr, 0, sizeof(struct sockaddr_in));
    }

    Endpoint(const char * ip, int port)
    {   memset(&addr, 0, sizeof(struct sockaddr_in));
        addr.sin_family = AF_INET;
        addr.sin_port = htons(port);
        addr.sin_addr.s_addr = inet_addr(ip);;
    }

    Endpoint(struct in_addr ip, int port)
    {   memset(&addr, 0, sizeof(struct sockaddr_in));
        addr.sin_family = AF_INET;
        addr.sin_port = htons(port);
        addr.sin_addr = ip;
    }

    const auto & address() const
    {   return addr;
    }

    auto & address()
    {   return addr;
    }

    std::string to_string()
    {
        std::string out;
        int port = -1;
        sockaddr_to_ip_port(&addr, out, port);

        out += ":";
        char buf[32];
        sprintf(buf, "%d", port); 
        out += buf;
        return out;
    }
};

struct Mcast
{
    Endpoint local;
    Endpoint remote;
};


int getInterface(const std::string & interface, in_addr & addr);
int getInterface(const std::string & interface, std::string & ip);

bool resolve(const std::string & remoteip, in_addr & addr);
bool resolve(const std::string & remoteip, std::string & addr);

// Sets non-blocking mode on a socket
int setNonBlockMode(int sock);

int setBlockMode(int sock);

int setReceiveBuffer(int sock, size_t size);

int setSendBuffer(int sock, size_t size);

int setWindowSize(int sock, int window_size);

int getWindowSize(int sock);

int setReadTimeOut(int sock, size_t val); //in seconds
int setKeepAlive(int sock);

void toggleNonBlockMode(int sock);

// Sets reusable socket address flag
int setReuseAddr( int sock );

// Sets NODELAY mode
int setNodelay( int sock);


int open (TCP & tcp);

int open (UDP & udp);

inline int create(auto & proto)
{   
    return open(proto);
}


// Binds socket to ip:port

// Binds socket to localAddr
int bind( int sock,  in_addr localadr, int port );

int bind( int sock, const char* ip, int port );

int accept(int acceptfd, int & sockfd);

// Starts listening on socket
int listen( int sock);

int connect( int sock, struct in_addr inp, int port );

int connect( int sock, Endpoint & endpoint );

// Connects to remote ip:port. Assumes async socket
int connect( int sock, const char * ip, int port );

// Fills sockAddr with socket address obtained from socket descriptor


int getLocalAddr( int sock, Address & local);

int getSockAddr( int sock, Address & remote);
int getRemoteAddr( int sock, Address & remote);
// Resolving interface name to IP and fills sockAddr object.
bool resolveInterfaceAddr( const char* intfName, Address & address );

// Receive on async TCP socket
int recv( int sock, void* buf, size_t len, bool log);

// Async receive of UDP datagram
int recvfrom( int   socket , char* buf , size_t len , Endpoint & remote , bool log);

// Sends data over async TCP socket. Returns -1 if socket full, number of
// bytes sent otherwise.
ssize_t send( int socket, void * buf, size_t len , bool log);

// Sends datagram to addr destination.
ssize_t sendto( int sock, const char* buf, size_t len, const Endpoint & remote, bool log );

// Shuts down socket. Note that data may still arrive after this call.
// Therefore, correct shutdown sequence is shutdown, followed by receive
// (untile recv returns zero), followed by close.

int shutdown( int socket );

// Closed socket. Note shutdown should be called prior to close. 'recv'
// should be invoked prior to close until it returns zero (no more data).
int close( int socket );

int is_open(int sock);

void closeSocket(int sock);

void initOnloadStack( const char* stackName );
void spinOnloadThread();
void nameThread(const char* name);

// Adds mcast membeship (joining mcast group)
int addMcastMembership( int sock, const Mcast & mcast );

// Drops mcast membership
int dropMcastMembership( int sock, const Mcast & mcast);

// Sets mcast socket time to live, 2 is minimum required for internal mcast network
int setMcastTTL( int sock, int value = 2 );

// Sets interface for outgoing mcast socket
int setOutgoingMcastIntf( int sock, const char* local_ip );

// Sets interface for outgoing mcast socket
void setOutgoingMcastIntf( int sock, const Endpoint & local );


} // End of sockets
} // End of fastapps namespace

#endif // End of FXLocus_Transport_SocketUtils_h

