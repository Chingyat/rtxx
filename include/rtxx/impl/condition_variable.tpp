#pragma once

#include <rtxx/clock.hpp>
#include <rtxx/condition_variable.hpp>
#include <type_traits>

namespace rtxx
{
template <typename Rep, typename Period>
bool condition_variable::wait_for(std::unique_lock<mutex> &lock,
                                  chrono::duration<Rep, Period> const &rel_time)
{
#ifdef RTXX_USE_POSIX
  return wait_until(lock, realtime_clock::now() + rel_time);
#elif defined(RTXX_USE_ALCHEMY)
  return wait_until(lock, monotonic_clock::now() + rel_time);
#endif
}

#ifdef RTXX_USE_POSIX
template <typename Clock, typename Duration>
bool condition_variable::wait_until(
    std::unique_lock<mutex> &lock,
    chrono::time_point<Clock, Duration> const &abs_time)
{
  static_assert(std::is_same<Clock, realtime_clock>(),
                "Currently only realtime_clock is supported");
  const auto ts = detail::duration_to_timespec(abs_time.time_since_epoch());
  return wait_until(lock, &ts);
}
#elif defined(RTXX_USE_ALCHEMY)
template <typename Clock, typename Duration>
bool condition_variable::wait_until(
    std::unique_lock<mutex> &lock,
    chrono::time_point<Clock, Duration> const &abs_time)
{
  static_assert(std::is_same<Clock, monotonic_clock>(),
                "Currently only monotonic clock is supported");
  const auto ts = detail::duration_to_timespec(abs_time.time_since_epoch());
  return wait_until(lock, &ts);
}
#endif

} // namespace rtxx