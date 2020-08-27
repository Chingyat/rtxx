#pragma once

#include <rtxx/clock.hpp>
#include <rtxx/config.hpp>

#if defined(RTXX_USE_POSIX)
#include <semaphore.h>
#elif defined(RTXX_USE_ALCHEMY)
#include <alchemy/sem.h>
#endif

namespace rtxx
{
class semaphore
{
public:
  using value_type = unsigned;

  /// Create a semaphore
  RTXX_DECL explicit semaphore(value_type init_value);

  /// Explicitly deleted copy constructor
  semaphore(const semaphore &other) = delete;

  /// Explicitly deleted move constructor
  semaphore(semaphore &&other) = delete;

  /// Destroy the semaphore
  RTXX_DECL ~semaphore();

  /// Explicitly deleted copy assignement operator
  semaphore &operator=(const semaphore &) = delete;

  /// Explicitly deleted move assignment operator
  semaphore &operator=(semaphore &&) = delete;

  /// Get the value of the semaphore
  [[nodiscard]] RTXX_DECL value_type get_value() const;

  /// Lock the semaphore
  RTXX_DECL void wait();

  /// Lock the semaphore
  [[nodiscard]] RTXX_DECL bool try_wait();

  template <typename Rep, typename Period>
  bool wait_for(chrono::duration<Rep, Period> const &rel_time);

  template <typename Clock, typename Duration>
  bool wait_until(chrono::time_point<Clock, Duration> const &abs_time);

  RTXX_DECL bool wait_until(const struct timespec *abs_timeout);

  /// Unlock the semaphore
  RTXX_DECL void post();

private:
#if defined(RTXX_USE_POSIX)
  sem_t sem_;

#elif defined(RTXX_USE_ALCHEMY)
  RT_SEM sem_;
#endif
};

template <typename Rep, typename Period>
bool semaphore::wait_for(chrono::duration<Rep, Period> const &rel_time)
{
#if defined(RTXX_USE_POSIX)
  return wait_until(realtime_clock::now() + rel_time);
#elif defined(RTXX_USE_ALCHEMY)
  return wait_until(monotonic_clock::now() + rel_time);
#endif
}

template <typename Clock, typename Duration>
bool semaphore::wait_until(chrono::time_point<Clock, Duration> const &abs_time)
{
#if defined(RTXX_USE_POSIX)
  static_assert(std::is_same<Clock, realtime_clock>::value,
                "currently only realtime clock is supported for posix");
#endif

#if defined(RTXX_USE_ALCHEMY)
  static_assert(std::is_same<Clock, monotonic_clock>::value,
                "currently only monotonic clock is supported for alchemy");
#endif
  const auto ts = detail::duration_to_timespec(abs_time.time_since_epoch());
  return wait_until(&ts);
}

} // namespace rtxx

#ifdef RTXX_HEADER_ONLY
#include <rtxx/impl/semaphore.ipp>
#endif