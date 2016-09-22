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

#ifndef APP_H
#define	APP_H

#include <src/connection.h>
#include <src/smtp.h>

   
namespace fastapp
{

    

void formatException();


class App
{
public:
    enum eAlgo { SNew, SRpl };

    void parseAlgo(const char * s)
    {
        if (strcmp(s,"new") == 0)
        {
            _algo = SNew;
        }
        else if (strcmp(s,"rpl") == 0)
        {
            _algo = SRpl;
        }
    }

    static void usage(int argc, char * argv[]);
    void parseCmd(int argc, char * argv[]);
    
    App();
    ~App ();
    bool connect();
    bool setup();
    void run ();
    void run_once (size_t & good_conns);
    void stop_mktdata();
    void run_once ()
    {
        size_t  good_conns = 0;
        run_once(good_conns);
    }
    
    void addConnection(TcpConnection && connection);
    void addConnection(UdpConnection && connection);
    
    TcpConnection &  newTcpConnection();
    UdpConnection &  newUdpConnection();

    //TcpConnection &  newConnection();
    
    //TcpConnection &  lastConnection();
    
    TcpConnection &  lastTcpConnection();
    UdpConnection &  lastUdpConnection();
    
    //TcpConnection &  getConnection(size_t index);
    
    TcpConnection &  getTcpConnection(size_t index);
    UdpConnection &  getUdpConnection(size_t index);
    
    
    size_t numOfConnections();
    
    size_t numOfTcpConnections();
    size_t numOfUdpConnections();
    
    int64_t getDelta() const;
    int64_t getPrintDelta() const;
    
//    // email
    int sendMail(const std::string & source, const std::string & subject, const std::string & text, const std::string & remote_ip, int remote_port);
    
    void terminate()
    {
        _time2exit = true;
    }

    eAlgo getAlgo()
    {
        return _algo;
    }

    int64_t getStuckSec()
    {
        return _stuck_sec;
    }

    bool getBlockingMode()
    {    return _block_mode;    }

    int64_t getSleep() 
    {
        return _sleep;
    }

    const std::string & getName() { return _app_name; }
    void setName(const std::string & name) {_app_name = name; }

    TcpConnection & newTcpConnection(TcpConnection * c)
    {
        _tcp_connections.push_back(c);
        return lastTcpConnection();
    }


    UdpConnection & newUdpConnection(UdpConnection * c)
    {
        _udp_connections.push_back(c);
        return lastUdpConnection();
    }

protected:    

    std::vector<TcpConnection *> _tcp_connections;
    std::vector<UdpConnection *> _udp_connections;
    
    smtp::Mail              _mail;
    std::string             _env;
    int64_t                 _sleep;
    int64_t                 _delta;
    int64_t                 _print_delta;
    bool                    _block_mode;
    volatile bool           _time2exit;
    eAlgo                   _algo;
    int64_t                 _stuck_sec;
    std::string             _app_name;
};

    
} // fastapp


#endif	/* APP_H */

