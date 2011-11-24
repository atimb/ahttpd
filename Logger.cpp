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

#include "Logger.h"
#include <iostream>
#include <sstream>
#include <stdio.h>
using namespace std;


Logger* Logger::m_instance = NULL;


Logger* Logger::getInstance() {
    if (m_instance == NULL) {
        m_instance = new Logger();
    }
    return m_instance;
}


void Logger::flush() {
    if (m_debugmode) {
        cout << debug.str();
    }
    debug.str("");

    cout << out.str();
    ::flush(cout);
    out.str("");

    cerr << err.str();
    ::flush(cerr);
    err.str("");
}
