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

#include <iostream>
#include <string>

#include <src/defs.h>
#include <src/socketutils.h>
#include <src/soup_handlers.h>
#include <src/connection.h>


namespace fastapp
{
   
namespace soupbin
{

namespace callbacks
{
    
void Handlers::registerSoupbinCallbacks ()    
{
    using namespace fastapp::soupbin;
    
    if (_session != nullptr)
    {
        _session->registerOnSend
        (
            [&](const TcpLogin & login)
            {
                set(login);
            }
        );
    }

    sub
    ( [&] (const LoginAccept & loginAccept, int32_t size, uint64_t seq) 
    {
        int64_t tod = ::fastapp::utils::getTimeOfDay();
        FMPRINT( ::fastapp::Micro(tod), " : LoginAccept - session:'"\
        , std::string(loginAccept.session, ::fastapp::soupbin::session_size) , "' : next soupbin sequence: '"\
        , std::string(loginAccept.seqnum, ::fastapp::soupbin::seqnum_size) , "'");
        if (seq != 0)
        {
           FMPRINT( " seq_num: '"  , seq , "'" );
        }
//        if (strncmp(loginAccept.session, _sbd.getLogin().session, ::fastapp::soupbin::session_size) != 0)
//        {
//            FMPRINT( "\n") , KRED , "<sessionID:'" , std::string(loginAccept.session, ::fastapp::soupbin::session_size) , "'!='" , d.getLogin().session , "'>" KNRM;
//        }
        if (setSessionID)
        {    setSessionID ( std::string(loginAccept.session, ::fastapp::soupbin::session_size) );    }

        auto login = getLogin();
        ssize_t i = std::stoi(std::string(login.seqnum, ::fastapp::soupbin::seqnum_size));
        ssize_t in_seq = std::stoi(std::string(loginAccept.seqnum, ::fastapp::soupbin::seqnum_size));
        if (seq != 0 && (ssize_t)seq != in_seq - 1)
        {
            FMPRINT( "\n" , KRED , "<GAP! sequence number gap:'(" , in_seq  , " - 1) != " , seq , "'>", KNRM);
        }

        
        char padded_seq[::fastapp::soupbin::seqnum_size];
        sprintf(padded_seq, "%*ld", ::fastapp::soupbin::seqnum_size, i);
        if (strncmp(loginAccept.seqnum, padded_seq, ::fastapp::soupbin::seqnum_size) != 0)
        {
            FMPRINT( "\n" , KYEL , "<Ignoring requested seqnum:(" , login.seqnum , ") != actual seqnum (" , std::string(loginAccept.seqnum, ::fastapp::soupbin::seqnum_size) , ") >" , KNRM);
        }


        FMPRINT( "\n");
    }
    );

    
    sub
    ( [&] (const LoginReject & loginReject, int32_t size, uint64_t seq)
    {
        int64_t tod = ::fastapp::utils::getTimeOfDay();  
        FMPRINT(::fastapp::Micro(tod) , " : LoginReject : '" , loginReject.rejectReason , "'"\
         ,  ( (seq != 0) ?  (std::string(" seq_num: '") +  std::to_string(seq) +  std::string("'") ) : std::string())\
         , "\n");
    }
    );
    
    sub 
    ( [&] (const ClientHeartbeat &, int32_t size, uint64_t seq)
    {
        if (IsCliHrtbtOn ? IsCliHrtbtOn() : true)
        {
            /*
            static int i = 0;
            if (i % 5 == 0)
            {
                int64_t tod = ::fastapp::utils::getTimeOfDay();  
                ::fastapp::utils::formatTime(tod, std::cerr);                  
                FMPRINT( " : ClientHeartbeat" 
                          , ( (seq != 0) ?  (std::string(" seq_num: '") +  std::to_string(seq) +  std::string("'") ) : std::string() )
                          , "\n");
            }

            ++i;
            */
        }
    }
    );
    
    sub
    (
    [&] (const ServerHeartbeat &, int32_t size, uint64_t seq)
    {
        /*
        if (IsSrvHrtbtOn ? IsSrvHrtbtOn() : true)
        {
            static int i = 0;
            if (i % 5 == 0)
            {
                int64_t tod = ::fastapp::utils::getTimeOfDay();  
                ::fastapp::utils::formatTime(tod, std::cerr);                  
                FMPRINT( " : In (ServerHeartbeat)" 
                          ,  ( (seq != 0) ?  (std::string(" seq_num: '") +  std::to_string(seq) +  std::string("'") ) : std::string() )                         
                          , "\n");
            }

            ++i;
        }
        */
    }
    );
    
    if (_override_seq)
    {
        sub
        (
        [&] (const SequencedDataHeader & data, int32_t size, uint64_t seq)
        {
            bool trace = IsTraceOn ? IsTraceOn() : false;
            if (trace)
            {
                int64_t tod = ::fastapp::utils::getTimeOfDay();
                FMPRINT( ::fastapp::Micro(tod) , " : SequencedDataHeader seq: " , seq , " : size (", size , ")");
            }
            if (size > (int32_t)sizeof(SequencedDataHeader) && size >= data.size)
            {
                if (trace) FMPRINT( " : Soupbin SequencedData : size (" , size - (int32_t)sizeof(SequencedDataHeader) , ") : ");

                // Place to call
                // ITCH/OUTCH processor
            }
            FMPRINT(  ( (seq != 0) ?  (std::string(" seq_num: '") +  std::to_string(seq) +  std::string("'") ) : std::string() ) , "\n");
        }
        );
    }
    
    sub
    ( [&] (const UnsequencedDataHeader & m, int32_t size, uint64_t seq)
    {
        int64_t tod = ::fastapp::utils::getTimeOfDay();  
        FMPRINT( ::fastapp::Micro(tod) , " : UnSequencedDataHeader : " , m.type , ":" , size\
        , ( (seq != 0) ?  (std::string(" seq_num: '") +  std::to_string(seq) +  std::string("'") ) : std::string() ) , "\n");
    }
    );
    
    sub
    ( [&] (const EndOfSession & m, int32_t size, uint64_t seq)
    {
        int64_t tod = ::fastapp::utils::getTimeOfDay();  
        FMPRINT( Micro(tod) , " : EndOfSession : " , m.type , ":" , size\
        , ( (seq != 0) ?  (std::string(" seq_num: '") +  std::to_string(seq) +  std::string("'") ) : std::string() ) , "\n");
    }
    );
}

}
}
}
