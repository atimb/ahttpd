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

#ifndef Logger_class
#define Logger_class

#include <iostream>
#include <sstream>
using namespace std;


class Logger {

    public:
        ostringstream debug, out, err, log, errlog;
        void flush();
        static Logger* getInstance();

    private:
        Logger(): m_debugmode(0) {}
        ~Logger() {}

    private:
        static Logger* m_instance;
        int m_debugmode;

};

#define Log Logger::getInstance()

#endif
