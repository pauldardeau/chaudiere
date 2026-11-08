// Copyright Paul Dardeau, SwampBits LLC 2014
// BSD License

#ifndef CHAUDIERE_INTERNAL_SOCKETIO_H
#define CHAUDIERE_INTERNAL_SOCKETIO_H

// INTERNAL to chaudiere. This is not part of the public API: it lives in a
// subdirectory so that the install rule (which installs src/*.h only) does not
// install it, and no installed header includes it.
//
// It exists for two reasons:
//   1. it keeps the POSIX system-call details of Socket::readSocket(buffer,
//      count, timeoutMillis) (poll(), recv(MSG_DONTWAIT), errno) out of the
//      public Socket.h, so the public header needs no platform includes;
//   2. it lets the tests substitute poll()/recv(), which is the only way to
//      make "EINTR from recv()" and "recv() reports EAGAIN although poll()
//      reported the descriptor readable" happen on demand, rather than hoping
//      the kernel produces them.

#include <poll.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <chrono>

namespace chaudiere
{
namespace detail
{

/**
 * The two system calls the bounded read is built from. Production code always
 * uses systemSocketIoOps(); only tests supply other values.
 */
struct SocketIoOps
{
   int (*poll)(struct pollfd* fds, nfds_t nfds, int timeoutMillis);
   ssize_t (*recv)(int fd, void* buffer, size_t length, int flags);
};

/**
 * @return the real ::poll() and ::recv()
 */
const SocketIoOps& systemSocketIoOps();

/**
 * Reads bytes from a descriptor until bytesToRead have arrived, the time budget
 * is used up, the peer closes or an error occurs. This is the whole of the
 * descriptor-level work of Socket::readSocket(buffer, count, timeoutMillis).
 *
 * The descriptor is never put into non-blocking mode and none of its options
 * are changed: readiness is awaited with poll(), and the data is taken with
 * recv(MSG_DONTWAIT), so the recv() cannot block even if poll() reported
 * readiness that is no longer true.
 *
 * @param fd the descriptor
 * @param buffer destination, at least bytesToRead bytes
 * @param bytesToRead number of bytes wanted, must be greater than 0
 * @param timeoutMillis total budget, measured from start. Less than 0: unlimited.
 * 0: only bytes that are available without waiting. Greater than 0: the read
 * gives up once this many milliseconds have elapsed since start in total,
 * however much data keeps arriving.
 * @param start when the budget began
 * @param ops the system calls to use
 * @return the number of bytes read (bytesToRead on success, less if the budget
 * ran out, the peer closed or an error occurred, 0 if none were read)
 */
int boundedReadFromDescriptor(int fd,
                              char* buffer,
                              int bytesToRead,
                              int timeoutMillis,
                              std::chrono::steady_clock::time_point start,
                              const SocketIoOps& ops);

/**
 * Waits for a connection attempt that connect() reported as interrupted (EINTR)
 * to end, and says how it ended. The attempt is not over when connect() fails
 * with EINTR: it goes on in the kernel, so the caller keeps the descriptor and
 * waits for it here instead of treating EINTR as a failure or calling connect()
 * again.
 *
 * Waits with poll(POLLOUT) and no time limit (like the blocking connect() it
 * stands in for), resuming after every EINTR. When poll() reports an event
 * the outcome is read with getsockopt(SO_ERROR): writability alone is not
 * success and POLLERR / POLLHUP alone are not failure.
 *
 * @param fd the descriptor on which connect() returned EINTR
 * @param ops the system calls to use (poll() is the one used)
 * @return 0 if the connection is established, otherwise the error number of the
 * failure: the SO_ERROR value of the attempt, EBADF if poll() reports the
 * descriptor invalid, the errno of a failed poll() or getsockopt(), or ETIMEDOUT
 * after repeated poll() results that report nothing at all
 */
int awaitConnectCompletion(int fd, const SocketIoOps& ops);

/**
 * The system calls Socket::open() waits for an interrupted connection with:
 * systemSocketIoOps() unless a test has replaced them.
 */
const SocketIoOps& connectIoOps();

/**
 * Test hook: makes Socket::open() use ops (only poll() is used) from now on.
 * nullptr restores the real calls. The ops must outlive their use.
 * @return the ops that were in use before
 */
const SocketIoOps* setConnectIoOps(const SocketIoOps* ops);

/**
 * The system call Socket::sendPayload() and Socket::sendPayloadSize() transmit with. A
 * separate structure from SocketIoOps (which stays as it is) so that the tests that
 * build a SocketIoOps are not affected.
 */
struct SendIoOps
{
   ssize_t (*send)(int fd, const void* buffer, size_t length, int flags);
};

/**
 * @return the real ::send()
 */
const SendIoOps& systemSendIoOps();

/**
 * The send the Socket send paths use: systemSendIoOps() unless a test has replaced it.
 */
const SendIoOps& sendIoOps();

/**
 * Test hook: makes the Socket send paths use ops from now on. nullptr restores the real
 * call. The ops must outlive their use.
 * @return the ops that were in use before
 */
const SendIoOps* setSendIoOps(const SendIoOps* ops);

/**
 * The system call ServerSocket::accept() takes a connection with. A separate structure, for the
 * same reason as SendIoOps. Tests replace it to make accept() fail on demand with an error the
 * kernel will not produce when asked (see AcceptPolicy.h for what the accept loop does with them).
 */
struct AcceptIoOps
{
   int (*accept)(int fd, struct sockaddr* address, socklen_t* addressLength);
};

/**
 * @return the real ::accept()
 */
const AcceptIoOps& systemAcceptIoOps();

/**
 * The accept ServerSocket::accept() uses: systemAcceptIoOps() unless a test has replaced it.
 */
const AcceptIoOps& acceptIoOps();

/**
 * Test hook: makes ServerSocket::accept() use ops from now on. nullptr restores the real call.
 * The ops must outlive their use.
 * @return the ops that were in use before
 */
const AcceptIoOps* setAcceptIoOps(const AcceptIoOps* ops);

}
}

#endif
