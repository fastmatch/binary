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

#ifndef ADAPTER_H
#define	ADAPTER_H

#include <src/print.h>

namespace fastapp
{
namespace ouch
{
class Ostore;
}
}

#include <src/ostore.h>
#include <src/itch/dispatcher.h>
#include <src/ouch/dispatcher.h>


namespace fastapp
{
namespace callbacks
{

template <typename DS, typename CALLBACK>
class ItchAdapter
{
public:
    typedef ::fastapp::itch::Dispatcher DI;

    ItchAdapter () : call(dsoup, ditch), _initialized(false)
    {
        init();
    }

    ItchAdapter (const ItchAdapter &) = default;
    ItchAdapter (ItchAdapter &&) = default;
    ItchAdapter & operator = (ItchAdapter &&) = default;
    ItchAdapter & operator = (ItchAdapter &) = default;
    ~ItchAdapter () = default;


    void process(void *ptr, int32_t size)
    {
        dsoup.process(ptr, size);
    }

    void init()
    {
        if (_initialized) return;

        call.registerCallbacks () ;

        dsoup.sub( [&] (const ::fastapp::soupbin::SequencedDataHeader & data, int32_t size, uint64_t seq)
        {
            bool trace = ditch.IsTraceOn ? ditch.IsTraceOn() : false;

            if (trace)
            {
                uint64_t tod = ::fastapp::utils::getTimeOfDay() ;
                FMPRINT(Micro(tod)," :Itchd SequencedDataHeader seq: ", seq , " : size (", size,  ") data.size: (", data.size, ")" );
            }

            if (size > (int32_t)sizeof(::fastapp::soupbin::SequencedDataHeader) && size >= data.size)
            {
                if (trace) FMPRINT(" : Itchd SequencedData : size (", size - (int32_t)sizeof(::fastapp::soupbin::SequencedDataHeader), ") : ");

                ditch.process
                (
                    (void *)(reinterpret_cast<const char *>(&data) + sizeof(::fastapp::soupbin::SequencedDataHeader)) ,
                    size - (int32_t)sizeof(::fastapp::soupbin::SequencedDataHeader), seq
                );
            }

            if (trace) FMPRINT("\n");
        });

        _initialized = true;
    }

    void set(const fastapp::soupbin::Login & login)
    {
        call.set(login);
    }
    
    void clear(const std::string & sym)
    {
        call.clearBook(sym);
    }

    void setStuckDelay(int64_t sec)
    {
        call.setStuckDelay(sec);
    }
    
    void registerInfoMessage(typename DI::FuncInstrumentInfoMessage func)
    {
        ditch.sub(func); // _funcInfoMsg = func;
    }

    void registerEOFSession(typename DS::FuncEndoOfSession func)
    {
        dsoup.sub(func) ; //_funcEOFSession = func;
    }
    
    void registerBookEvent(typename DI::FuncBookEvent func)
    {
        ditch.sub(func); //_book_event = func;
    }

    void registerSessionID(fastapp::callbacks::set_id_t   setSessionID )
    {
        call.reg(setSessionID);
    }

    void registerBookOn(fastapp::callbacks::is_func_t  bookOn )
    {
        call.IsBookOn     = bookOn;
    }

    void registerAllOn(fastapp::callbacks::is_func_t  hbOn )
    {
        call.IsCliHrtbtOn = hbOn;
        call.IsSrvHrtbtOn = hbOn;
        call.IsTraceOn    = hbOn;
        call.IsBookOn     = hbOn;
        ditch.IsBookOn    = hbOn;
        ditch.IsTraceOn   = hbOn;

    }

    void registerHelpers
    (
        fastapp::callbacks::is_func_t  traceOn, 
        fastapp::callbacks::is_func_t  srvHrtOn, 
        fastapp::callbacks::is_func_t  cliHrtOn,
        fastapp::callbacks::is_func_t  bookOn,
        fastapp::callbacks::set_id_t   setSessionID,
        fastapp::callbacks::send_email_t email
    )
    {
        call.IsTraceOn    = traceOn;
        call.IsCliHrtbtOn = cliHrtOn;
        call.IsSrvHrtbtOn = srvHrtOn;
        call.IsBookOn     = bookOn;
        ditch.IsBookOn    = bookOn;
        ditch.IsTraceOn   = traceOn;
        call.reg(setSessionID);
        ditch.setEmailCallback(email);
    }
    
    
protected:    
    DS dsoup;
    DI ditch;
    CALLBACK call;
    bool _initialized;
};



template <typename DS>
class ItchBareAdapter
{
public:
    typedef ::fastapp::itch::Dispatcher DI;

    ItchBareAdapter () 
    {
        init();
    }

    ItchBareAdapter (const ItchBareAdapter &) = default;
    ItchBareAdapter (ItchBareAdapter &&) = default;
    ItchBareAdapter & operator = (ItchBareAdapter &&) = default;
    ItchBareAdapter & operator = (ItchBareAdapter &) = default;
    ~ItchBareAdapter () = default;


    void process(void *ptr, int32_t size)
    {
        dsoup.process(ptr, size);
    }

    void set(const fastapp::soupbin::Login & login)
    {
        dsoup.set(login);
    }

    void init()
    {
        dsoup.sub( [&] (const ::fastapp::soupbin::SequencedDataHeader & data, int32_t size, uint64_t seq)
        {
            bool trace = ditch.IsTraceOn ? ditch.IsTraceOn() : false;

            if (trace)
            {
                uint64_t tod = ::fastapp::utils::getTimeOfDay() ;
                FMPRINT(tod, " :Itchd SequencedDataHeader seq: ",  seq, " : size (", size, ") data.size: (", data.size, ")");
            }

            if (size > (int32_t)sizeof(::fastapp::soupbin::SequencedDataHeader) && size >= data.size)
            {
                if (trace) FMPRINT(" : Itchd SequencedData : size (" , size - (int32_t)sizeof(::fastapp::soupbin::SequencedDataHeader) , ") : ");

                ditch.process
                (
                    (void *)(reinterpret_cast<const char *>(&data) + sizeof(::fastapp::soupbin::SequencedDataHeader)) ,
                    size - (int32_t)sizeof(::fastapp::soupbin::SequencedDataHeader), seq
                );
            }

            if (trace) FMPRINT("\n");
        });
    }
    
    void registerInfoMessage(typename DI::FuncInstrumentInfoMessage func)
    {
        ditch.sub(func);
    }

    void registerEOFSession(typename DS::FuncEndoOfSession func)
    {
        dsoup.sub(func);
    }
    
    void registerBookEvent(typename DI::FuncBookEvent func)
    {
        ditch.sub(func);
    }

    void registerAllOn(fastapp::callbacks::is_func_t  hbOn )
    {
        ditch.IsBookOn    = hbOn;
        ditch.IsTraceOn   = hbOn;
    }

    void registerHelpers
    (
        fastapp::callbacks::is_func_t  traceOn, 
        fastapp::callbacks::is_func_t  bookOn,
        fastapp::callbacks::send_email_t email
    )
    {
        ditch.IsBookOn    = bookOn;
        ditch.IsTraceOn   = traceOn;
        ditch.setEmailCallback(email);
    }
    
    DS  & getSoupbinDispatcher()
    {
        return dsoup;
    }

    DI & getItchDispatcher()
    {
        return ditch;
    }

protected:    
    DS dsoup;
    DI ditch;
};

template <typename DS, typename CALLBACK>
class OuchAdapter
{
public:

    typedef ::fastapp::ouch::Dispatcher DO;


    OuchAdapter () : call(dsoup, douch),  _initialized(false)
    {
        init(nullptr);
    }

    OuchAdapter(::fastapp::ouch::Ostore * store) : call(dsoup, douch),  _initialized(false)
    {
        init(store);
    }

    OuchAdapter (const OuchAdapter &) = default;
    OuchAdapter (OuchAdapter &&) = default;
    OuchAdapter & operator = (OuchAdapter &&) = default;
    OuchAdapter & operator = (OuchAdapter &) = default;
    ~OuchAdapter () = default;
    

    void process(void *ptr, int32_t size)
    {
        dsoup.process(ptr, size);
    }

    void init_store(::fastapp::ouch::Ostore * store) 
    {
        if (store != 0) call.setStore(store);
    }


    void init(::fastapp::ouch::Ostore * store)
    {
        if (_initialized) return;

        call.registerCallbacks();

        init_store(store);

        dsoup.sub( [&] (const ::fastapp::soupbin::SequencedDataHeader & data, int32_t size, uint64_t seq)
        {
            bool trace = douch.IsTraceOn ? douch.IsTraceOn() : false;
            bool ouch  = douch.IsOuchOn  ? douch.IsOuchOn()  : false;
            if (trace && ouch )
            {
                int64_t tim = ::fastapp::utils::getTimeOfDay();
                FMPRINT( ::fastapp::Micro(tim) , " : Ouch SequencedDataHeader seq: " , seq , " : size (", size , ") data.size: (", data.size, ")");
            }

            if (size > (int32_t)sizeof(::fastapp::soupbin::SequencedDataHeader) && size >= data.size)
            {
                if (trace && ouch ) 
                {   int64_t tim = ::fastapp::utils::getTimeOfDay();
                    FMPRINT( ::fastapp::Micro(tim) , " : Ouch SequencedData : size (" , size - (int32_t)sizeof(::fastapp::soupbin::SequencedDataHeader) , ") : "); 
                }

                douch.process
                (
                    (void *)(reinterpret_cast<const char *>(&data) + sizeof(::fastapp::soupbin::SequencedDataHeader)) ,
                    size - (int32_t)sizeof(::fastapp::soupbin::SequencedDataHeader), seq
                );
            }

            if (trace && ouch )
            {    FMPRINT( "\n");    }
        });


        _initialized = true;

    }

    void set(const fastapp::soupbin::Login & login)
    {
        call.set(login);
    }
    
    void registerEOFSession(typename DS::FuncEndoOfSession func)
    {
        dsoup.sub(func);
    }

    void registerHelpers
    (
        fastapp::callbacks::is_func_t  traceOn, 
        fastapp::callbacks::is_func_t  srvHrtOn, 
        fastapp::callbacks::is_func_t  cliHrtOn,
        fastapp::callbacks::is_func_t  ouchOn,
        fastapp::callbacks::set_id_t   setSessionID,
        fastapp::callbacks::send_email_t email
    )
    {
        call.IsTraceOn    = traceOn;
        call.IsCliHrtbtOn = cliHrtOn;
        call.IsSrvHrtbtOn = srvHrtOn;
        call.IsOuchOn     = ouchOn;
        douch.IsOuchOn    = ouchOn;
        douch.IsTraceOn   = traceOn;
        call.reg(setSessionID);
        douch.setEmailCallback(email);
    }
    
    void setStore(::fastapp::ouch::Ostore * store)
    {
        call.setStore(store);
    }

    void setConnection(::fastapp::TcpConnection * trade_con)
    {
        call.setConnection(trade_con);
    }
    
protected:    
    DS dsoup;
    DO douch;
    CALLBACK call;
    bool  _initialized;
};

}
}

#endif	/* ADAPTER_H */

