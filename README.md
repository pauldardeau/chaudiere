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
This project is implemented with C++ (pre c++11). It is tested with
g++ compiler on Linux (primarily Ubuntu 14.04 server), FreeBSD, and
OpenBSD.

Threading Options
-----------------
This project was initially coded to use Posix threads (pthreads).
Apple's libdispatch is available on Mac OSX and FreeBSD.

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
The code is pre-C++11. This is intentional to provide maximum
portability.

Coding Style
------------
You may find my coding style slightly peculiar. It's my personal style
that I've adopted after a lot of experimentation about what works best
for me.

Meaning of Chaudière
--------------------
What does 'Chaudière' mean?  It's a French word that means kettle,
cooking pot, or cauldron.
