/**
 *  ahttpd basic HTTP server
 *
 *  (C) 2008 Attila Incze <attila.incze@gmail.com>
 *  http://atimb.me
 *
 *  This work is licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported License. To view a copy of this license, visit
 *  http://creativecommons.org/licenses/by-nc-sa/3.0/ or send a letter to Creative Commons, 444 Castro Street, Suite 900, Mountain View, California, 94041, USA.
 * 
 */

#include "Socket.h"
#include "string.h"
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <iostream>


Socket::Socket() :
  m_sock ( -1 )
{

  memset ( &m_addr,
       0,
       sizeof ( m_addr ) );

}

Socket::~Socket()
{
  if ( is_valid() )
    ::close ( m_sock );
}

bool Socket::create()
{
  m_sock = socket ( AF_INET,
            SOCK_STREAM,
            0 );

  if ( ! is_valid() )
    return false;


  // TIME_WAIT - argh
  int on = 1;
  if ( setsockopt ( m_sock, SOL_SOCKET, SO_REUSEADDR, ( const char* ) &on, sizeof ( on ) ) == -1 )
    return false;


  return true;

}



bool Socket::bind ( const int port )
{

  if ( ! is_valid() )
    {
      return false;
    }



  m_addr.sin_family = AF_INET;
  m_addr.sin_addr.s_addr = INADDR_ANY;
  m_addr.sin_port = htons ( port );

  int bind_return = ::bind ( m_sock,
                 ( struct sockaddr * ) &m_addr,
                 sizeof ( m_addr ) );


  if ( bind_return == -1 )
    {
      return false;
    }

  return true;
}


bool Socket::listen() const
{
  if ( ! is_valid() )
    {
      return false;
    }

  int listen_return = ::listen ( m_sock, MAXCONNECTIONS );


  if ( listen_return == -1 )
    {
      return false;
    }

  return true;
}


int Socket::accept ( Socket& new_socket ) const
{
  int addr_length = sizeof ( m_addr );
  new_socket.m_sock = ::accept ( m_sock, ( sockaddr * ) &m_addr, ( socklen_t * ) &addr_length );
  return new_socket.m_sock;
}


int Socket::send ( const char* data, int size, int &sentbytes ) const
{
  // Replace SO_NOSIGPIPE with MSG_NOSIGNAL if not found
  int status = ::send ( m_sock, data, size, SO_NOSIGPIPE );
  sentbytes = status;
  if ( status == -1 )
    {
      return errno;
    }
  else
    {
      return 9999;
    }
}


int Socket::recv ( char* buf, int &readbytes ) const
{
  int status = readbytes = ::recv ( m_sock, buf, MAXRECV, 0 );

  if ( status == -1 )
    {
      return errno;
    }
  else if ( status == 0 )
    {
      return ECONNRESET;
    }
  else
    {
      return 9999;
    }
}



bool Socket::connect ( const std::string host, const int port )
{
  if ( ! is_valid() ) return false;

  m_addr.sin_family = AF_INET;
  m_addr.sin_port = htons ( port );

  int status = inet_pton ( AF_INET, host.c_str(), &m_addr.sin_addr );

  if ( errno == EAFNOSUPPORT ) return false;

  status = ::connect ( m_sock, ( sockaddr * ) &m_addr, sizeof ( m_addr ) );

  if ( status == 0 )
    return true;
  else
    return false;
}


void Socket::set_non_blocking ( const bool b )
{

  int opts;

  opts = fcntl ( m_sock,
         F_GETFL );

  if ( opts < 0 )
    {
      //exit(1);
      return;
    }

  if ( b )
    opts = ( opts | O_NONBLOCK );
  else
    opts = ( opts & ~O_NONBLOCK );

  fcntl ( m_sock,
      F_SETFL,opts );

}
