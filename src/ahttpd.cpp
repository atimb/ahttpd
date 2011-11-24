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
#include "Logger.h"
#include <sys/types.h>
#include <pwd.h>
#include <sys/socket.h>
#include <sys/errno.h>
#include <string>
#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>
#include <map>
#include <pthread.h>
#include <cstdlib>
#include <poll.h>
using namespace std;


struct stat_def
{
    long bytes_accepted, bytes_sent, active_conns, all_conns;
} stat;


struct clientConn
{
    ServerSocket *sock;
    map< string, string > headers;
    stringstream readBuf;
    bool keepAlive, busy, destroy;
};

struct conf
{
    string htdocs;
} config;


void find_and_replace( string &source, const string find, string replace ) {

    size_t j;
    for ( ; (j = source.find( find )) != string::npos ; ) {
        source.replace( j, find.length(), replace );
    }
}


void send_NotFound(clientConn *conn) {
    ostringstream header;
    header << "HTTP/1.0 200 OK\r\n";
    if (conn->keepAlive)
        header << "Connection: Keep-Alive\r\n";
    else
        header << "Connection: Close\r\n";
    header << "Content-length: 40\r\n\r\n404 - Not Found\r\nPowered by ahttpd v0.5b";
    *conn->sock << header.str();
    stat.bytes_sent += header.str().length();
    conn->busy = false;
    if (!conn->keepAlive) {
        delete conn->sock;
        conn->destroy = true;
    }
    conn->readBuf.str("");
}



void send_Stat(clientConn *conn) {
    ostringstream content;
    content << "<h2>ahttpd v0.5b</h2><br/>" << endl;
    content << "Actice connections: " << stat.active_conns << "<br/>";
    content << "<h3>Overall statistics:</h3>";
    content << "All connections: " << stat.all_conns << "<br/>";
    content << "Bytes received: " << stat.bytes_accepted << "<br/>";
    content << "Bytes sent: " << stat.bytes_sent << "<br/>";

    ostringstream header;
    header << "HTTP/1.0 200 OK\r\n";
    if (conn->keepAlive)
        header << "Connection: Keep-Alive\r\n";
    else
        header << "Connection: Close\r\n";
    header << "Content-Type: " << "text/html" << "\r\n";
    header << "Content-length: " << content.str().length() <<"\r\n";
    header << "\r\n";
    *conn->sock << header.str();
    *conn->sock << content.str();
    stat.bytes_sent += content.str().length() + header.str().length();
    conn->busy = false;
    if (!conn->keepAlive) {
        delete conn->sock;
        conn->destroy = true;
    }
    conn->readBuf.str("");
}


void *handleRequest(void *_conn) {

    clientConn *conn = (clientConn*)_conn;

    try
    {

            conn->busy = true;

            // parse request
            string command, url, proto;
            conn->readBuf.seekg(ios_base::beg);
            conn->readBuf >> command >> url >> proto;

            conn->keepAlive = true;

            // find out the extension of file requested!
            string reqfile = url;
            size_t found = reqfile.find("?");
            string querystring;
            if (found != string::npos) {
                querystring = reqfile.substr(found+1, reqfile.length()-found-1);
                reqfile = reqfile.substr(0, found);
            }

            // Self statistics
            if (reqfile == "/_stat_") {
                send_Stat(conn);
                pthread_exit(NULL);
            }


            find_and_replace(reqfile, "\\", "/");
            if (url.find("../") != string::npos) {
                send_NotFound(conn);
                pthread_exit(NULL);
            }

            string full_file = reqfile;

            found = reqfile.rfind("/");
            if (found != string::npos)
                reqfile = reqfile.substr(found+1, reqfile.length()-found-1);

            if (reqfile == "") {
              reqfile = "index.html";
              full_file = full_file+reqfile;
            }

            string extn = reqfile;
            found = extn.rfind(".");
            if (found != string::npos)
                extn = extn.substr(found+1, extn.length()-found-1);

            string path = config.htdocs + full_file;

            // Call php !
            if (extn == "php" || extn == "php5") {
                            
                pid_t nPid;
                int pipeto[2];      /* pipe to feed the exec'ed program input */
                int pipefrom[2];    /* pipe to get the exec'ed program output */

                if ( pipe( pipeto ) != 0 )
                {
                    perror( "pipe() to" );
                    exit(255);
                }
                if ( pipe( pipefrom ) != 0 )
                {
                    perror( "pipe() from" );
                    exit(255);
                }

                nPid = fork();
                if ( nPid < 0 )
                {
                    perror( "fork() 1" );
                    exit(255);
                }
                else if ( nPid == 0 )
                {
                    /* dup pipe read/write to stdin/stdout */
                    dup2( pipeto[0], STDIN_FILENO );
                    dup2( pipefrom[1], STDOUT_FILENO  );
                    /* close unnecessary pipe descriptors for a clean environment */
                    close( pipeto[0] );
                    close( pipeto[1] );
                    close( pipefrom[0] );
                    close( pipefrom[1] );
                    string param1 = "SCRIPT_FILENAME=" + path;
                    querystring = "QUERY_STRING=" + querystring;
                    const char* envp[4] = {"REQUEST_METHOD=GET", param1.c_str(), querystring.c_str(), NULL};
                    execle("/usr/bin/php5-cgi", "/usr/bin/php5-cgi", NULL, envp);
                    perror( "execle()" );
                    pthread_exit(NULL);
                }
                else
                {
                    pid_t nPid2;

                    /* Close unused pipe ends. This is especially important for the
                     * pipefrom[1] write descriptor, otherwise readFromPipe will never 
                     * get an EOF. */
                    close( pipeto[0] );
                    close( pipefrom[1] );

                    //nPid2 = fork();
                    //if ( nPid2 < 0 )
                    //{
                        //perror( "fork() 2" );
                        //exit(255);
                    //}
                    ///else if ( nPid2 == 0 )
                    {
                        /* Close pipe write descriptor, or we will never know when the
                         * writer process closes its end of the pipe and stops feeding the
                         * exec'ed program. */
                        close( pipeto[1] );

                        FILE *stream;
                        int ch;

                        if ( (stream = fdopen( pipefrom[0], "r" )) == NULL )
                        {
                            perror( "fdopen() r" );
                            exit(255);
                        }

                        stringstream ss;
                        string end = "\r\n\r\n";
                        char last[4] = {'a', 'a', 'a', 'a'};
                        long size = 0, header_l;
                        while ( (ch = getc( stream )) != EOF ) {
                            ++size;
                            ss.write( (char*)&ch, 1 );
                            last[0] = last[1];
                            last[1] = last[2];
                            last[2] = last[3];
                            last[3] = ch;
                            if (end == (string)last)
                                header_l = size;
                        }

                        fclose( stream );

                        ss.flush();

                        size = ss.str().length() - header_l;

                        stringstream header;
                        header << "HTTP/1.0 200 OK\r\n";
                        if (conn->keepAlive)
                            header << "Connection: Keep-Alive\r\n";
                        else
                            header << "Connection: Close\r\n";
                        header << "Content-Length:" << size << "\r\n";
                        //*conn->sock << header.str();

                        header << ss.str();
                        header.flush();


                        header.seekg(ios_base::beg);

                        char memblock[MAXSEND+5];
                        long trans = 0;
                        while (size > trans)
                        {
                            header.read(memblock, MAXSEND);
                            long read = header.gcount();
                            long tr = 0;
                            do {
                                conn->sock->send(memblock+tr, read-tr);
                                int result = conn->sock->getLastIOResult();
                                if (result == EINTR)
                                    continue;
                                if (result == 9999)
                                    tr += conn->sock->getTransferredBytes();
                            } while (read != tr);
                            trans += read;
                        }
                        stat.bytes_sent += size;

                    }
                }

            }


            string content_type;
            if (extn == "htm" || extn == "html") {
                content_type = "text/html";
            } else
            if (extn == "jpeg") {
                content_type = "image/jpeg";
            } else
            if (extn == "png") {
                content_type = "image/x-png";
            } else
            if (extn == "tiff") {
                content_type = "image/tiff";
            } else
                content_type = "text/plain";

              if (command == "GET")
              {

                  ifstream file(path.c_str(), ios::in|ios::binary|ios::ate);

                  if (file.is_open())
                  {
                    ifstream::pos_type size = file.tellg();
                    ifstream::pos_type trans = 0;
                    ostringstream header;
                    header << "HTTP/1.0 200 OK\r\n";
                    if (conn->keepAlive)
                        header << "Connection: Keep-Alive\r\n";
                    else
                        header << "Connection: Close\r\n";
                    header << "Content-Type: " << content_type << "\r\n";
                    header << "Content-Length:" << size << "\r\n";
                    header << "\r\n";
                    *conn->sock << header.str();
                    char memblock[MAXSEND+5];
                    file.seekg (0, ios::beg);
                    while (size > trans)
                    {
                        file.read (memblock, MAXSEND);
                        long read = file.gcount();
                        long tr = 0;
                        do {
                            conn->sock->send(memblock+tr, read-tr);
                            int result = conn->sock->getLastIOResult();
                            if (result == EINTR)
                                continue;
                            if (result == 9999)
                                tr += conn->sock->getTransferredBytes();
                        } while (read != tr);
                        trans += read;
                    }
                    file.close();
                    stat.bytes_sent += size;
                    
                  } else {
                     send_NotFound(conn);
                     pthread_exit(NULL);
                  }
              }

        // worker finishes
        conn->busy = false;
        conn->readBuf.str("");
        pthread_exit(NULL);

    }
    catch ( SocketException& e ) {
      Log->err << "Exception in respone worker thread:\n" << e.description() << endl;
      Log->flush();
      conn->busy = false;
      conn->readBuf.str("");
      pthread_exit(NULL);
    }

}



void cycleConns(vector < clientConn* > &conns, pollfd fds[], int &allconn) {

  try {

      vector< clientConn* >::iterator itConn;
      int connIndex = 1;

      for(itConn = conns.begin(); itConn != conns.end();) {

        clientConn *conn = *itConn;

        if (fds[connIndex].revents & POLLHUP) {
            delete conn->sock;
            itConn = conns.erase(itConn);
            for (int i=connIndex; i<allconn-1; ++i)
                fds[i] = fds[i+1];
            --allconn;
            continue;
        }

        if (!(fds[connIndex].revents & POLLIN)) {
            ++itConn;
            ++connIndex;
            continue;
        }

        // If it works in another thread
        if (conn->busy) {
            ++itConn;
            continue;
        }

        if (conn->destroy) {
            itConn = conns.erase(itConn);
            continue;
        }

        char *data = new char[MAXRECV+5];
        conn->sock->recv(data);
        int len = conn->sock->getTransferredBytes();

        if (len <= 0)
        {
            ++itConn;
            ++connIndex;
            continue;
        }

        // Add to read buffer if read was successful
        conn->readBuf.write(data, len);
        conn->readBuf.flush();
        stat.bytes_accepted += len;

        // handle request and generate response, if header is arrived
        conn->readBuf.seekg(ios_base::beg);
        pthread_t t;
        if (conn->readBuf.str().find("\r\n\r\n") != string::npos)
            int rc = pthread_create(&t, NULL, handleRequest, (void *)conn);

        ++itConn;
        ++connIndex;
     }
  } catch ( SocketException& e ) {
      Log->err << "Error in connection cycle.\n" << e.description() << endl;
      Log->flush();
  }

}


void writePidFile() {
    pid_t pid = getpid();
    ofstream file("ahttpd.pid");
    if (file.is_open()) {
        file << pid << endl;
    } else {
        Log->errlog << "Cannot write pid file 'ahttpd.pid'" << endl;
        Log->flush();
    }
    file.close();
}


int main ( int argc, char** argv )
{

  try
    {

    Log->out << "\nStartup ahttpd v0.5b...     ";

    string running_user="root";
    int port = 80;
    config.htdocs = "./htdocs";

    ifstream cfg("ahttpd.ini", ios::in);
    if (cfg.is_open()) {
        char buf[512];
        string s;
        while (!cfg.eof()) {
            cfg.getline(buf, 512);
            if (cfg.gcount() > 0) {
                if (buf[0] == '#')
                    continue;
                stringstream ss((string)buf, stringstream::in | stringstream::out);
                string key, value;
                ss >> key;
                if (key == "User")
                    ss >> running_user;
                if (key == "Port")
                    ss >> port;
                if (key == "Htdocs")
                    ss >> config.htdocs;
            }

        }
        cfg.close();
    }

    stat.bytes_sent = 0;
    stat.bytes_accepted = 0;
    stat.all_conns = 1;
    stat.active_conns = 0;

    vector < clientConn* > conns;
    // Create the socket
    ServerSocket server ( port );
    ServerSocket* new_client = new ServerSocket();

    Log->out << "                              [OK]\n";
    Log->out << "Dropping root privileges... ";

    struct passwd *pw = getpwnam(running_user.c_str());
    if (pw == NULL) {
        throw SocketException("Cannot find user");
    }
    setuid(pw->pw_uid);
    setgid(pw->pw_gid);

    Log->out << "                              [OK]\n";
    Log->out << "Entering daemon mode...     ";

    int i=fork();

    // fork error
    if (i<0) {
        throw SocketException("Cannot fork proccess. Maybe already running?!");
    }

    // parent exits
    if (i<=0)
    {
        Log->out << "                              [OK]\n\n";
    } else {
        exit(0);
    }

    writePidFile();

    pollfd fds[500];
    fds[0].fd = server.getFd();
    fds[0].events = POLLIN;
    fds[0].revents = 0;

    int allconn = 1;

    while ( true )
    {

        Log->flush();

        for (int i=0; i<allconn; ++i)
        {
            fds[i].revents = 0;
        }

        int status;
        do
        {
            status = poll(fds, allconn, -1);
        } while (status < 0  &&  errno == EAGAIN);

        if (status <= 0)
        {
          Log->errlog << "Error during poll(). errno = " << errno << endl;
          Log->flush();
          continue;
        }

        // accept incoming connection
        if (fds[0].revents & POLLIN)
        {
            int result = server.accept ( *new_client );
            if (result == -1) {
                Log->errlog << "Accept failed with errno: " << errno << endl;
                Log->flush();
            }
            clientConn* new_conn = new clientConn();
            new_conn->busy = false;
            new_conn->sock = new_client;
            new_conn->destroy = false;
            conns.push_back(new_conn);
            new_client = new ServerSocket();
            fds[allconn].fd = result;
            fds[allconn].events = POLLIN;
            fds[allconn].revents = 0;
            ++allconn;
            ++stat.all_conns;
        }

        stat.active_conns = allconn;

        // cycle through connections
        cycleConns(conns, fds, allconn);

    }
   } catch ( SocketException& e )
    {
      Log->err << "\nException in main:\n" << e.description() << endl;
      Log->flush();
    }

  return 0;
}
