Chaudière
=========

**Chaudière** provides C++ infrastructure for commonly used functionality
such as sockets and multithreading. Conceptually, it's similar to
Apache's Portable Runtime (APR). It is common code that was extracted
from **misère** and **tonnerre**.

License
-------
BSD

Platforms/Compilers
-------------------
This project is implemented with C++11. It is tested with clang and
g++ compilers. It is also tested on Mac OS X, FreeBSD 10, PC-BSD 10,
and Ubuntu 14.04 Server.

Threading Options
-----------------
This project was initially coded to use Posix threads (pthreads).
Since C++11 has built-in support for threading, that was recently
added and is still being tweaked. Additionally, Apple's libdispatch
(foundation of Grand Central Dispatch [GCD]) is available on Mac
OS X, as well as on some other systems.

Socket Options
--------------
Chaudière supports kernel event mechanisms such as epoll (linux) and
kqueue (freebsd, osx), as well as a built-in socket server.

Status/Maturity
---------------
Parts of the code are very mature and well tested, and other parts
aren't. I wouldn't use this for anything important just yet. It'll
slowly get there.

C++ Standards
-------------
The code is somewhat C++11 compliant. The intention is to make it more
and more C++11 compliant over time.

Coding Style
------------
You may find my coding style slightly peculiar. It's my personal style
that I've adopted after a lot of experimentation about what works best
for me.

Meaning of Chaudière
--------------------
What does 'Chaudière' mean?  It's a French word that means kettle,
cooking pot, or cauldron.
