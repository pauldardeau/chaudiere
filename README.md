[![Build and test](https://github.com/pauldardeau/chaudiere/actions/workflows/build-and-test.yml/badge.svg)](https://github.com/pauldardeau/chaudiere/actions/workflows/build-and-test.yml)

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
This project is implemented with C++ (c++20). It is tested on
Linux (Ubuntu, Debian, Mint), FreeBSD, and
OpenBSD.

Threading Options
-----------------
This project was initially coded to use Posix threads (pthreads).
Apple's libdispatch is available on macOS and FreeBSD.

Socket Options
--------------
Chaudière supports kernel event mechanisms such as epoll (linux) and
kqueue (freebsd, macos), as well as a built-in socket server.

Meaning of Chaudière
--------------------
What does 'Chaudière' mean?  It's a French word that means kettle,
cooking pot, or cauldron.
