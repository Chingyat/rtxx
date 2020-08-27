#pragma once

#include <cstring>
#include <rtxx/error.hpp>
#include <rtxx/mutex.hpp>

namespace rtxx
{
mutex::mutex()
{
  int err = 0;
#if defined(RTXX_USE_POSIX)
  err = pthread_mutex_init(&i_, nullptr);
#elif defined(RTXX_USE_ALCHEMY)
  err = -rt_mutex_create(&i_, nullptr);
#else
#error "no implementation selected"
#endif
  if (err)
    throw system_error(err, system_category(), "mutex::mutex");
}

mutex::~mutex()
{
  int err;
#if defined(RTXX_USE_POSIX)
  err = pthread_mutex_destroy(&i_);
#elif defined(RTXX_USE_ALCHEMY)
  err = -rt_mutex_delete(&i_);
#else
#error "no implementation selected"
#endif
  if (err)
    fprintf(stderr, "mutex::~mutex: %s", strerror(err));
}

#if defined(RTXX_USE_POSIX)
bool mutex::try_lock_until(realtime_clock::time_point time_limit)
#elif defined(RTXX_USE_ALCHEMY)
bool mutex::try_lock_until(monotonic_clock::time_point time_limit)
#endif
{
  const struct timespec ts =
      detail::duration_to_timespec(time_limit.time_since_epoch());
  int err;
#if defined(RTXX_USE_POSIX)
  err = pthread_mutex_timedlock(&i_, &ts);
#elif defined(RTXX_USE_ALCHEMY)
  err = -rt_mutex_acquire_timed(&i_, &ts);
#else
#error "no implementation selected"
#endif

  if (!err)
    return true;

  if (err == ETIMEDOUT || err == EWOULDBLOCK)
    return false;

  throw system_error(err, system_category(), "mutex::try_lock_until");
}

bool mutex::try_lock()
{
  int err;
#if defined(RTXX_USE_POSIX)
  err = pthread_mutex_trylock(&i_);
#elif defined(RTXX_USE_ALCHEMY)
  err = -rt_mutex_acquire(&i_, TM_NONBLOCK);
#else
#error "no implementation selected"
#endif
  if (err == EWOULDBLOCK || err == EBUSY)
    return false;

  if (!err)
    return true;

  throw system_error(err, system_category(), "mutex::try_lock");
}

void mutex::lock()
{
  int err;
#if defined(RTXX_USE_POSIX)
  err = pthread_mutex_lock(&i_);
#elif defined(RTXX_USE_ALCHEMY)
  err = -rt_mutex_acquire(&i_, TM_INFINITE);
#else
#error "no implementation selected"
#endif
  if (err)
    throw system_error(err, system_category(), "mutex::lock");
}

void mutex::unlock()
{
  int err;
#if defined(RTXX_USE_POSIX)
  err = pthread_mutex_unlock(&i_);
#elif defined(RTXX_USE_ALCHEMY)
  err = -rt_mutex_release(&i_);
#else
#error "no implementation selected"
#endif

  if (err)
    throw system_error(err, system_category(), "mutex::unlock");
}

} // namespace rtxx
