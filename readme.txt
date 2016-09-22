#
#    This file is part of Fastmatch binary market data and order flow examples.
#
#    Fastmatch binary market data and order flow examples are free software: you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation, either version 3 of the License, or
#   (at your option) any later version.
#
#    Fastmatch binary market data and order flow examples are distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#
#    You should have received a copy of the GNU General Public License
#    along with Fastmatch binary market data examples.  If not, see <http:#www.gnu.org/licenses/>.
#

This package contains set of examples detailing ways of communicating with Fastmatch binary Market Data and Order/Trade gateways.  Please note examples are single threaded.  All reads/writes meant to happen on the same thread. This design makes code to run faster avoiding locks.  In case you need to scale multiple threads can still be used vertically. You would need to handle multiple sockets by the same thread by using either App class or perhaps your own session (connection) container.

Basic Descriptions of Files provided:
src/socketutils.h/ src/socketutils.cpp   - cover basic udp/tcp communications

src/connection.h, src/connection.cpp - cover basics of working with itch/ouch tcp and udp connections/sessions. Each connection class is really a tcp/udp session.
UdpConnection and TcpConnection classes provide abstraction for interaction with binary tcp/udp services covering both Market Data and Order/Trade interactions 

src/itch_connection.h          - defines a higher level set of Market Data connection classes such as     					tcp_itch_connection_t, udp_itch_connection_t,                                                                                                            					tcp_itch_connection_ref_t, udp_itch_connection_ref_t

src/itch_connection_helpers.h  - set of functions, helping to fully initialize               							connection objects (used in sample13.cpp)

src/itch/dispatcher.h          - define parsing and intercepting of SoupBin and itch/ouch like messages
src/ouch/dispatcher.h
src/SoupBin/dispatcher_udp.h
src/SoupBin/dispatcher.h
src/dispatcher_defs.h

src/print.h  - defines variadic macro PRINT and variadic template function print,  responsible for outputting data to console.

Macro PRINT can be redefined. In order to do that one has to define macro PRINT_PATH, pointing to a header file with another implementation of print/log function and  another definition of macro PRINT. 

Example of macro definition:

#define FMPRINT_PATH(p) src/print.h

And in that header one has to define PRINT macro e.g.:

#define FMPRINT(args...) ::fastapp::print(args)   

Inside of src/print.h includes PRINT_PATH as bellow:

#include FMPRINT_PATH(print)

This way console logging can be easily replaced by some other implementation

src/adapter.h   - defines set of classes helping to mesh together various dispatchers and callbacks ItchAdapter, ItchBareAdapter, OuchAdapter, OuchBareAdapter


src/connection.h
src/connection.cpp
src/app.h
src/app.cpp     - Code is using regular posix socket API.  Messages are transmitted out using send and received using recv. Methods run inside the Connection and App classes use continues loops. These behavior makes sense either for handling number of unblocking connections or for handling single blocking connection. Continues polling allows for better performance specially if combined with onload API. The goal and end result is typically 100% CPU utilization. This might be not desirable in quite a few use cases. Those use cases obviously require use of poll/epoll/select methods. No such code is provided yet as part of this set of examples.
It is possible to force blocking receive call to periodically time out. It may be needed for variety of reasons, e.g.  some housekeeping, exit, etc. This behavior may be achieved by using SO_RCVTIMEO socket option. Connection class has method setReadTimeout(seconds). Also one can or use option: 
“-rtimeout 10” (set 10 seconds read timeout) while running samples.


Binary Market Data Samples:

Following few examples demonstrates how to connect, login, heartbeat, logout and disconnect
These examples are somewhat low level as their show how to intercept the main session callback, the one effectively processing all messages coming in.

These 4 examples explicitly set and initialize SoupBin and itch dispatchers and are somewhat very explicit in exposing the very guts of this implementation.

Examples 1,11,12 demonstrate how to connect, login, heartbeat, logout and disconnect. 

Sample1.mak  ,  sample1.cpp      - mostly using facilities of TmplConnection class
Sample11.mak, sample11.cpp    -  using application class as a connection/session managing Container
Sample12.mak  sample12.cpp    -  progression to use more of class app, about the same as 11

Sample12x.mak, cli.cpp  - this one is a bit apart from previous examples. It is full blown example, able to subscribe to various market data in different ways. 

Examples 13,14,15 are somewhat more high level as they hide some of low level details.

Sample13.mak , sample13.cpp, itch_connection_helpers.h  
Sample14.mak,  sample14.cpp -  these two samples are similar and demonstrating how to subscribe and get market data, the difference is mostly in a way connection objects are initialized inside of getConnection  function. 
Please also note that, unlike in samples 11 and 12, one can subscribe for list of coma separated symbols
 
Sample15.mak, sample15.cpp - full blown example, able to subscribe to various market data in different ways. Intercepts SIGTERM and SIGINT signals, allows to specify in the command line different options detailing what to log, what data to subscribe for e.g. trades, BBO, market data and where to mail in case of an error. 

Also note that one can subscribe for list of coma separated symbols as well as ALL available symbols. Second option works only because subscription request is issued in response to InstrumentInfoMessage message and ListRequestMessage can be issued with symbol ALL, triggering multiple InstrumentInfoMessages.

All examples can be built with onload=true,  in case you use Solarflare network cards and optimize for performance.

Example:   
    Fastmatch>  make -f Sample15.mak onload=true

The code assumes an onload library is downloaded and placed in onload folder, please take a look at the makefile of any example. 

Onload/
     include/
         onload/
             extensions.h  
      extensions_zc.h
     lib/
         libonload_ext.a  
    libonload_ext.so  
    libonload_ext.so.1  
    libonload_ext.so.1.0.0  
    libonload.so
Fastmatch/
    src/
    Sample15.mak

Example : 
(. ./onload_profile.sh ; onload --profile=latency GNU-amd64-Linux/Sample1/Sample15 -itch -bind eth2 -dest ip_address:port_number -blocking -sleep 0 -rbuf 16024 -wbuf 1024 -sym "ALL" )

In this example we

1. source onload_profile.sh in order to spin TCP and UDP reads and writes
2. run example with onload.so preloaded and using onload stacks
3. bind to eth2 interface
4. connect to endpoint ip_address:port
5. use blocking socket
7. introduce no delays during message processing 
8. specify read and write socket buffers

One can run executable without options to see full list of options available

GNU-amd64-Linux/Sample1/Sample15 <Options>

Options:
        -help - this prompt
        -new
        -new=tcp
        -tcp  - creates tcp connection/session
        -new=udp
        -udp  - creates udp connection/session
        -bind localip:port
        -rbuf READ_BUFFER_SIZE in bytes
        -wbuf WRITE_BUFFER_SIZE in bytes
        -dest <host>:<port>
        -sym      SYMBOL   ,
        -depth    LEVELS   - 0-means all levels,
        -sub_type FLAG     - 1-aggregated, 2-not aggregated,
        -print heartbeat=on,trace=on,book=on[,book=off],lowlevel=on - enable disable loggers combinations,
        -updateType book,book=on,book=off,trade,midpoint,onclose ,
        -blocking          - use blocking IO sockets ,
        -non_blocking      - use non blocking sockets ,

        CREDENTIALS
        -user     USERNAME ,
        -password PASSWORD ,

        EMAIL
        -mailhost SERVER:PORT- mail server,
        -from SENDERS_EMAIL,
        -to   DESTINATION_EMAIL,

        EXTRA
        -session  SESIONID ,
        -seq_num  SEQUENCE_NUMBER,
        -sleep    millis, delay message consumption on each loop iteration by xxx milliseconds, can be used for testing slow client behavior
        -delta    millis, time in millis to e.g. submit orders
        -stuck_sec   sec, quote considered stuck if it is on the book for longer than time in sec provided
        -print_delta    millis, time in millis to e.g. print statistics
        -env      NAME     - envornment name,
        -listReqOn         - enable list request,

        -listReqOff       -  disable list request,

        -algo new|rpl     - select algo strategy send new/rpl,

Examples:
        taskset -c 14-15 ./GNU-amd64-Linux/Sample1/Sample14  -bind interface_ip_address e.g. 192.168.1.100  -dest server:port  -sym EUR/USD -sub_type 2  -depth 0 -print book=on  -updateType trade,midpoint,book -non_blocking -user FM -password FM &> out.txt
         In order to run with onload as below, please make sure to build examples with onload=true e.g. make -f SampleXYZ.mak onload=true
         Also you need to export content of onload_profile.sh, onload has quite a few settings and you may want to consult onload docs
         modify makefile to point to onload headers and libs
        (. ./onload_profile.sh ; taskset -c 14-15 onload --profile=latency ./GNU-amd64-Linux/Sample1/Sample14  -bind interface_name e.g. eth1 -dest server:port -sym EUR/USD -sub_type 2  -depth 0 -print book=on  -updateType trade,midpoint,book -non_blocking )
        taskset -c 14-15 onload --profile=latency ./GNU-amd64-Linux/Sample1/Sample14  -bind interface_name -dest server:port -sym EUR/USD -sub_type 2  -depth 0 -print book=on  -updateType trade,midpoint,book -blocking -mailhost mailhost:port


Applications: Sample15 and ItchMon, Sample12x are providing about the same end result and allow to subscribe and collect market data



Binary Oder Flow Samples (coming soon)

Application Sniper. Application Sniper is a test application capable of aiming at market data using two different strategies: 
1.  IOC orders
2. Creating Limit orders with a limit price obtained by crossing the book. The orders are Replaced shortly after in case those orders are still open.


