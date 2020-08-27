#pragma once

#include <chrono>

#include "rtxx/config.hpp"

#if defined(RTXX_USE_POSIX)
#include <time.h>
#elif defined(RTXX_USE_ALCHEMY)
#include <alchemy/timer.h>
#endif

namespace rtxx
{
namespace chrono = std::chrono;

namespace detail
{
constexpr auto duration_to_timespec(chrono::nanoseconds n)
{
  const struct timespec ts = {
      .tv_sec = static_cast<time_t>(n.count() / 1'000'000'000),
      .tv_nsec = static_cast<long>(n.count() % 1'000'000'000),
  };
  return ts;
};
} // namespace detail

/// C++ Wrapper of CLOCK_MONOTONIC
/** @par Concepts
 *      @li Clock
 */
class monotonic_clock
{
public:
  using duration = chrono::nanoseconds;
  using rep = duration::rep;
  using period = duration::period;
  using time_point = chrono::time_point<monotonic_clock>;

  /// The posix clock ID of monotonic clock
  static const clockid_t clockid{CLOCK_MONOTONIC};

  static const bool is_steady{false};

  /// Get the current clock value
  RTXX_DECL static time_point now();
};

#if defined(RTXX_USE_POSIX)
/// C++ Wrapper of CLOCK_REALTIME
/** @par Concepts
 *      @li Clock
 */
class realtime_clock
{
public:
  using duration = chrono::nanoseconds;
  using rep = duration::rep;
  using period = duration::period;
  using time_point = chrono::time_point<realtime_clock>;
  static const clockid_t clockid{CLOCK_REALTIME};

  static const bool is_steady{true};

  /// Get the current clock value
  RTXX_DECL static time_point now();
};
#endif

} // namespace rtxx

#ifdef RTXX_HEADER_ONLY
#include <rtxx/impl/clock.ipp>
#endif
