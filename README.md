# ahttpd Basic HTTP Server with php-cgi Support

*Not even nearly RFC compatible.*

Implemented in C++, can be compiled on unix like systems.

## Features

* Supported methods: GET
* Supported response codes: 200, 404
* Multi-threaded
* Bind as root, run as user
* Config file
* Web statistics
* php support via php-cgi invocation

## Howto

Build with `make`.

Config through `ahttpd.ini`.

Run with `./ahttpd`.

## Docs

There is a developer and a user documentation (unfortunately only hungarian) under `docs`.

## License

(C) 2008 Attila Incze <attila.incze@gmail.com>

http://atimb.me

This work is licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported License. To view a copy of this license, visit
http://creativecommons.org/licenses/by-nc-sa/3.0/ or send a letter to Creative Commons, 444 Castro Street, Suite 900, Mountain View, California, 94041, USA.