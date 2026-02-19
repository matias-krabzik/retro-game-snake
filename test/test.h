#ifndef TEST_H
#define TEST_H

#include <stdio.h>
#include <string.h>

static int t_run, t_pass, t_fail;

#define RUN(fn) do { \
    t_run++; t_fail = 0; \
    printf("  %-44s", #fn); \
    fn(); \
    if (t_fail) { printf("\n"); } \
    else { t_pass++; printf("OK\n"); } \
} while (0)

#define ASSERT(c) do { \
    if (!(c)) { \
        t_fail = 1; \
        printf("FAIL (%s:%d: %s)", __FILE__, __LINE__, #c); \
        return; \
    } \
} while (0)

#define ASSERT_EQ(a, b) do { \
    long _a = (long)(a), _b = (long)(b); \
    if (_a != _b) { \
        t_fail = 1; \
        printf("FAIL (%s:%d: %s=%ld, want %ld)", \
               __FILE__, __LINE__, #a, _a, _b); \
        return; \
    } \
} while (0)

#define ASSERT_TRUE(c)  ASSERT(c)
#define ASSERT_FALSE(c) ASSERT(!(c))

#define TEST_REPORT() do { \
    printf("  %d/%d passed\n\n", t_pass, t_run); \
    return t_pass == t_run ? 0 : 1; \
} while (0)

#endif /* TEST_H */
