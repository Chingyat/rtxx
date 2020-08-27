add_library(rtxx-header-only INTERFACE)

target_include_directories(rtxx-header-only
    INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

target_compile_features(rtxx-header-only
    INTERFACE
    cxx_std_17
)

add_library(rtxx-shared SHARED rtxx.cpp)
target_include_directories(rtxx-shared PRIVATE
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
    INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)
target_compile_features(rtxx-shared PUBLIC cxx_std_17)
target_compile_definitions(rtxx-shared INTERFACE
    RTXX_SHARED_LIBRARY)
target_link_libraries(rtxx-shared PRIVATE Threads::Threads)
set_target_properties(rtxx-shared
    PROPERTIES
    OUTPUT_NAME rtxx
    )

add_library(rtxx::rtxx ALIAS rtxx-shared)
