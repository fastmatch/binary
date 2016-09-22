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

#include <src/app.h>
#include <src/smtp.h>
#include <src/helpers.h>
  
namespace fastapp
{

int64_t App::getDelta() const 
{
    return _delta;
}

int64_t App::getPrintDelta() const
{
    return _print_delta;
}

size_t App::numOfTcpConnections()
{
    return _tcp_connections.size();
}

size_t App::numOfUdpConnections()
{
    return _udp_connections.size();
}

App::~App ()
{
    for (auto * c : _tcp_connections) delete c;
    for (auto * c : _udp_connections) delete c;
}


App::App() 
: _sleep(0)
, _delta(60*Connection::onesec)
, _print_delta(60*Connection::onesec)
, _block_mode(false)
, _time2exit (false)
, _stuck_sec(120)
{   
    _tcp_connections.reserve(2);
    _udp_connections.reserve(2);
    _mail.setFrom("ItchOuch@domain");
    _mail.setTo("user@domain");    
}

void formatException()
{
    try
    {
        throw;
    }
    catch(const ::fastapp::sockets::SocketError & ex)
    {
        FMPRINT("Exception SocketError: '", ex.what() , "' ; ec=" , ex.error_code() , "\n");
    }
    catch(const ::fastapp::sockets::SocketDisconnect &dex)
    {
        FMPRINT("Exception SocketDisconnect: '",  dex.what(), KNRM, "' ; ec=", dex.error_code(), "\n");
    }
    catch(const std::runtime_error &err)
    {
        FMPRINT("Exception runtime_error: '", err.what() ,"'" , "\n");
    }
    catch(const std::exception &e)
    {
        FMPRINT("Exception std::exception: '" , e.what() , "'", "\n" );
    }    
    catch(...) 
    {
        FMPRINT("Unknown exception", "\n");
    }                
}

bool App::connect()
{
    return setup();
}

bool App::setup()
{
    if (!getName().empty())
    {
         std::string nm(getName());
         if (nm.size() > 8)
         {
             nm.resize(8);
         }

         sockets::initOnloadStack(nm.c_str());
    }
 
    for( TcpConnection * p_conn : _tcp_connections)
    {
        if (p_conn == 0) continue;
        TcpConnection & c = *p_conn;
        c.setBlockingMode(_block_mode);
        c.setDelay(_sleep);
        //c.activateBlockingMode(); blocking budefault
        
        c.setMailSender
        (
           [&](const std::string & source, const std::string & subject, const std::string & text, const std::string & remote_ip, int remote_port)
           {
               return sendMail(source, subject, text, remote_ip, remote_port);
           }
        );
    }

    for( UdpConnection * p_conn : _udp_connections)
    {
        if (p_conn == 0) continue;
        UdpConnection &c = *p_conn;
 
        c.setBlockingMode(_block_mode);
        c.setDelay(_sleep);
        //c.activateBlockingMode();
        
        c.setMailSender
        (
           [&](const std::string & source, const std::string & subject, const std::string & text, const std::string & remote_ip, int remote_port)
           {
               return sendMail(source, subject, text, remote_ip, remote_port);
           }
        );
    }
    
    
    for( TcpConnection * p_conn : _tcp_connections)
    {
        if (p_conn == 0) continue;
        TcpConnection & c = *p_conn;
 
        if (!c.isInitialized())
        {
            FMPRINT( KRED , "TCP: connection=" , c.getProtocolStr() , "//" , c.getRemoteHost() , ":" , c.getRemotePort()  , " is not initialized with processor callback!", KNRM , "\n");
            return false;
        }
        
        if (!c.setup() ) return false;
    }

    for( UdpConnection * p_conn : _udp_connections)
    {
        if (p_conn == 0) continue;
        UdpConnection & c = *p_conn;

        if (!c.isInitialized())
        {
            FMPRINT( KRED , "UDP: dest=" , c.getProtocolStr() , "//" , c.getRemoteHost() , ":" , c.getRemotePort()  , " is not initialized with processor callback!", KNRM , "\n");
            return false;
        }
        
        if (!c.setup() ) return false;
    }
    
    for( TcpConnection * p_conn : _tcp_connections)
    {
        if (p_conn == 0) continue;
        TcpConnection & c = *p_conn;


        if (!c.connect() ) 
        {
            FMPRINT( KRED , "TCP: Failed to connect to " , c.getProtocolStr() , "//" , c.getRemoteHost() , ":" , c.getRemotePort()  , KNRM , "\n");
            return false;
        }
    }

    for( UdpConnection * p_conn : _udp_connections)
    {
        if (p_conn == 0) continue;
        UdpConnection & c = *p_conn;

        if (!c.connect() ) 
        {
            FMPRINT( KRED , "UDP: Failed to execute connect call. Dest:  " , c.getProtocolStr() , "//" , c.getRemoteHost() , ":" , c.getRemotePort()  , KNRM , "\n")  ;
            return false;
        }
    }
    
    return true;
}

void App::run_once (size_t & good_conns)
{
    for( TcpConnection * p_conn : _tcp_connections)
    {
        if (p_conn == 0) continue;
        TcpConnection & c = *p_conn;

        if (c.good() )
        {
            good_conns ++;
            try
            {
                if (!c.run_once())    
                {
                    c.disconnect();
                    c.disable();  
                }
            }
            catch(...)
            {
                formatException();
                c.disconnect();
                c.disable();
            }
        }

        if (_sleep > 0)
        {
            std::this_thread::sleep_for( std::chrono::milliseconds(_sleep));
        }
    }
    
    for( UdpConnection * p_conn : _udp_connections)
    {

        if (p_conn == 0) continue;
        UdpConnection & c = *p_conn;

        if (c.good() )
        {
            good_conns ++;
            try
            {
                if (!c.run_once())    
                {
                    c.disable();  
                    c.disconnect();
                }
            }
            catch(...)
            {
                formatException();
                c.disconnect();
                c.disable();
            }
        }

        if (_sleep > 0)
        {
            std::this_thread::sleep_for( std::chrono::milliseconds(_sleep));
        }
    }    
}
void App::stop_mktdata()
{
    for( TcpConnection * p_conn : _tcp_connections)
    {
        if (p_conn == 0) continue;
        TcpConnection & c = *p_conn;


        if (c.good() )
        {
            try
            {
                if (c.getProtocol() == ConnectionBase::Protocol::ITCH)
                {
                    c.disconnect();
                    c.disable();
                }
            }
            catch(...)
            {
                formatException();
                c.disable();
            }
        }
    }

    for( UdpConnection * p_conn : _udp_connections)
    {
        if (p_conn == 0) continue;
        UdpConnection & c = *p_conn;

        if (c.good() )
        {
            try
            {
                if (c.getProtocol() == ConnectionBase::Protocol::ITCH)
                {
                    c.disconnect();
                    c.disable();
                }
            }
            catch(...)
            {
                formatException();
                c.disable();
            }
        }
    }
}

void App::run ()
{
    if (!getName().empty())
    {
         std::string nm(getName());
         if (nm.size() > 15)
         {
             nm.resize(15);
         }

         sockets::nameThread(nm.c_str());         
    }

    sockets::spinOnloadThread();

    FMPRINT("artificial sleep in millis: ", (int)_sleep, "\n" );

    size_t good_conns = _tcp_connections.size() + _udp_connections.size();
    while(good_conns > 0 && !_time2exit)
    {
        good_conns = 0;
        run_once(good_conns);
    }

    stop_mktdata();
    FMPRINT( KGRN , "Shutting down... wait for 30 sec" , KNRM , "\n") ;

    int64_t tod = ::fastapp::timeOfDay();
    int32_t counter = 0;
    int64_t now = tod;
    while (good_conns > 0 && tod + 30 * Connection::onesec > now)
    {
        good_conns = 0;
        run_once(good_conns);

        now = timeOfDay();
        int64_t current_counter = (now - tod)/Connection::onesec;
        if ( current_counter != counter)
        {
            FMPRINT(".");
            counter = current_counter;
        }
    }
    FMPRINT( "\n" );
}

int App::sendMail(const std::string & source, const std::string & subject, const std::string & text, const std::string & remote_ip, int remote_port)
{
    if (_mail.isInitialized())
    {
        int ret = _mail.sendAlert(source, subject, text, _env + "(" + remote_ip + ":" + std::to_string(remote_port) + ") ");
        if (ret < 0)
        {
            FMPRINT( KRED , "SMTP issue: " , _mail.error(ret) ,KNRM , "\n");
        }

        return ret;
    }

    return -6;
}
    


void App::usage(int argc, char * argv[])
{
    FMPRINT( "Usage: " , "\n" , argv[0] , " <Options>" , "\n" ,\
    "Options:" , "\n"  ,\
    "\t-help - this prompt","\n" ,\
    "\t-new\n\t-new=tcp\n\t-tcp  - creates tcp connection", "\n"  ,\
    "\t-new=udp\n\t-udp  - creates udp connection", "\n"  , \
    "\t-bind localip:port", "\n"  ,\
    "\t-rbuf READ_BUFFER_SIZE in bytes","\n"  ,\
    "\t-wbuf WRITE_BUFFER_SIZE in bytes", "\n"  ,\
    "\t-dest <host>:<port> " , "\n"  ,\
    "\t-sym      SYMBOL   ," , "\n"  ,\
    "\t-depth    LEVELS   - 0-means all levels," , "\n"  ,\
    "\t-sub_type FLAG     - 1-aggregated, 2-not aggregated,", "\n"  ,\
    "\t-print heartbeat=on,trace=on,book=on[,book=off],lowlevel=on - enable disable loggers combinations,", "\n"  ,\
    "\t-updateType book,book=on,book=off,trade,midpoint,onclose ," , "\n"  ,\
    "\t-blocking          - use blocking IO sockets ,", "\n"  ,\
    "\t-non_blocking      - use non blocking sockets ,", "\n"  ,"\n"  ,\
\
    "\tCREDENTIALS","\n"  ,\
    "\t-user     USERNAME ,", "\n"  ,\
    "\t-password PASSWORD ,", "\n"  ,"\n"  ,\
    "\tEMAIL", "\n"  ,\
    "\t-mailhost SERVER:PORT- mail server,", "\n"  ,\
    "\t-from SENDERS_EMAIL,", "\n"  ,\
    "\t-to   DESTINATION_EMAIL,", "\n"  ,"\n"  ,\
    "\tEXTRA", "\n"  ,\
    "\t-session  SESIONID ," , "\n"  ,\
    "\t-seq_num  SEQUENCE_NUMBER," , "\n"  ,\
    "\t-sleep    millis, delay message consumption on each loop iteration by xxx milliseconds, can be used for testing slow client behavior" , "\n"  , \
    "\t-delta    millis, time in millis to e.g. submit orders" , "\n"  , \
    "\t-stuck_sec   sec, quote considered stuck if it is on the book for longer than time in sec provided", "\n" ,\
    "\t-print_delta    millis, time in millis to e.g. print statistics" , "\n"  , \
    "\t-env      NAME     - envornment name," , "\n"  ,\
    "\t-listReqOn         - enable list request," , "\n"  , "\n"  , \
    "\t-listReqOff       -  disable list request," , "\n"  , "\n"  , \
    "\t-algo new|rpl     - select algo strategy send new/rpl," , "\n"  , "\n"  ,\
    "Examples: " , "\n"  ,\
    "\ttaskset -c 14-15 ./", argv[0] , "  -bind interface_ip_address e.g. 192.168.1.100  -dest server:port  -sym EUR/USD -sub_type 2  -depth 0 -print book=on  -updateType trade,midpoint,book -non_blocking -user FM -password FM &> out.txt \n" ,\
    "\t In order to run with onload as below, please make sure to build examples with onload=true e.g. make -f SampleXYZ.mak onload=true", "\n"  ,\
    "\t Also you need to export content of onload_profile.sh, onload has quite a few settings and you may want to consult onload docs" , "\n"  ,\
    "\t modify makefile to point to onload headers and libs" , "\n"  ,\
    "\t(. ./onload_profile.sh ; taskset -c 14-15 onload --profile=latency ./" , argv[0] , "  -bind interface_name e.g. eth1 -dest server:port -sym EUR/USD -sub_type 2  -depth 0 -print book=on  -updateType trade,midpoint,book -non_blocking )" , "\n"  ,\
    "\ttaskset -c 14-15 onload --profile=latency ./" , argv[0] , "  -bind interface_name -dest server:port -sym EUR/USD -sub_type 2  -depth 0 -print book=on  -updateType trade,midpoint,book -blocking -mailhost mailhost:port" , "\n" );
}

void App::parseCmd(int argc, char * argv[])
{
    Connection::Protocol protocol = Connection::Protocol::ITCH;
    ConnectionBase * c = &newTcpConnection();
    c->setProtocol(protocol);
    for (int i = 1; i < argc; i++)
    {
        std::string key = argv[i];
        std::vector<std::string>  params;
        if (key == "-help" || key=="--help" || key=="-h")
        {
            App::usage(argc, argv);
            exit(0);
        }
        else if (key == "-new" || key == "-new=tcp" || key == "-tcp")
        {
            c = &newTcpConnection();
        }
        else if (key == "-new=udp" || key == "-udp")
        {
            if (i == 1)
            {
                for(auto * c : _tcp_connections) {delete c;}
                _tcp_connections.resize(0); // connection is udp
            }
            
            c = &newUdpConnection();
        }
        else if (key == "-ouch")
        {
            c->setProtocol(Connection::Protocol::OUCH);
        }
        else if (key == "-itch")
        {
            c->setProtocol(Connection::Protocol::ITCH);
        }
        else if (key == "-rbuf")
        {
            i++;
            c->setReadBufferSize(atoi(argv[i]));
        }
        else if (key == "-wbuf")
        {
            i++;
            c->setWriteBufferSize(atoi(argv[i]));
        }
        else if (key == "-rwnd")
        {
            i++;
            c->setWindowSize(atoi(argv[i]));
        }
        else if (key == "-rtimeout")
        {
            i++;
            c->setReadTimeout(atoi(argv[i]));
        }
        else if (key == "-keepalive")
        {
            c->setKeepAlive(true);
        }
        else if (key == "-bind")
        {
            i++;
            params.clear();
            ::fastapp::utils::split(params, argv[i],":");

            c->setLocalIP(params[0]);
            c->setLocalPort(params.size() > 1 ? atoi(params[1].c_str()) : 0);
        }
        else if (key == "-dest")
        {
            i++;
            params.clear();
            ::fastapp::utils::split(params, argv[i],":");
            c->setRemoteIP(params[0]);
            c->setRemotePort
            (
                 (params.size() > 1 ? atoi(params[1].c_str()) : 0)
            );
        }
        else if (key == "-session" || key== "-sessionid")
        {
            i++;
            c->setSessionID(argv[i]);
        }
        else if (key == "-env")
        {
            i++;
            _env = argv[i];
        }
        else if (key == "-listReqOn")
        {
            c->setListReqMod(true);
        }
        else if (key == "-listReqOff")
        {
            c->setListReqMod(false);
        }
        else if (key == "-sleep")
        {
            i++;
            _sleep = atoi(argv[i]);
        }
        else if (key == "-delta")
        {
            i++;
            _delta = atoi(argv[i]);
        }
        else if (key == "-stuck_sec")
        {
            i++;
            _stuck_sec = atoi(argv[i]);
        }
        else if (key == "-print_delta")
        {
            i++;
            _print_delta = atoi(argv[i]);
        }
        else if (key == "-seq_num")
        {
            i++;
            c->setSeqNum
            (
                atoi(argv[i]) 
            );
        }
        else if (key == "-sym")
        {
            i++;
            c->setSymbols(argv[i]);
        }
        else if (key == "-depth")
        {
            i++;
            c->setDepth(atoi(argv[i]));
        }
        else if (key == "-sub_type")
        {
            using fastapp::itch::eSubType;
            i++;
            char ch = *argv[i];
            if ( ch == (char)eSubType::Aggregated || ch == (char)eSubType::NonAggregated)
            {
                c->setSubType((eSubType)ch);
            }
            else if (
              strcmp(argv[i], "aggr") == 0 || strcmp(argv[i], "aggreagted") == 0
            )
            {
                c->setSubType(eSubType::Aggregated);
            }
            else if (
              strcmp(argv[i], "non") == 0 || strcmp(argv[i], "non-aggreagted") == 0
            )
            {
                c->setSubType(eSubType::NonAggregated);
            }
            else
            {
                FMPRINT( KRED , "Unknown aggregation subscription type, use aggregated subscription" , KNRM ,  "\n");
                c->setSubType(eSubType::Aggregated);
            }
            
        }
        else if (key == "-algo")
        {
            i++;
            parseAlgo(argv[i]); 
        }
        else if (key == "-user")
        {
            i++;
            c->setUsername(argv[i]);
        }
        else if (key == "-password")
        {
            i++;
            c->setPassword(argv[i]);
        }
        else if (key == "-blocking")
        {
            _block_mode = true;
        }
        else if (key == "-non_blocking")
        {
            _block_mode = false;
        }
        else if (key == "-mailhost")
        {
            i++;
            params.clear();

            ::fastapp::utils::split(params, argv[i], ":" );
            
            _mail.setMailHost
            (
                params[0], 
                (params.size() > 1 ? atoi(params[1].c_str()) : 25 ) 
            );
        }
        else if (key == "-from")
        {
            i++;
            _mail.setFrom(std::string(argv[i]) );
        }
        else if (key == "-to")
        {
            i++;
            _mail.setTo(std::string(argv[i]) );
        }
        else if (key == "-print")
        {
            using namespace fastapp::utils;
            
            i++;
            params.clear();
            ::fastapp::utils::split(params, argv[i], ":;,") ;
            uint64_t print_flag = c->getPrintFlag();
            for (const std::string & s: params)
            {
                if (s == "heartbeat=on")
                {
                    print_flag |= PrintFlags::ClientHrtBeat;
                    print_flag |= PrintFlags::ServerHrtBeat;
                }
                else if (s == "trace=on")
                {
                    print_flag |= PrintFlags::Trace;
                }
                else if (s == "book=on")
                {
                    print_flag |= PrintFlags::Book;
                }
                else if (s == "book=off")
                {
                    print_flag &= ~PrintFlags::Book;
                }
                else if (s == "lowlevel=on")
                {
                    print_flag |= PrintFlags::Lowlevel;
                }
                else if (s == "ouch=on")
                {
                    print_flag |= PrintFlags::Ouch;
                }                
                else if (s == "ouch=off")
                {
                    print_flag &= ~PrintFlags::Ouch;
                }                
                else
                {
                    FMPRINT( KRED , "Unknown print option:" , s , "; from option line: " , argv[i] , KNRM,  "\n" );
                }
            }
            c->setPrintFlag(print_flag);
        }
        else if (key == "-updateType")
        {
            i++;
            params.clear();
            ::fastapp::utils::split(params, argv[i], ":;," );
            uint64_t update_flag = c->getUpdateFlag();
            for (const std::string & s: params)
            {
                if (s == "book" || s == "book=on")
                {
                    update_flag |= (uint64_t)UpdateFlags::Book;
                }
                else if (s == "book=off")
                {
                    update_flag &= ~(uint64_t)UpdateFlags::Book;
                }                
                else if (s == "trade")
                {
                    update_flag |= (uint64_t)UpdateFlags::Trade;
                }
                else if (s == "midpoint")
                {
                    update_flag |= (uint64_t)UpdateFlags::Midpoint;
                }
                else if (s == "onclose")
                {
                    update_flag |= (uint64_t)UpdateFlags::Onclose;
                }
                else
                {
                    FMPRINT(KRED , "Unknown update flag option:" , s , "; from option line: " , argv[i] , KNRM,  "\n" );
                }
            }
            c->setUpdateFlag(update_flag);
        }
        else
        {
            FMPRINT( KRED , "Unknown option: " , argv[i] , KNRM ,  "\n");
            i++;
        }

    }

}

TcpConnection & App::newTcpConnection()
{
    _tcp_connections.push_back(new TcpConnection);
    return lastTcpConnection();
}

//TcpConnection & App::newConnection()
//{
//    return newTcpConnection();
//}

TcpConnection &  App::lastTcpConnection()
{
    return *_tcp_connections.back();
}

TcpConnection &  App::getTcpConnection(size_t index)
{
    return *_tcp_connections.at(index);
}

UdpConnection & App::newUdpConnection()
{
    _udp_connections.push_back(new UdpConnection);
    return lastUdpConnection();
}

UdpConnection &  App::lastUdpConnection()
{
    return *_udp_connections.back();
}

UdpConnection &  App::getUdpConnection(size_t index)
{
    return *_udp_connections.at(index);
}

} // fastapp
