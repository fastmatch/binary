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

#include <stdlib.h>
#include <strings.h>
#include <unistd.h>
#include <cstring>

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <utility>
#include <string>

#include <src/smtp.h>
#include <iostream>
#include <src/socketutils.h>
#include <src/print.h>
#include <cstdarg>

namespace smtp
{

namespace
{
    static const char *error_text[] = 
    {
        "Ok" //0
    ,   "ERROR opening socket" //1
    ,   "ERROR, no such host"  //2
    ,   "ERROR connecting"     //3
    ,   "ERROR reading from socket"  //4
    ,   "ERROR writing to socket"    //5
    ,   "ERROR invalid mailhost" //6                
    };
}

void Mail::plog(int err, const char * text)
{
#ifdef FMPRINT
    if (trace)
    {
        const int bsize = 2048;
        char buf[bsize];
        if (strerror_r(err, buf,  bsize) == 0)   
        {    FMPRINT(text, buf, "\n");    }
        else
        {   FMPRINT(text, "\n"); }
    }
#endif
}

void Mail::plogf(int err, const char * format, ...)
{
#ifdef FMPRINT
    if (trace)
    {
        const int bsize = 2048;
        char buffer[bsize];
        va_list params;
        va_start(params, format);

        vsnprintf(buffer, bsize, format, params);

        plog(err, (const char *)buffer);
    }
#endif
}

void Mail::log(const char * text)
{
#ifdef FMPRINT
    if (trace) FMPRINT(text,"\n");
#endif
}

void Mail::log(const char * format, const char * params)
{
#ifdef FMPRINT
    if (trace)
    {
        const int bsize = 2048;
        char buf[bsize];
        
        snprintf(buf, bsize, format, params);
        FMPRINT(buf);
    }
#endif
}


int Mail::init(const std::string & host, int port, const std::string & from_, const std::string & to_)
{
    setFrom(from_);
    setTo(to_);
    return setMailHost(host, port);
}
void Mail::setTracing(bool on)
{
    trace = on;
}

int Mail::setMailHost(const std::string & host, int mailport)
{
    struct hostent *server(0);
    memset((void *) &serv_addr, 0, sizeof(serv_addr));

    mailhost = host;

    server = gethostbyname(mailhost.c_str());
    if (server == NULL) 
    {
        return -2;
    }

    serv_addr.sin_family = AF_INET;
    memcpy((char *)&serv_addr.sin_addr.s_addr, (char *)server->h_addr, server->h_length);
    serv_addr.sin_port = htons(mailport);       
    return 0;
}



void Mail::setFrom(const std::string & from_)
{
    from = from_;
}

void Mail::setTo(const std::string & to_)
{
    to = to_;
}

int getSO_ERROR(int fd) 
{
   int err = 1;
   socklen_t len = sizeof err;
   if (-1 == getsockopt(fd, SOL_SOCKET, SO_ERROR, (char *)&err, &len))
   {   return -1;    }
   if (err)
   {   
       errno = err;              // set errno to the socket SO_ERROR
   }
   return err;
}

void Mail::closeSocket(int fd) 
{ 
   if (fd >= 0) 
   {
      if(getSO_ERROR(fd) == -1) // clear any errors, which can cause close to fail
      {
          plog(errno, "socket in unknown state");
          return;
      }
      
      if (shutdown(fd, SHUT_RDWR) < 0) // terminate the tcp-reliable delivery
      {
         if (errno != ENOTCONN && errno != EINVAL) 
         {
             char discard[64];
             int ret = 0;
             int n = 0;
             while( (ret=read(fd, discard, sizeof(discard) )) > 0)
             {
                 if (n >20) break;
                 n++;
             };
             
             if ( ret <= 0)
             {
                 plogf(errno, "n=%d, socket shutdown", n);
             }
             else
             {
                 plog(errno, "failed to flush socket");
             }
         }
         else
         {
             plog(errno, "socket shutdown failed");
         }
      }
      
      int err = close(fd);
      if (err < 0) // finally call close()
      {
          plogf(errno, "failed to close socket, errno=%d", errno);
      }
      else
      {
          // plog(errno, "socket closed");
      }
   }
}

int Mail::sendAlert
( 
    const std::string& source,
    const std::string& subject,
    const std::string& msg,
    const std::string& subject_prefix,
    bool force 
)
{
    if( force )
    {
        return send( subject_prefix + subject, msg );
    }

    int64_t now = ::fastapp::utils::getTimeOfDay();

    //int64_t & eventTime = _events[ std::make_pair(source, subject) ];

    auto pr = _events.insert(std::make_pair(std::make_pair(source, subject),0));

    int64_t & eventTime = (*((pr).first)).second;

    if( pr.second || now - eventTime > 300000000 )
    {
        eventTime = now;
        return send( subject_prefix + subject, msg );
    }

    return 0;
}

int Mail::send
(
  const std::string & subject
, const std::string & body
)
{
    if (serv_addr.sin_addr.s_addr == 0 ) return -6;
        
    int n;
    static const int buf_size = 256;
    static const int buf_size_m1 = buf_size - 1;;
    static const int last_index  = buf_size - 1;
    char buffer[buf_size];

    //portno = atoi(argv[2]);

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
    {
        return -1;
    }
    
    log("login into %s", mailhost.c_str());
    if (connect(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0) 
    {
        return -3;
    }

    n = read(sockfd,buffer,buf_size_m1);
    if (n < 0) 
    {
        return -4;
    }

    if (n < buf_size) {buffer[n] = 0;} else {buffer[last_index] = 0;}

    log(buffer);

/*------------------------------*/

    log("EHLO");
    sprintf (buffer,"ehlo %s\n", mailhost.c_str());
    n = write(sockfd, buffer,strlen(buffer) );
    if (n < 0) 
    {
        return -5;
    }

    n = read(sockfd, buffer, buf_size_m1);
    if (n < 0) 
    {
        return -4;
    }

    if (n < buf_size) {buffer[n] = 0;} else {buffer[last_index] = 0;}

    log("%s\n",buffer);

    sprintf(buffer, "MAIL FROM: %s\n", from.c_str());
    log(buffer);

    n = write(sockfd, buffer, strlen(buffer));
    if (n < 0) 
    {     return -5; //error("ERROR writing to socket");    
    }

    n = read(sockfd, buffer, buf_size_m1);
    if (n < 0) 
    {
        return -4; //error("ERROR reading from socket");
    }

    if (n < buf_size) {buffer[n] = 0;} else {buffer[last_index] = 0;}

    log("\t%s\n",buffer);


    log("MAIL TO");

    sprintf(buffer,"RCPT TO: %s\n", to.c_str());
    log("\n%s", buffer);
    n = write(sockfd,buffer,strlen(buffer));
    if (n < 0) 
    {
        return -5;
    }

    n = read(sockfd,buffer,buf_size_m1);
    if (n < 0) 
    {
        return -4;;
    }

    if (n < buf_size) {buffer[n] = 0;} else {buffer[last_index] = 0;}
    log("\t%s\n",buffer);

    log ("DATA");

    strcpy(buffer,"DATA\r\n");
    n = write(sockfd,buffer,strlen(buffer));
    if (n < 0) 
    {
        return -5;
    }
    
    n = read(sockfd, buffer, buf_size_m1);
    if (n < 0) 
    {
        return -4;
    }
    
    sprintf(buffer,"TO: %s\n", to.c_str());
    log("\n%s", buffer);
    n = write(sockfd,buffer,strlen(buffer));
    if (n < 0) 
    {
        return -5;
    }

    if (n < buf_size) {buffer[n] = 0;} else {buffer[last_index] = 0;}
    log("\t%s\n",buffer);

    sprintf(buffer,"Subject: %s\r\n", subject.c_str());

    n = write(sockfd, buffer, strlen(buffer));
    if (n < 0) 
    {
        return -5;
    }

    n = write(sockfd,body.c_str(), body.size());
    if (n < 0) 
    {
        return -5;
    }

    strcpy(buffer,"\n\n");

    n = write(sockfd,buffer,strlen(buffer));
    if (n < 0) 
    {
        return -5;
    }

    strcpy(buffer,".\n");

    n = write(sockfd,buffer,strlen(buffer));
    if (n < 0) 
    {
        return -5;
    }

    strcpy(buffer,"quit\n");

    n = write(sockfd,buffer,strlen(buffer));
    if (n < 0) 
    {
        return -5;
    }

    n = read(sockfd, buffer, buf_size_m1);
    if (n < 0) 
    {
        return -4;
    }

    if (n < buf_size) {buffer[n] = 0;} else {buffer[last_index] = 0;}
    log("%s\n",buffer);
    
    closeSocket(sockfd);

/*------------------------------*/

    return 0;
}


const char * Mail::error(ssize_t error_code) const
{
    if (error_code < 0) error_code *= -1;
    
    if ( error_code >= 0 && error_code < (ssize_t)(sizeof(error_text)/sizeof(error_text[0]) ) )
    {
        return error_text[error_code];
    }
    
    return 0;
}

} // smtp



#ifdef _TEST_MAIN_

int main (int argc, char * argv[])
{
    smtp::Mail mail;
    int ret = mail.init(
      std::string (argv[1])   // mailhost
    , atoi(argv[2])           // port
    , std::string (argv[3]  ) // from
    , std::string (argv[4]  ) // to
    );
    
    if (ret < 0)
    { 
        printf(smtp::error_text[ret * (-1) ]);
    }
    
    ret = mail.send
    (
      std::string (argv[5]  ) + "_1"// subject
    , std::string (argv[6]  ) + "_1"// body
    );

    if (ret < 0) printf("%s\n", mail.error(ret) );
    
    ret = mail.send
    (
      std::string (argv[5]  ) + "_2" // subject
    , std::string (argv[6]  ) + "_2" // body
    );

    if (ret < 0) printf("%s\n", mail.error(ret) );
    
    ret = mail.send
    (
      std::string (argv[5]  )  + "_3" // subject
    , std::string (argv[6]  )  + "_3"// body
    );

    if (ret < 0) printf("%s\n", mail.error(ret) );
    
    return 0;
}

#endif
