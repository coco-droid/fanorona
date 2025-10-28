#ifndef COMPAT_H
#define COMPAT_H

#include <stdlib.h>
#include <string.h>

// Ensure strdup availability
#ifndef _GNU_SOURCE
#ifndef _POSIX_C_SOURCE
// Compatible strdup function if it doesn't exist
static inline char* safe_strdup(const char* s) {
    if (!s) return NULL;
    size_t len = strlen(s) + 1;
    char* copy = (char*)malloc(len);
    if (copy) {
        memcpy(copy, s, len);
    }
    return copy;
}

// Redefine strdup only if it doesn't exist
#ifndef strdup
#define strdup safe_strdup
#endif

#endif // _POSIX_C_SOURCE
#endif // _GNU_SOURCE

#endif // COMPAT_H