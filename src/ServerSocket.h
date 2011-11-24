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

#ifndef ServerSocket_class
#define ServerSocket_class

#include "Socket.h"


class ServerSocket : private Socket
{
 public:

  ServerSocket ( int port );
  ServerSocket (){ };
  virtual ~ServerSocket();

  const ServerSocket& operator << ( const std::string& );
  const ServerSocket& operator >> ( std::string& );

  void send( const char*, int);
  void recv( char*);

  int accept ( ServerSocket& );

  int getLastIOResult();
  int getTransferredBytes();

  int getFd();

private:

  int m_lastIOResult;
  int m_TransferredBytes;

};


#endif
