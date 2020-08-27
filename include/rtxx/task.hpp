#pragma once

#include <chrono>
#include <functional>
#include <memory>
#include <rtxx/clock.hpp>
#include <rtxx/config.hpp>
#include <rtxx/error.hpp>

#if defined(RTXX_USE_POSIX)
#include <pthread.h>
#include <sched.h>
#elif defined(RTXX_USE_ALCHEMY)
#include <alchemy/task.h>
#endif

namespace rtxx
{
class task;

/// Functions to access current task
namespace this_task
{
namespace detail
{
extern thread_local task *current_task;
}

/// Make the current task periodic
template <typename Clock, typename Duration, typename Duration1>
void set_periodic(chrono::time_point<Clock, Duration> start,
                  Duration1 interval);

/// Make the current task periodic
template <typename Clock, typename Duration, typename Duration1>
void set_periodic(chrono::time_point<Clock, Duration> start, Duration1 interval,
                  error_code &ec);

/// Wait for the next periodic release point.
RTXX_DECL unsigned wait_period(error_code &ec);

/// Wait for the next periodic release point.
RTXX_DECL unsigned wait_period();

/// Yield the processor
RTXX_DECL void yield(error_code &ec);

/// Yield the processor
RTXX_DECL void yield();

} // namespace this_task

/// Realtime task class
/** @par Example
 * @code
 *   #include <rtxx/task.hpp>
 *
 *   using namespace chrono_literals;
 *   using namespace rtxx;
 *
 *   std::atomic_flag run(true);
 *
 *   int main() {
 *     task my_task(task::options{priority(99)}, [] {
 *       this_task::set_periodic(monotonic_clock::now() + 10ms, 3ms);
 *
 *       while (!run.test_and_set()) {
 *         const auto overrun = this_task::wait_period();
 *         // do something
 *       }
 *
 *       // exit task
 *     });
 *
 *     my_task.join();
 *   }
 * @endcode
 */
class task
{
public:
  /// Task options
  struct options
  {
    /// The name of the task.
    /** This will be set after the task is started and before
     * the user supplied function object being invoked.
     */
    const char *name{};

    /// The stack size of the new task.
    /** If set to zero, a system-dependent default value will be used.
     */
    int stack_size{0};

    /// The priority of the new task.
    /** When set to zero, the task will be non-realtime.
     *  If the value is between 1 and 99, the task will be realtime.
     */
    int priority{0};

    /// CPU affinity of the new task
    const cpu_set_t *cpu_set{};

    /// Schedule policy of the new.
    /** This is effective only if the priority > 0.
     */
    int schedpolicy{SCHED_FIFO};

    /// Automatically join the thread when destructed
    bool auto_join{false};

    /// Construct options from convenient initializers
    /** GCC 7.* does not support non-trivial designated initializers,
     *  so I provide this way to initialize options.
     */
    template <typename... Initializers>
    constexpr explicit options(Initializers &&... init) noexcept;
  };

#if defined(RTXX_USE_POSIX)
  /// Native handle to the task.
  using native_handle_type = pthread_t;

#elif defined(RTXX_USE_ALCHEMY)
  using native_handle_type = RT_TASK *;
#endif

  /// Deleted copy constructor
  task(task const &) = delete;

  /// Create and run a realtime task
  /** Equivalent to \c task(options, F&&). */
  template <typename F> explicit task(F &&f);

  /// Create and run a realtime task
  template <typename F> task(options opt, F &&f);

  /// Destroy a task.
  /** @par Preconditions
   *    this->joinable() returns false.
   */
  RTXX_DECL ~task();

  /// Wait for a task to terminate
  /** @throw system_error when error occurs. */
  RTXX_DECL void join();

  /// Wait for a task to terminate
  RTXX_DECL void join(error_code &ec);

  /// Checks if the std::thread object identifies an active thread of
  /// execution.
  RTXX_DECL bool joinable() const;

  /// permits the thread to execute independently from the thread handle
  RTXX_DECL void detach();

  /// permits the thread to execute independently from the thread handle
  RTXX_DECL void detach(error_code &ec);

  template <typename Duration> void set_periodic(Duration interval);

  /// Make the task periodic
  template <typename Clock, typename Duration, typename Duration1>
  void set_periodic(chrono::time_point<Clock, Duration> start,
                    Duration1 interval);

  /// Make the task periodic
  template <typename Clock, typename Duration, typename Duration1>
  void set_periodic(chrono::time_point<Clock, Duration> start,
                    Duration1 interval, error_code &ec);

#if defined(RTXX_USE_POSIX)
  /// Make the task periodic
  RTXX_DECL void set_periodic(clockid_t clock, const struct itimerspec *spec,
                              error_code &ec);
#elif defined(RTXX_USE_ALCHEMY)
  RTXX_DECL void set_periodic(RTIME start, RTIME interval, error_code &ec);
#endif

  /// Get the native handle to the task.
  RTXX_DECL native_handle_type native_handle();

private:
  task() noexcept = default;

  /// Entry function for the task
  RTXX_DECL static void *entry(void *arg) noexcept;

  /// Initialize the task
  RTXX_DECL void init(options const &opt, error_code &ec);

  /// Wait for the next periodic release point.
  /** This can only be called from the current task. */
  RTXX_DECL unsigned wait_period();

  /// Wait for the next periodic release point.
  /** This can only be called from the current task. */
  RTXX_DECL unsigned wait_period(error_code &ec);

#if defined(RTXX_USE_POSIX)
  pthread_t h_{};
#elif defined(RTXX_USE_ALCHEMY)
  RT_TASK task_{};
  bool joinable_{false};
#endif

#if defined(RTXX_USE_POSIX)
  /// The timer will be used if \c set_periodic() is called.
  int tfd_{-1};
  clockid_t clk_{};
#endif

  unsigned long flags_;

  /// Type-erased task routine.
  std::function<void()> fn_;

  friend unsigned this_task::wait_period();
  friend unsigned this_task::wait_period(error_code &ec);
};

/// Returns an initializer for priority task option.
RTXX_INLINE_DECL constexpr auto priority(int value);

/// Returns an initializer for name task option.
RTXX_INLINE_DECL constexpr auto name(const char *name);

/// Returns an initializer for stack size task option.
RTXX_INLINE_DECL constexpr auto stack_size(int size);

/// Returns an initializer for cpu_set task option.
/** Ownership of the cpu_set_t object is not transferred after
 *  calling this function.
 */
RTXX_INLINE_DECL constexpr auto cpu_set(const cpu_set_t *set);

/// Returns an initializer for schedpolicy task option
RTXX_INLINE_DECL constexpr auto schedpolicy(int sched);

} // namespace rtxx

#include <rtxx/impl/task.hpp>
#include <rtxx/impl/task.tpp>
#ifdef RTXX_HEADER_ONLY
#include <rtxx/impl/task.ipp>
#endif