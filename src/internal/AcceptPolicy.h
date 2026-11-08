// Copyright Paul Dardeau, SwampBits LLC 2014
// BSD License

#ifndef CHAUDIERE_INTERNAL_ACCEPTPOLICY_H
#define CHAUDIERE_INTERNAL_ACCEPTPOLICY_H

// INTERNAL to chaudiere. This is not part of the public API: it lives in a
// subdirectory so that the install rule (which installs src/*.h only) does not
// install it, and no installed header includes it.
//
// What SocketServer::runSocketServer() does when ServerSocket::accept() fails
// (defect F-07). The loop used to retry every failure at once, so a failure that
// does not go away (a listening descriptor that has become invalid, a full
// descriptor table) made it fail millions of times a second at 100% CPU,
// silently. The decision is made here, in plain code with no system calls, so
// that tests can drive it directly; the loop only carries it out.
//
//   Class 1  RetryImmediately  the failure belongs to one connection or one
//            interruption, the listener is fine:  EINTR, ECONNABORTED, EPROTO and the
//            pending network errors Linux passes on from the new connection
//            (ENETDOWN, ENOPROTOOPT, EHOSTDOWN, ENONET, EHOSTUNREACH, ENETUNREACH).
//            Retried at once, but at most IMMEDIATE_RETRY_LIMIT times in a row.
//   Class 2  BackOff           resource exhaustion that ends when resources return
//            (EMFILE, ENFILE, ENOBUFS, ENOMEM, EAGAIN/EWOULDBLOCK), and every error
//            not listed anywhere (Class 4 - never a tight loop, never terminal).
//            Retried after a pause of 10, 20, 40, 80, 160, 250, 250, ... ms.
//   Class 3  Terminal          the listener, or the arguments of accept(), cannot be
//            used and no retry can change that:  EBADF, ENOTSOCK, EINVAL, EFAULT.
//            The loop ends and run() returns non-zero.
//
// Counter semantics (see AcceptFailureTracker):
//   - A failure of Class 1 is retried immediately while fewer than
//     IMMEDIATE_RETRY_LIMIT (100) immediate retries have been granted since the
//     last successful accept(). The 101st such failure, and every later one, is
//     handled like a Class 2 failure (pause, then retry).
//   - The pause starts at INITIAL_BACKOFF_MILLIS, doubles after every pause and is
//     capped at MAX_BACKOFF_MILLIS. It does not depend on which errno caused the
//     failure, so alternating errors do not restart it.
//   - Only a successful accept() resets the immediate-retry count, the pause and
//     the streak (and so the logging state). Nothing else does.
//
// The two seams at the bottom (AcceptLoopOps) exist so that tests can replace the
// clock and the pause; production always uses the real ones.

#include <errno.h>

#include <chrono>

namespace chaudiere
{
namespace detail
{

enum class AcceptErrorClass
{
   RetryImmediately,
   BackOff,
   Terminal
};

/**
 * @param error an errno value left by a failed accept()
 * @return what to do about it. Never depends on anything but the value, so it is
 * the same on every call.
 */
inline AcceptErrorClass classifyAcceptError(int error) {
   // Written as comparisons, not as a switch: on some platforms two of these
   // constants have the same value (EAGAIN and EWOULDBLOCK), which a switch
   // would reject as a duplicate label, and some of them do not exist on every
   // platform.
   if (error == EBADF || error == ENOTSOCK || error == EINVAL || error == EFAULT) {
      return AcceptErrorClass::Terminal;
   }

   if (error == EINTR || error == ECONNABORTED) {
      return AcceptErrorClass::RetryImmediately;
   }
#ifdef EPROTO
   if (error == EPROTO) {
      return AcceptErrorClass::RetryImmediately;
   }
#endif
#ifdef ENETDOWN
   if (error == ENETDOWN) {
      return AcceptErrorClass::RetryImmediately;
   }
#endif
#ifdef ENOPROTOOPT
   if (error == ENOPROTOOPT) {
      return AcceptErrorClass::RetryImmediately;
   }
#endif
#ifdef EHOSTDOWN
   if (error == EHOSTDOWN) {
      return AcceptErrorClass::RetryImmediately;
   }
#endif
#ifdef ENONET
   if (error == ENONET) {
      return AcceptErrorClass::RetryImmediately;
   }
#endif
#ifdef EHOSTUNREACH
   if (error == EHOSTUNREACH) {
      return AcceptErrorClass::RetryImmediately;
   }
#endif
#ifdef ENETUNREACH
   if (error == ENETUNREACH) {
      return AcceptErrorClass::RetryImmediately;
   }
#endif

   // EMFILE, ENFILE, ENOBUFS, ENOMEM, EAGAIN, EWOULDBLOCK, EOPNOTSUPP (recoverable
   // by Linux's documentation, but not something to retry in a tight loop) and
   // everything this function does not know
   return AcceptErrorClass::BackOff;
}

const int IMMEDIATE_RETRY_LIMIT = 100;
const int INITIAL_BACKOFF_MILLIS = 10;
const int MAX_BACKOFF_MILLIS = 250;

// the longest time between two repeated diagnostics of the same streak
const std::chrono::milliseconds REPEATED_DIAGNOSTIC_INTERVAL(5000);

enum class AcceptAction
{
   RetryNow,
   PauseThenRetry,
   Stop
};

enum class AcceptLog
{
   Nothing,
   StreakStarted,     // the first failure of a streak that is not retried at once
   StreakContinues,   // a repeat, at most one per REPEATED_DIAGNOSTIC_INTERVAL
   Terminal           // the listener is unusable; the loop ends
};

struct AcceptDecision
{
   AcceptAction action = AcceptAction::RetryNow;
   AcceptErrorClass errorClass = AcceptErrorClass::RetryImmediately;
   int pauseMillis = 0;          // for PauseThenRetry
   AcceptLog log = AcceptLog::Nothing;
   long suppressed = 0;          // StreakContinues: failures since the previous diagnostic that were not reported
   long failuresInStreak = 0;    // failures that were not retried at once since the last success
};

struct AcceptRecovery
{
   bool wasReported = false;     // a diagnostic was written for the streak that has just ended
   long failuresInStreak = 0;
};

/**
 * The state of one accept loop between successes. Plain data and arithmetic: no
 * system calls, no logging, no sleeping. One instance belongs to one loop.
 */
class AcceptFailureTracker
{
public:
   typedef std::chrono::steady_clock::time_point TimePoint;

   AcceptFailureTracker() :
      m_immediateRetries(0),
      m_nextPauseMillis(INITIAL_BACKOFF_MILLIS),
      m_failuresInStreak(0),
      m_suppressed(0),
      m_streakReported(false) {
   }

   /**
    * @param error the errno value of the failed accept()
    * @param now the current monotonic time (used to space repeated diagnostics)
    */
   AcceptDecision onFailure(int error, TimePoint now) {
      AcceptDecision decision;
      decision.errorClass = classifyAcceptError(error);

      if (decision.errorClass == AcceptErrorClass::Terminal) {
         decision.action = AcceptAction::Stop;
         decision.log = AcceptLog::Terminal;
         decision.failuresInStreak = m_failuresInStreak;
         return decision;
      }

      if (decision.errorClass == AcceptErrorClass::RetryImmediately &&
          m_immediateRetries < IMMEDIATE_RETRY_LIMIT) {
         ++m_immediateRetries;
         decision.action = AcceptAction::RetryNow;
         return decision;
      }

      // the pause
      decision.action = AcceptAction::PauseThenRetry;
      decision.pauseMillis = m_nextPauseMillis;
      m_nextPauseMillis = (m_nextPauseMillis > MAX_BACKOFF_MILLIS / 2) ?
                          MAX_BACKOFF_MILLIS : (m_nextPauseMillis * 2);
      ++m_failuresInStreak;
      decision.failuresInStreak = m_failuresInStreak;

      if (!m_streakReported) {
         m_streakReported = true;
         m_lastReport = now;
         m_suppressed = 0;
         decision.log = AcceptLog::StreakStarted;
      } else if (now - m_lastReport >= REPEATED_DIAGNOSTIC_INTERVAL) {
         m_lastReport = now;
         decision.log = AcceptLog::StreakContinues;
         decision.suppressed = m_suppressed;
         m_suppressed = 0;
      } else {
         ++m_suppressed;
      }
      return decision;
   }

   /**
    * A successful accept(): everything starts again.
    */
   AcceptRecovery onSuccess() {
      AcceptRecovery recovery;
      recovery.wasReported = m_streakReported;
      recovery.failuresInStreak = m_failuresInStreak;
      m_immediateRetries = 0;
      m_nextPauseMillis = INITIAL_BACKOFF_MILLIS;
      m_failuresInStreak = 0;
      m_suppressed = 0;
      m_streakReported = false;
      return recovery;
   }

private:
   int m_immediateRetries;
   int m_nextPauseMillis;
   long m_failuresInStreak;
   long m_suppressed;
   bool m_streakReported;
   TimePoint m_lastReport;
};

/**
 * The clock and the pause of the accept loop. Production always uses
 * systemAcceptLoopOps(); only tests supply other values.
 */
struct AcceptLoopOps
{
   std::chrono::steady_clock::time_point (*now)();
   void (*sleepMillis)(int millis);
};

/**
 * @return the real steady clock and a real sleep
 */
const AcceptLoopOps& systemAcceptLoopOps();

/**
 * @return the ops the accept loop uses: systemAcceptLoopOps() unless a test has
 * replaced them
 */
const AcceptLoopOps& acceptLoopOps();

/**
 * Test hook: makes the accept loop use ops from now on. nullptr restores the real
 * ones. The ops must outlive their use.
 * @return the ops that were in use before
 */
const AcceptLoopOps* setAcceptLoopOps(const AcceptLoopOps* ops);

}
}

#endif
