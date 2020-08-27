#pragma once

#include <cassert>
#include <rtxx/error.hpp>
#include <rtxx/semaphore.hpp>

namespace rtxx
{
#if defined(RTXX_USE_POSIX)
semaphore::semaphore(value_type init_value)
{
  int err = sem_init(&sem_, 0, init_value);
  if (err == -1)
    throw system_error(errno, system_category(), "semaphore::semaphore");
}

semaphore::~semaphore()
{
  int err = sem_destroy(&sem_);
  if (err == -1)
    perror("semaphore::~semaphore");
}

void semaphore::post()
{
  int err = sem_post(&sem_);
  if (err == -1)
    throw system_error(errno, system_category(), "semaphore::post");
}

void semaphore::wait()
{
  int err = sem_wait(&sem_);
  if (err == -1)
    throw system_error(errno, system_category(), "semaphore::wait");
}

bool semaphore::try_wait()
{
  if (int err = sem_trywait(&sem_); err == -1)
  {
    if (errno == EAGAIN)
      return false;
    throw system_error(errno, system_category(), "semaphore::try_wait");
  }
  return true;
}

bool semaphore::wait_until(const struct timespec *abs_timeout)
{
  int r = sem_timedwait(&sem_, abs_timeout);
  if (r == -1)
  {
    if (errno == ETIMEDOUT)
    {
      return false;
    }

    throw system_error(errno, system_category(), "semaphore::wait_until");
  }
  return true;
}

semaphore::value_type semaphore::get_value() const
{
  int sval;

  // If  one  or more processes or threads are blocked waiting to lock the
  // semaphore with sem_wait(3), POSIX.1 permits two possibilities for the
  // value returned in sval: either 0 is returned; or a negative number whose
  // absolute value is  the count of the number of processes and threads
  // currently blocked in sem_wait(3).  Linux adopts the former behavior.
  int r = sem_getvalue(const_cast<sem_t *>(&sem_), &sval);

  if (r == -1)
    throw system_error(errno, system_category(), "semaphore::value");

#ifdef __linux__
  assert(sval >= 0);
  return sval;
#else
  if (sval < 0)
    sval = 0;
  return sval;
#endif
}

#elif defined(RTXX_USE_ALCHEMY)

semaphore::semaphore(value_type init_value)
{
  int err = rt_sem_create(&sem_, nullptr, init_value, S_FIFO | S_PRIO);
  if (err != 0)
    throw system_error(-err, system_category(), "semaphore::semaphore");
}

semaphore::~semaphore()
{
  int err = rt_sem_delete(&sem_);
  if (err != 0)
  {
    fprintf(stderr, "semaphore::~semaphore: %s\n", strerror(-err));
  }
}

void semaphore::post()
{
  int err = rt_sem_v(&sem_);
  if (err != 0)
    throw system_error(-err, system_category(), "semaphore::post");
}

void semaphore::wait()
{
  int err = rt_sem_p(&sem_, TM_INFINITE);
  if (err != 0)
    throw system_error(-err, system_category(), "semaphore::wait");
}

bool semaphore::try_wait()
{
  if (int err = rt_sem_p(&sem_, TM_NONBLOCK); err == -1)
  {
    if (err == -EWOULDBLOCK)
      return false;
    throw system_error(-err, system_category(), "semaphore::try_wait");
  }
  return true;
}

bool semaphore::wait_until(const struct timespec *abs_timeout)
{
  int err = rt_sem_p_timed(&sem_, abs_timeout);

  if (err == 0)
    return true;

  if (err == -ETIMEDOUT || err == -EWOULDBLOCK)
    return false;

  throw system_error(-err, system_category(), "semaphore::wait_until");
}

#else
#error "not implemented"
#endif
} // namespace rtxx