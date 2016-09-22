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

#ifndef BIN_SMTP_H
#define	BIN_SMTP_H

#include <map>

namespace smtp
{


class Mail
{
    
private:
    
    bool trace = false;
    int sockfd = 0;
    struct sockaddr_in  serv_addr;    

    std::string         mailhost;
    std::string         to;
    std::string         from;

    
    void log(const char * text);
    void log(const char * format, const char * params);
    void plog(int err, const char * text);
    void plogf(int err, const char * format, ...);
    void closeSocket(int fd); 

public:    
    int  init(const std::string & host, int port, const std::string & from_, const std::string & to_);
    int  send (const std::string & subject, const std::string & body);
    
    void setTracing(bool on);
    int  setMailHost(const std::string & host, int mailport);
    void setFrom(const std::string & from_);
    void setTo(const std::string & to_);
    bool isInitialized()
    {
        return (serv_addr.sin_addr.s_addr != 0 );
    }
    
    const char * error(ssize_t error_code) const;
    
    int sendAlert
    ( 
        const std::string& source,
        const std::string& subject,
        const std::string& msg,
        const std::string& subject_prefix,
        bool force = false 
    );

private:
    typedef std::pair<std::string, std::string> Key;
    typedef std::map<Key, int64_t> Events;    
    
    Events       _events;
};

} // smtp


#endif	/* SMTP_H */

