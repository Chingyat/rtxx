#pragma once

#include <rtxx/clock.hpp>
#include <rtxx/config.hpp>

#if defined(RTXX_USE_POSIX)
#include <pthread.h>
#elif defined(RTXX_USE_ALCHEMY)
#include <alchemy/mutex.h>
#endif

namespace rtxx
{
/// The mutex class.
/** @par Concepts
 *      @li TimedMutex
 */
class mutex
{
public:
  /// Deleted copy constructor
  mutex(const mutex &) = delete;

  /// Deleted copy assign operator
  mutex &operator=(const mutex &) = delete;

  /// Create a mutex
  RTXX_DECL mutex();

  /// Destroy a mutex
  RTXX_DECL ~mutex();

  /// Lock a mutex
  RTXX_DECL bool try_lock_for(chrono::nanoseconds duration);

#if defined(RTXX_USE_POSIX)
  /// Lock a mutex
  RTXX_DECL bool try_lock_until(realtime_clock::time_point time_limit);
#elif defined(RTXX_USE_ALCHEMY)
  RTXX_DECL bool try_lock_until(monotonic_clock::time_point time_limit);
#endif

  /// Lock a mutex
  RTXX_DECL bool try_lock();

  /// Lock a mutex
  RTXX_DECL void lock();

  /// Unlock a mutex
  RTXX_DECL void unlock();

#if defined(RTXX_USE_POSIX)
  using native_handle_type = pthread_mutex_t *;
#elif defined(RTXX_USE_ALCHEMY)
  using native_handle_type = RT_MUTEX *;
#endif

  RTXX_INLINE_DECL native_handle_type native_handle();

private:
#if defined(RTXX_USE_POSIX)
  pthread_mutex_t i_;
#elif defined(RTXX_USE_ALCHEMY)
  RT_MUTEX i_;
#endif
};

} // namespace rtxx

#include <rtxx/impl/mutex.hpp>
#ifdef RTXX_HEADER_ONLY
#include <rtxx/impl/mutex.ipp>
#endif
