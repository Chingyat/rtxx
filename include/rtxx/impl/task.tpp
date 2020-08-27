#pragma once

#include <rtxx/task.hpp>

namespace rtxx
{
namespace this_task
{
template <typename Clock, typename Duration, typename Duration1>
void set_periodic(chrono::time_point<Clock, Duration> start, Duration1 interval)
{
  detail::current_task()->set_periodic(start, interval);
}

template <typename Clock, typename Duration, typename Duration1>
void set_periodic(chrono::time_point<Clock, Duration> start, Duration1 interval,
                  error_code &ec)
{
  detail::current_task()->set_periodic(start, interval, ec);
}

} // namespace this_task

template <typename... Initializers>
constexpr task::options::options(Initializers &&... init) noexcept
{
  (init(this), ...);
}

template <typename F> task::task(options opt, F &&f) : fn_(std::forward<F>(f))
{
  error_code ec;
  init(opt, ec);
  if (ec)
    throw system_error(ec, "task::init");
}

template <typename F> task::task(F &&f) : task(options{}, std::forward<F>(f)) {}

template <typename Duration> void task::set_periodic(Duration interval)
{
  error_code ec;
  set_periodic(monotonic_clock::now(), interval, ec);
  if (ec)
    throw system_error(ec, "task::set_periodic");
}

template <typename Clock, typename Duration, typename Duration1>
void task::set_periodic(chrono::time_point<Clock, Duration> start,
                        Duration1 interval)
{
  error_code ec;
  set_periodic(start, interval, ec);
  if (ec)
    throw system_error(ec, "task::set_periodic");
}

template <typename Clock, typename Duration, typename Duration1>
void task::set_periodic(chrono::time_point<Clock, Duration> start,
                        Duration1 interval, error_code &ec)
{
  const auto start_ns =
      chrono::duration_cast<chrono::nanoseconds>(start.time_since_epoch());
  const auto interval_ns = chrono::duration_cast<chrono::nanoseconds>(interval);

#if defined(RTXX_USE_POSIX)
  static_assert(
      std::is_same<Clock, monotonic_clock>() ||
          std::is_same<Clock, realtime_clock>(),
      "currently only monotonic clock and realtime clock are supported");
  const struct timespec start_ts = {
      .tv_sec = static_cast<time_t>(start_ns.count() / 1'000'000'000),
      .tv_nsec = static_cast<time_t>(start_ns.count() % 1'000'000'000)};
  const struct timespec interval_ts = {
      .tv_sec = static_cast<time_t>(interval_ns.count() / 1'000'000'000),
      .tv_nsec = static_cast<time_t>(interval_ns.count() % 1'000'000'000)};
  const struct itimerspec its = {.it_interval = interval_ts,
                                 .it_value = start_ts};

  set_periodic(Clock::clockid, &its, ec);
#elif defined(RTXX_USE_ALCHEMY)
  static_assert(std::is_same<Clock, monotonic_clock>(),
                "alchemy skin only supports monotonic clock");

  set_periodic(start_ns.count(), interval_ns.count(), ec);
#endif
}
} // namespace rtxx
