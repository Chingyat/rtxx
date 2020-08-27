add_executable(task_test task_test.cxx)
target_link_libraries(task_test PRIVATE rtxx::rtxx Threads::Threads)
add_test(task_test task_test)

add_executable(semaphore_test semaphore_test.cxx)
target_link_libraries(semaphore_test PRIVATE rtxx::rtxx Threads::Threads)
add_test(semaphore_test semaphore_test)

add_executable(compile_test compile_test.cxx)
target_link_libraries(compile_test PRIVATE rtxx::rtxx)

add_executable(redefinition_test redefinition_test_1.cxx redefinition_test_2.cxx)
target_link_libraries(redefinition_test PRIVATE rtxx-header-only)
