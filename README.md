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

Dependencies
------------
Poivre - C++ unit test framework, included as a git submodule
(`poivre/`). `tests/` links the actual poivre library; `src/` itself
only needs one poivre header at compile time (`AutoPointer.h`, used
internally by `SocketServer.cpp`), not the library. Clone with
`--recurse-submodules`, or after cloning:

```bash
git submodule update --init --recursive
```

Building
--------
```bash
make -C src    # builds libchaudiere.so
make -C tests  # builds test_chaudiere (needs the poivre submodule)
```

Your own programs need `-I` for `src/`, and link against
`src/libchaudiere.so` (see `tests/Makefile` for the exact flags).

### Building with CMake

A `CMakeLists.txt` is also provided, mainly for consumers who want to pull chaudière in as
a proper dependency rather than hand-writing `-I` flags:

```bash
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure   # builds and runs test_chaudiere
```

For an ASan/UBSan or TSan build, set the standard CMake sanitizer flags globally (this
also covers poivre, pulled in via `add_subdirectory()`, with no extra wiring needed):

```bash
cmake -S . -B build-asan \
   -DCMAKE_CXX_FLAGS="-fsanitize=address,undefined -fno-omit-frame-pointer -g -O0" \
   -DCMAKE_EXE_LINKER_FLAGS="-fsanitize=address,undefined" \
   -DCMAKE_SHARED_LINKER_FLAGS="-fsanitize=address,undefined"
cmake --build build-asan
ASAN_OPTIONS=detect_leaks=0 ctest --test-dir build-asan --output-on-failure
```

Swap `address,undefined` for `thread` for a TSan build (see `.github/workflows/build-and-test.yml`
for the exact `cmake-asan-ubsan`/`cmake-tsan` CI jobs, including the `setarch -R` TSan needs
under some kernels/ASLR configurations).

To depend on chaudière from another CMake project - vendored as a git submodule, the same
way misère/tonnerre/chapeau already vendor it:

```cmake
add_subdirectory(chaudiere)
target_link_libraries(my_target PRIVATE chaudiere)
```

That's it - no `-I` needed; the include directory and C++20 requirement both propagate
automatically. `add_subdirectory()` also skips building chaudière's own ~40-file test suite
by default (only relevant when chaudière itself is the top-level project), so it doesn't pull
poivre in or add an extra test binary to your build.

The Makefile isn't going anywhere - both build systems compile the same sources and stay in
sync with each other, so use whichever fits your project.

Groups of Functionality
------------------------
Chaudière is organized around a handful of areas. Each group below
lists the concrete classes you'd actually use; abstract interfaces
(`Mutex`, `ConditionVariable`, `Thread`, `Logger`, `ThreadPoolDispatcher`,
`SocketServiceHandler`, and so on) exist so a group can have more than
one backend implementation.

### Threading & Concurrency

- **`Runnable`** — the unit of work interface (a `run()` method plus
  completion-notification hooks); everything else in this group
  ultimately executes one of these.
- **`Thread`** (interface), **`PthreadsThread`**, **`StdThread`** — a
  thread that either runs a `Runnable` you hand it, or runs its own
  overridden `run()`. Two backends: POSIX threads and `std::thread`.
- **`Mutex`** (interface), **`PthreadsMutex`**, **`StdMutex`**, and
  **`MutexLock`** — the RAII lock wrapper used everywhere a `Mutex` is
  held.
- **`ConditionVariable`** (interface), **`PthreadsConditionVariable`**,
  **`StdConditionVariable`** — the usual "wait for a condition while a
  mutex is held" primitive, paired with a matching `Mutex` backend.
- **`ThreadingFactory`** (interface), **`PthreadsThreadingFactory`**,
  **`StdThreadingFactory`** — a single factory for creating a matched
  set of `Thread`/`Mutex`/`ConditionVariable`/thread-pool instances
  from one backend, plus a process-wide default you can override with
  `ThreadingFactory::setThreadingFactory()`.
- **`ThreadPool`**, **`ThreadPoolQueue`**, **`ThreadPoolWorker`** — a
  fixed-size pool of worker threads pulling `Runnable`s off a shared
  queue. This is what `SocketServer` uses to dispatch incoming
  requests when threading is enabled.

### Networking

- **`Socket`** — a connected TCP socket, wrapping both the
  client-connect path (`Socket(host, port)`) and the
  already-have-an-fd path (`Socket(fd)`, e.g. from `accept()`).
  Supports raw `send`/`receive`, buffered `read`/`readLine`/
  `readSocket`, and an optional 2-byte length-prefixed payload mode
  (`setIncludeMessageSize`).
- **`ServerSocket`** — binds, listens, and `accept()`s, handing back
  `Socket` instances for each connection.
- **`SocketRequest`**, **`RequestHandler`**, **`SocketServiceHandler`**
  — the pieces a server wires together per connection: a
  `SocketRequest` wraps the accepted socket, a `SocketServiceHandler`
  decides what to do with it, and a `RequestHandler` is the
  `Runnable` that actually does the work (and owns the socket's
  lifetime).
- **`SocketServer`** — the base class for building a server: reads an
  `.ini` config section for its port/threading model, runs an
  accept loop (`runSocketServer()`), and dispatches each connection
  either synchronously or through a `ThreadPool`, depending on
  configuration. Subclass it and implement `handlerForSocket()` to
  plug in your own protocol (see tonnerre's `MessagingServer` for a
  real example).
- **`KernelEventServer`** (interface), **`EpollServer`** (Linux),
  **`KqueueServer`** (BSD/macOS) — an alternative to the accept-loop
  model: register file descriptors with the OS's kernel event
  mechanism and get notified when they're ready, for handling many
  connections without a thread per connection.
- **`ServiceInfo`** — a small value type (host, port, persistent-
  connection flag) for describing where a service lives; used by
  tonnerre's service registry.

### Data Structures & Parsing

- **`ByteBuffer`**, **`CharBuffer`** — low-level, resizable memory
  buffers (`ByteBuffer` owns/copies its data with value semantics;
  `CharBuffer` is the lighter-weight buffer `Socket` and `Message`
  parsing use internally).
- **`KeyValuePairs`** — an ordered string-to-string map, used
  throughout the library (parsed `.ini` sections, message headers,
  command-line options all end up as one of these).
- **`StrUtils`** — string utilities: parsing (`parseInt`, `parseLong`,
  ...), formatting, `startsWith`/`endsWith`/`containsString`,
  `split`, `padLeft`/`padRight`, `replaceAll`, and more.
- **`StringTokenizer`** — iterates the tokens of a string given a set
  of delimiter characters.
- **`IniReader`** (implements **`SectionedConfigDataSource`**) — reads
  and parses `.ini`-style configuration files into `KeyValuePairs`
  per section.

### System & Logging

- **`Logger`** (interface), **`FileLogger`**, **`StdLogger`** — leveled
  logging (`Critical` through `Verbose`) behind a process-wide
  singleton (`Logger::setLogger()` / `Logger::critical()` /
  `Logger::info()` / ...). `FileLogger` writes to a file; `StdLogger`
  writes to stdout and also tracks per-class instance-lifecycle
  counts and arbitrary named occurrence counts.
- **`OSUtils`** — OS-level utilities: filesystem (paths, directory
  listing, file size/rename/delete, CRC-32), platform/OS identification,
  host name/user, load averages, and CPU/memory info (coverage of the
  last group varies by platform).
- **`DateTime`** — date/time value type with unix-time conversion,
  formatting, and comparison.
- **`SystemInfo`**, **`SystemStats`** — thin wrappers around `uname()`
  and system load/uptime/process-count queries.
- **`DynamicLibrary`** — a `dlopen`/`dlsym`/`dlclose` wrapper for
  loading shared libraries and resolving symbols at runtime.
- **`OptionParser`** — command-line argument parsing: register boolean
  flags and string options (with defaults), then parse `argv`.
- **`Utils`** — small file helpers (`WriteFile`/`ReadFile` against a
  `ByteBuffer`) and console output (`Write`/`WriteLine`).

### Exceptions

- **`BasicException`** — chaudière's base exception type. It derives
  from poivre's `BasicException` so it's catchable by poivre's test
  framework (`requireException()`), while remaining chaudière's own
  type for existing production code.
- **`InvalidKeyException`**, **`NumberFormatException`** — thrown by
  `KeyValuePairs::getValue()` for a missing key, and by numeric
  parsing failures, respectively.

Quick Start
-----------
A minimal look at four of the groups above. Every snippet here was
compiled and run against this library while writing this README.

**Threading** — run work on a pool of worker threads:

```cpp
#include "ThreadPool.h"
#include "Runnable.h"

using namespace chaudiere;

class GreetingTask : public Runnable {
public:
   void run() override {
      // work happens here, on a pool worker thread
   }
};

ThreadPool pool(4);               // starts 4 worker threads immediately
pool.addRequest(new GreetingTask);
// ...
pool.stop();
```

**Networking** — a bare client/server socket pair:

```cpp
#include "ServerSocket.h"
#include "Socket.h"

using namespace chaudiere;

ServerSocket serverListener(9000);

Socket clientSocket("127.0.0.1", 9000);
Socket* serverSideSocket = serverListener.accept();

clientSocket.write(std::string("hello from client"));

char buffer[64] = {0};
serverSideSocket->readSocket(buffer, 17);  // "hello from client" is 17 bytes
// buffer now holds "hello from client"

delete serverSideSocket;
```

**Configuration** — read an `.ini` file:

```cpp
#include "IniReader.h"
#include "KeyValuePairs.h"

using namespace chaudiere;

// [server]
// host = 127.0.0.1
// port = 9000
IniReader reader("myapp.ini");
KeyValuePairs settings;
reader.readSection("server", settings);

const std::string& host = settings.getValue("host");
const std::string& port = settings.getValue("port");
```

**Logging** — install a logger and log through the static facade:

```cpp
#include "Logger.h"
#include "FileLogger.h"

using namespace chaudiere;

Logger::setLogger(new FileLogger("myapp.log", Debug));
Logger::info("starting up");
Logger::critical("something went very wrong");
```

Testing
-------
Chaudière has a large test suite (`tests/`, built on poivre's
`TestSuite`/`TEST_CASE` framework) covering essentially every concrete
class above with real threads, real loopback sockets, and real files
rather than mocks. Build and run it with:

```bash
make -C src
make -C tests
LD_LIBRARY_PATH=src tests/test_chaudiere
```

Platforms/Compilers
-------------------
This project is implemented with C++ (c++20). It is tested on
Linux (Ubuntu, Debian, Mint), FreeBSD, and
OpenBSD.

Threading Options
-----------------
This project was initially coded to use Posix threads (pthreads), and
that remains the default (`PthreadsThreadingFactory`). A C++11
`std::thread`-based backend (`StdThreadingFactory`) is also available.
Apple's libdispatch is available on macOS and FreeBSD.

Socket Options
--------------
Chaudière supports kernel event mechanisms such as epoll (linux) and
kqueue (freebsd, macos), as well as a built-in socket server
(`SocketServer`).

Meaning of Chaudière
--------------------
What does 'Chaudière' mean?  It's a French word that means kettle,
cooking pot, or cauldron.
