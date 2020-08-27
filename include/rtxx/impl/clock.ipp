#pragma once

#include <rtxx/clock.hpp>
#include <rtxx/error.hpp>

namespace rtxx
{
monotonic_clock::time_point monotonic_clock::now()
{
#if defined(RTXX_USE_POSIX)
  struct timespec ts;
  if (-1 == clock_gettime(clockid, &ts))
    throw system_error(errno, system_category(), "clock_gettime");

  rep r = ts.tv_nsec + static_cast<rep>(ts.tv_sec) * 1'000'000'000LL;
  return time_point(duration(r));
#elif defined(RTXX_USE_ALCHEMY)
  return time_point(duration(rt_timer_ticks2ns(rt_timer_read())));
#endif
}

#if defined(RTXX_USE_POSIX)
realtime_clock::time_point realtime_clock::now()
{
  struct timespec ts;
  if (-1 == clock_gettime(clockid, &ts))
    throw system_error(errno, system_category(), "clock_gettime");

  rep r = ts.tv_nsec + static_cast<rep>(ts.tv_sec) * 1'000'000'000LL;
  return time_point(duration(r));
}
#endif

} // namespace rtxx
