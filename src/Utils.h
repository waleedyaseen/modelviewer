#pragma once

#define MAKE_NON_COPYABLE(ClassName)      \
    ClassName(ClassName const&) = delete; \
    ClassName& operator=(ClassName const&) = delete;

#define IMP_LOG_ERROR(...)        \
    fprintf(stderr, __VA_ARGS__); \
    fprintf(stderr, "\n")
#define IMP_LOG_WARN(...)         \
    fprintf(stderr, __VA_ARGS__); \
    fprintf(stderr, "\n")
#define IMP_LOG_INFO(...)         \
    fprintf(stdout, __VA_ARGS__); \
    fprintf(stdout, "\n")
#define IMP_LOG_DEBUG(...)        \
    fprintf(stdout, __VA_ARGS__); \
    fprintf(stdout, "\n")
#ifdef IMP_DEBUG
#    define IMP_DEBUG_ASSERT(x)                            \
        do {                                               \
            if (!(x)) {                                    \
                IMP_LOG_ERROR("ASSERTION FAILED: %s", #x); \
                __debugbreak();                            \
            }                                              \
        } while (0)
#else
#    define IMP_DEBUG_ASSERT(x)
#endif

#define BIT(x) (1 << (x))
