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

#ifndef ITCH_HDRS_H
#define	ITCH_HDRS_H

#include <src/defs.h>

#include <src/soup_handlers.h>

#include <src/itch/dispatcher.h>
#include <src/connection.h>
#include <src/itch/book.h>
#include <vector>

namespace fastapp
{
class ConnectionBase;

namespace itch
{
    
namespace callbacks
{

class Handlers : public ::fastapp::soupbin::callbacks::Handlers
{

public:

    Handlers (::fastapp::soupbin::Dispatcher & sbd
            , ::fastapp::itch::Dispatcher & itd
            , ::fastapp::ConnectionBase * session)
    : ::fastapp::soupbin::callbacks::Handlers(sbd, session)
    , IsBookOn([]() {return true;} )
    , _itchd(itd)
    , _stuck_sec(120)
    {
        _books.resize(512);
    }
    
    Handlers (::fastapp::soupbin::Dispatcher & sbd
            , ::fastapp::itch::Dispatcher & itd
            , ::fastapp::ConnectionBase & session)
    : ::fastapp::soupbin::callbacks::Handlers(sbd, &session)
    , IsBookOn([]() {return true;} )
    , _itchd(itd)
    , _stuck_sec(120)
    {
        _books.resize(512);
    }
    Handlers (::fastapp::soupbin::Dispatcher & sbd, ::fastapp::itch::Dispatcher & itd)
    : ::fastapp::soupbin::callbacks::Handlers(sbd)
    , IsBookOn([]() {return true;} )
    , _itchd(itd)
    , _stuck_sec(120)
    {
        _books.resize(512);
    }

    void registerCallbacks ()
    {
        registerSoupbinCallbacks () ;
        registerItchCallbacks () ;
    }
 
    void registerItchCallbacks () ;
    void registerItchCallbacks
    (
          ::fastapp::itch::Dispatcher & itchd
        , ::fastapp::soupbin::Dispatcher & sbd
        , ConnectionBase * session
    );
    
    void setSymbol(uint32_t index, const std::string & sym)
    {
        setSym(index, sym);
        setStuckDelay(index, _stuck_sec);
    }

    void setStuckDelay(int64_t sec)
    {
        for (Book & book : _books)
        {
            book.setStuckDelay(sec);
        }
        _stuck_sec = sec;
    }

    void setStuckDelay(uint32_t index, int64_t sec)
    {
        if (index >= _books.size()) return;

        Book & book = _books.at(index);
        book.setStuckDelay(sec);
    }

    void setSym(uint32_t index, const std::string & sym)
    {
        if (index >= _books.size()) return;

        Book & book = _books.at(index);
        book.setSymbol(sym);
        book.wipe();
    }

    const std::string & getSymbol(uint32_t index) const
    {
        static const std::string empty("?");
        if (index >= _books.size()) return empty;
        return _books[index].getSymbol();
    }


    void clearBook(const std::string & sym)
    {
        for (Book & book : _books)
        {
            if (book.getSymbol() == sym)
            {
                book.wipe();
            }
        }
    }

    void clearBook(uint32_t index)
    {
        if (index >= _books.size()) return;

        _books.at(index).wipe();
    }

    void setEmailCallback(fastapp::callbacks::send_email_t email)
    {
        sendEmail = email;
        for (Book & book : _books)
        {
            book.setEmailCallback(email);
        }
    }

    void registerBookOn(fastapp::callbacks::is_func_t  bookOn )
    {
        IsBookOn     = bookOn;
    }

    void registerAllOn(fastapp::callbacks::is_func_t  hbOn )
    {
        IsCliHrtbtOn = hbOn;
        IsSrvHrtbtOn = hbOn;
        IsTraceOn    = hbOn;
        IsBookOn     = hbOn;
    }

    void registerHelpers
    (
        fastapp::callbacks::is_func_t  traceOn,
        fastapp::callbacks::is_func_t  srvHrtOn,
        fastapp::callbacks::is_func_t  cliHrtOn,
        fastapp::callbacks::is_func_t  bookOn,
        fastapp::callbacks::set_id_t   setSssnID,
        fastapp::callbacks::send_email_t email
    )
    {
        IsTraceOn    = traceOn;
        IsCliHrtbtOn = cliHrtOn;
        IsSrvHrtbtOn = srvHrtOn;
        IsBookOn     = bookOn;
        setSessionID = setSssnID;
        setEmailCallback(email);
    }

    void registerHelpers
    (
        fastapp::callbacks::is_func_t  traceOn,
        fastapp::callbacks::is_func_t  srvHrtOn,
        fastapp::callbacks::is_func_t  cliHrtOn,
        fastapp::callbacks::is_func_t  bookOn,
        fastapp::callbacks::send_email_t email
    )
    {
        IsTraceOn    = traceOn;
        IsCliHrtbtOn = cliHrtOn;
        IsSrvHrtbtOn = srvHrtOn;
        IsBookOn     = bookOn;
        setEmailCallback(email);
    }
    ::fastapp::callbacks::is_func_t   IsBookOn;
    
protected:

    ::fastapp::itch::Dispatcher    & _itchd;
    int64_t                          _stuck_sec;
    std::vector<Book>                _books;
    fastapp::callbacks::send_email_t  sendEmail;
};
    
void printErrorCode(int16_t errorCode);

}

}

}
#endif	/* SOUPBIN_HDRS_H */

