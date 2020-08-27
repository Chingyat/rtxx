#pragma once

#include <mutex>
#include <rtxx/clock.hpp>
#include <rtxx/config.hpp>
#include <rtxx/error.hpp>
#if defined(RTXX_USE_POSIX)
#include <pthread.h>
#elif defined(RTXX_USE_ALCHEMY)
#include <alchemy/cond.h>
#endif

namespace rtxx
{
class mutex;

/// The condition_variable class
class condition_variable
{
public:
  /// Create a condition variable
  RTXX_DECL condition_variable();

  /// Destroy a condition variable
  RTXX_DECL ~condition_variable();

  /// Deleted copy constructor
  condition_variable(const condition_variable &) = delete;

  /// Deleted copy assigment operator
  condition_variable &operator=(const condition_variable &) = delete;

  /// Wake one task waiting on this condition variable
  RTXX_DECL void notify_one();

  /// Wake all tasks waiting on this condition variable
  RTXX_DECL void notify_all();

  /// blocks the current thread until the condition variable is woken up
  RTXX_DECL bool wait(std::unique_lock<mutex> &lock);

  /// blocks the current thread until the condition variable is woken up or
  /// after the specified timeout duration
  template <typename Rep, typename Period>
  bool wait_for(std::unique_lock<mutex> &lock,
                chrono::duration<Rep, Period> const &rel_time);

  /// blocks the current thread until the condition variable is woken up or
  /// until specified time point has been reached
  template <typename Clock, typename Duration>
  bool wait_until(std::unique_lock<mutex> &lock,
                  chrono::time_point<Clock, Duration> const &abs_time);

  RTXX_DECL bool wait_until(std::unique_lock<mutex> &lock,
                            const struct timespec *abs_time);

#if defined(RTXX_USE_POSIX)
  using native_handle_type = pthread_cond_t *;
#elif defined(RTXX_USE_ALCHEMY)
  using native_handle_type = RT_COND *;
#endif

  ///  returns the native handle
  RTXX_INLINE_DECL native_handle_type native_handle() const;

private:
#if defined(RTXX_USE_POSIX)
  pthread_cond_t c_;
#elif defined(RTXX_USE_ALCHEMY)
  RT_COND c_;
#endif
};
} // namespace rtxx

#include <rtxx/impl/condition_variable.tpp>

#ifdef RTXX_HEADER_ONLY
#include <rtxx/impl/condition_variable.ipp>
#endif