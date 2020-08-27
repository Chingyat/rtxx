find_package(Doxygen)

if (Doxygen_FOUND)
    set (DOXYGEN_GENERATE_HTML YES)
    set (DOXYGEN_GENERATE_XML YES)
    set (DOXYGEN_ENABLE_PREPROCESSING YES)
    set (DOXYGEN_PREDEFINED RTXX_USE_POSIX=1)
    doxygen_add_docs(
        doc
        ${CMAKE_CURRENT_SOURCE_DIR}/../
    )
endif()
