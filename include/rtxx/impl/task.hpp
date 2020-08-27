#pragma once

#include <cassert>
#include <rtxx/clock.hpp>
#include <rtxx/task.hpp>

namespace rtxx
{
inline void task::join()
{
  error_code ec;
  join(ec);
  if (ec)
    throw system_error(ec, "task::join");
}

inline bool task::joinable() const
{
#if defined(RTXX_USE_POSIX)
  return !!h_;
#elif defined(RTXX_USE_ALCHEMY)
  return joinable_;
#endif
}

namespace this_task
{

unsigned wait_period(error_code &ec)
{
  return detail::current_task()->wait_period(ec);
}

unsigned wait_period() { return detail::current_task()->wait_period(); }
} // namespace this_task

constexpr auto priority(int value)
{
  assert(value >= 0 && value < 100);

  return [value](task::options *opt) RTXX_CONSTEXPR_LAMBDA {
    assert(opt);
    opt->priority = value;
  };
}

constexpr auto name(const char *name)
{
  return [name](task::options *opt) RTXX_CONSTEXPR_LAMBDA {
    assert(opt);
    opt->name = name;
  };
}

constexpr auto stack_size(int size)
{
  return [size](task::options *opt) RTXX_CONSTEXPR_LAMBDA {
    assert(opt);
    opt->stack_size = size;
  };
}

constexpr auto cpu_set(const cpu_set_t *set)
{
  return [set](task::options *opt) RTXX_CONSTEXPR_LAMBDA {
    assert(opt);
    opt->cpu_set = set;
  };
}

constexpr auto schedpolicy(int sched)
{
  return [sched](task::options *opt) RTXX_CONSTEXPR_LAMBDA {
    assert(opt);
    opt->schedpolicy = sched;
  };
}

task::native_handle_type task::native_handle()
{
#if defined(RTXX_USE_POSIX)
  return h_;
#elif defined(RTXX_USE_ALCHEMY)
  return &task_;
#endif
}

} // namespace rtxx
