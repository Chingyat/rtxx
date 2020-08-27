#pragma once

#include <rtxx/mutex.hpp>

namespace rtxx
{
inline bool mutex::try_lock_for(chrono::nanoseconds duration)
{
#if defined(RTXX_USE_POSIX)
  return try_lock_until(realtime_clock::now() + duration);
#elif defined(RTXX_USE_ALCHEMY)
  return try_lock_until(monotonic_clock::now() + duration);
#endif
}

inline mutex::native_handle_type mutex::native_handle() { return &i_; }
} // namespace rtxx
