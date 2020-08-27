#include <csignal>
#include <iomanip>
#include <iostream>
#include <rtxx/mutex.hpp>
#include <rtxx/task.hpp>

using namespace rtxx;

using namespace std::literals;

int main()
{
  using namespace rtxx;

  task task1(task::options{name("task_test"), priority(99), schedpolicy(SCHED_FIFO)}, []() {
    auto expected = monotonic_clock::now();
    auto period = 1ms;

    this_task::set_periodic(expected, period);
    auto st = monotonic_clock::now();
    int c = 0;

    double diff_sum(0), minjitter(1000000), maxjitter(0);
    unsigned overrun(0);

    while (c != 1000)
    {
      auto o = this_task::wait_period();
      overrun += o;
      if (overrun > 0)
      {
        std::cerr << "overrun: " << overrun << "\n";
        return;
      }

      auto time = monotonic_clock::now();
      auto diff = (time.time_since_epoch().count() - expected.time_since_epoch().count());

      // typeid(get_object()).name();

      if (minjitter > abs(diff))
        minjitter = abs(diff);

      if (maxjitter < abs(diff))
        maxjitter = abs(diff);

      diff_sum += diff;

      expected += period * (1 + o);
      c += 1 + o;
    }
    auto nd = monotonic_clock::now();
    std::cout << "start:\t" << st.time_since_epoch().count() << '\n'
              << "end:\t" << nd.time_since_epoch().count() << '\n'
              << "avgdiff:\t" << (diff_sum / c) << '\n'
              << "maxjitter:\t" << (long)maxjitter << '\n'
              << "minjitter:\t" << (long)minjitter << '\n'
              << "overrun:\t" << overrun << '\n';
  });

  task1.join();
  std::cout << "task joined\n";
}
