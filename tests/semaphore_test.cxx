#include "rtxx/semaphore.hpp"

#include <iostream>

#include "rtxx/task.hpp"

using namespace rtxx;

int main() {
    semaphore sem(1);

    assert(sem.get_value() == 1);

    task t1(task::options{priority(99)}, [&] {
        sem.wait();
        std::cout << "t1 exited" << std::endl;
    });
    task t2(task::options{priority(99)}, [&] {
        if (sem.try_wait()) sem.post();
        std::cout << "t2 exited" << std::endl;
    });
    task t3(task::options{priority(98)}, [&] {
        sem.post();
        std::cout << "t3 exited" << std::endl;
    });

    t1.join();
    t2.join();
    t3.join();

    assert(sem.get_value() == 1);
}