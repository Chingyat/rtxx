#pragma once

#include <sys/timerfd.h>
#include <unistd.h>

#include <cassert>
#include <csignal>
#include <cstring>
#include <rtxx/clock.hpp>
#include <rtxx/task.hpp>

namespace rtxx
{

enum task_flag : unsigned long
{
  task_auto_join = 0x0001,
};

namespace this_task
{
namespace detail
{
thread_local task *current_task;
}

void yield(error_code &ec)
{
  int err;
#if defined(RTXX_USE_POSIX)
  err = pthread_yield();
  if (err != 0)
  {
    ec.assign(err, system_category());
  }
#elif defined(RTXX_USE_ALCHEMY)
  err = rt_task_yield();
  if (err != 0)
    return ec.assign(-err, system_category());
#else
#error "not implemented"
#endif
}

void yield()
{
  error_code ec;
  yield(ec);

  if (ec)
    throw system_error(ec, "this_task::yield");
}

} // namespace this_task

/// @FIXME this function is not correctly implemented
task::~task()
{
#if defined(RTXX_USE_POSIX)
  if (h_)
  {
    if (flags_ & task_auto_join)
      join();
    else
      std::terminate();
  }

  if (tfd_ != -1)
  {
    int err = ::close(tfd_);
    if (err)
    {
      fprintf(stderr, "task::~task: %s\n", strerror(errno));
    }
  }

#elif defined(RTXX_USE_ALCHEMY)
  if (joinable_)
  {
    if (flags_ & task_auto_join)
      join();
    else
      std::terminate();
  }
#else
#error "not implemented"
#endif
}

void task::join(error_code &ec)
{
  int err;

#if defined(RTXX_USE_POSIX)
  void *rv;
  err = pthread_join(h_, &rv);
  h_ = 0;
#elif defined(RTXX_USE_ALCHEMY)
  err = -rt_task_join(&task_);
  if (!err)
    joinable_ = false;
#endif
  if (err)
    return ec.assign(err, system_category());
}

#if defined(RTXX_USE_POSIX)
void task::set_periodic(clockid_t clock, const struct itimerspec *its,
                        error_code &ec)
{
  assert(clock == CLOCK_MONOTONIC || clock == CLOCK_REALTIME);

  if (clk_ != clock)
  {
    if (tfd_ != -1)
      ::close(tfd_);
    tfd_ = -1;
  }

  if (tfd_ == -1)
  {
    tfd_ = timerfd_create(clock, TFD_CLOEXEC);
    clk_ = clock;
  }

  if (tfd_ == -1)
    ec.assign(errno, system_category());

  int err = timerfd_settime(tfd_, TFD_TIMER_ABSTIME, its, nullptr);
  if (err)
    ec.assign(errno, system_category());
}
#elif defined(RTXX_USE_ALCHEMY)
void task::set_periodic(RTIME start, RTIME interval, error_code &ec)
{
  int err;
  err = rt_task_set_periodic(&task_, start, interval);
  if (err)
    ec.assign(-err, system_category());
}

#endif

unsigned task::wait_period(error_code &ec)
{
  assert(this == this_task::detail::current_task);

#if defined(RTXX_USE_POSIX)
  uint64_t buf;
  int n = ::read(tfd_, &buf, sizeof(buf));
  assert(n == sizeof(buf));
  return buf - 1;
#elif defined(RTXX_USE_ALCHEMY)
  unsigned long buf;
  int err = rt_task_wait_period(&buf);
  if (err)
    ec.assign(-err, system_category());
  return buf;
#endif
}

unsigned task::wait_period()
{
  error_code ec;
  auto n = wait_period(ec);
  if (ec && ec != std::errc::timed_out)
    throw system_error(ec, "task::wait_period");
  return n;
}

void task::detach()
{
  assert(this != this_task::detail::current_task);

#if defined(RTXX_USE_POSIX)
  int err = pthread_detach(h_);
  h_ = 0;
  if (err)
    throw system_error(err, system_category(), "task::detach");
#elif defined(RTXX_USE_ALCHEMY)
  int err = rt_task_unbind(&task_);
  if (err)
    throw system_error(-err, system_category(), "task::detach");
  joinable_ = false;
#endif
}

void *task::entry(void *arg) noexcept
{
  auto self = reinterpret_cast<task *>(arg);
  this_task::detail::current_task = self;

#if defined(RTXX_USE_POSIX) && defined(RTXX_DEBUG)
  signal(SIGDEBUG, [](int sig) {
    fprintf(stderr, "Thread %s: Signal caught: %s\n",
            this_task::detail::current_task->opts_.name, strsignal(sig));
    print_backtrace();
  });

  {
    int err = pthread_setmode_np(0, PTHREAD_WARNSW, nullptr);
    if (err)
    {
      fprintf(stderr, "task::entry: %s\n", strerror(err));
    }
  }
#endif

  try
  {
    self->fn_();
  }
  catch (...)
  {
    std::terminate();
  }
  return nullptr;
}

#if defined(RTXX_USE_POSIX)
void task::init(task::options const &opt, error_code &ec)
{
  int err;
  pthread_attr_t attr;

  struct destroy_attr
  {
    pthread_attr_t *p_attr;

    ~destroy_attr()
    {
      int err = pthread_attr_destroy(p_attr);
      if (err)
      {
        fprintf(stderr, "task::init: %s", strerror(err));
      }
    }
  };

  err = pthread_attr_init(&attr);
  if (err)
    return ec.assign(errno, system_category());

  const destroy_attr guard{&attr};

  if (opt.priority > 0)
  {
    err = pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
    if (err)
      return ec.assign(err, system_category());

    err = pthread_attr_setschedpolicy(&attr, opt.schedpolicy);
    if (err)
      return ec.assign(err, system_category());

    const struct sched_param param = {
        .sched_priority = opt.priority,
    };
    err = pthread_attr_setschedparam(&attr, &param);
    if (err)
      return ec.assign(err, system_category());
  }

  if (opt.stack_size > 0)
  {
    err = pthread_attr_setstacksize(&attr, opt.stack_size);
    if (err)
      return ec.assign(err, system_category());
  }

  if (opt.cpu_set)
  {
#if defined(HAVE_PTHREAD_ATTR_SETAFFINITY_NP)
    err = pthread_attr_setaffinity_np(&attr, sizeof(cpu_set_t), opt.cpu_set);
    if (err)
      return ec.assign(err, system_category());
#endif
  }

  err = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
  if (err)
    return ec.assign(err, system_category());

  pthread_t t;
  err = pthread_create(&t, &attr, entry, this);
  if (err)
    return ec.assign(err, system_category());

  h_ = t;

  if (opt.name)
  {
    int r = pthread_setname_np(h_, opt.name);
    if (r)
    {
      fprintf(stderr, "pthread_setname_np: %s\n", strerror(r));
    }
  }

  ec.clear();
}

#elif defined(RTXX_USE_ALCHEMY)

void task::init(const task::options &opt, error_code &ec)
{
  int mode = T_JOINABLE;
#if defined(RTXX_DEBUG) && defined(__COBALT__)
  mode |= T_WARNSW;
#endif
  int err;
  err = rt_task_create(&task_, opt.name, opt.stack_size, opt.priority, mode);
  if (err)
    return ec.assign(-err, system_category());

  if (opt.cpu_set)
  {
    err = rt_task_set_affinity(&task_, opt.cpu_set);
    if (err)
      return ec.assign(-err, system_category());
  }

  err = rt_task_start(
      &task_, [](void *a) { entry(a); }, this);
  if (err)
    return ec.assign(-err, system_category());

  joinable_ = true;

  if (opt.auto_join)
    flags_ |= task_auto_join;

  ec.clear();
}

#endif

} // namespace rtxx
