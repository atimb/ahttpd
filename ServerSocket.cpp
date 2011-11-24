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

#include "ServerSocket.h"
#include "SocketException.h"
#include <iostream>

ServerSocket::ServerSocket ( int port )
{
  if ( ! Socket::create() )
    {
      throw SocketException ( "Could not create server socket." );
    }

  if ( ! Socket::bind ( port ) )
    {
      throw SocketException ( "Could not bind to port." );
    }

  if ( ! Socket::listen() )
    {
      throw SocketException ( "Could not listen to socket." );
    }

}

ServerSocket::~ServerSocket()
{
    //std::cout <<"destroying socket\n";
}


const ServerSocket& ServerSocket::operator << ( const std::string& s )
{
  m_lastIOResult = Socket::send( s.c_str(), s.size(), m_TransferredBytes );
//      throw SocketException ( "Could not write to socket." );
  return *this;

}


void ServerSocket::send( const char* data, int size ) {
  m_lastIOResult = Socket::send( data, size, m_TransferredBytes );
}


void ServerSocket::recv( char* data ) {
  m_lastIOResult = Socket::recv( data, m_TransferredBytes );
}


const ServerSocket& ServerSocket::operator >> ( std::string& s )
{
  char *buf = new char[MAXRECV];
  m_lastIOResult = Socket::recv( buf, m_TransferredBytes );
  s = buf;
//      throw SocketException ( "Could not read from socket." );
  return *this;
}

int ServerSocket::accept ( ServerSocket& sock )
{
  return Socket::accept ( sock );
      //throw SocketException ( "Could not accept socket." );
}

int ServerSocket::getLastIOResult() {
  return m_lastIOResult;
}

int ServerSocket::getTransferredBytes() {
    return m_TransferredBytes;
}

int ServerSocket::getFd() {
    return Socket::getFd();
}
