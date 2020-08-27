#ifndef RTXX_CONFIG_HPP
#define RTXX_CONFIG_HPP

#if !defined(RTXX_USE_POSIX) && !defined(RTXX_USE_ALCHEMY) &&                  \
    !defined(RTXX_USE_RTDM)
#define RTXX_USE_POSIX
#endif

#if !defined(RTXX_HEADER_ONLY) && !defined(RTXX_SHARED_LIBRARY)
#define RTXX_HEADER_ONLY
#endif

#ifndef RTXX_DECL

#ifdef RTXX_HEADER_ONLY
#define RTXX_DECL inline
#elif RTXX_SHARED_LIBRARY
#ifdef _WIN32
#define RTXX_DECL __declspec(dllimport)
#else
#define RTXX_DECL __attribute__((visibility("default")))
#endif
#elif RTXX_STATIC_LIBRARY
#define RTXX_DECL
#endif
#endif

#define RTXX_INLINE_DECL inline

#define RTXX_CONSTEXPR_LAMBDA constexpr

#endif
