#pragma once

#include <cstring>
#include <rtxx/condition_variable.hpp>
#include <rtxx/error.hpp>
#include <rtxx/mutex.hpp>

namespace rtxx
{
using chrono::duration;
using chrono::time_point;
using namespace rtxx::detail;

condition_variable::condition_variable()
{
  int err;

#if defined(RTXX_USE_POSIX)
  err = pthread_cond_init(&c_, nullptr);
#elif defined(RTXX_USE_ALCHEMY)
  err = -rt_cond_create(&c_, nullptr);
#endif

  if (err)
    throw system_error(err, system_category(), "pthread_cond_init");
}

condition_variable::~condition_variable()
{
  int err;

#if defined(RTXX_USE_POSIX)
  err = pthread_cond_destroy(&c_);
#elif defined(RTXX_USE_ALCHEMY)
  err = -rt_cond_delete(&c_);
#endif

  if (err)
  {
    fprintf(stderr, "pthread_cond_destroy: %s\n", strerror(err));
  }
}

void condition_variable::notify_all()
{
  int err;

#if defined(RTXX_USE_POSIX)
  err = pthread_cond_broadcast(&c_);
#elif defined(RTXX_USE_ALCHEMY)
  err = -rt_cond_broadcast(&c_);
#endif

  if (err)
    throw system_error(err, system_category(), "pthread_cond_broadcast");
}

void condition_variable::notify_one()
{
  int err;

#if defined(RTXX_USE_POSIX)
  err = pthread_cond_signal(&c_);
#elif defined(RTXX_USE_ALCHEMY)
  err = -rt_cond_signal(&c_);
#endif

  if (err)
    throw system_error(err, system_category(), "pthread_cond_signal");
}

bool condition_variable::wait(std::unique_lock<mutex> &lock)
{
  int err;

#if defined(RTXX_USE_POSIX)
  err = pthread_cond_wait(&c_, lock.mutex()->native_handle());
#elif defined(RTXX_USE_ALCHEMY)
  err = -rt_cond_wait(&c_, lock.mutex()->native_handle(), TM_INFINITE);
#endif

  if (err == 0)
    return true;

  if (err == EINTR || err == EAGAIN)
    return false;

  throw system_error(err, system_category(), "pthread_cond_wait");
}

bool condition_variable::wait_until(std::unique_lock<mutex> &lock,
                                    const struct timespec *abs_time)
{
  int err;

#if defined(RTXX_USE_POSIX)
  err = pthread_cond_timedwait(&c_, lock.mutex()->native_handle(), abs_time);
#elif defined(RTXX_USE_ALCHEMY)
  err = -rt_cond_wait(&c_, lock.mutex()->native_handle(), TM_INFINITE);
#endif

  if (err == 0)
    return true;

  if (err == EINTR || err == EAGAIN || err == ETIMEDOUT)
    return false;

  throw system_error(err, system_category(), "pthread_cond_timedwait");
}

} // namespace rtxx
