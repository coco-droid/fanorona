#ifndef COMPAT_H
#define COMPAT_H

#include <stdlib.h>
#include <string.h>

// Assurer la disponibilité de strdup
#ifndef _GNU_SOURCE
#ifndef _POSIX_C_SOURCE
// Fonction strdup compatible si elle n'existe pas
static inline char* safe_strdup(const char* s) {
    if (!s) return NULL;
    size_t len = strlen(s) + 1;
    char* copy = (char*)malloc(len);
    if (copy) {
        memcpy(copy, s, len);
    }
    return copy;
}

// Redéfinir strdup seulement si elle n'existe pas
#ifndef strdup
#define strdup safe_strdup
#endif

#endif // _POSIX_C_SOURCE
#endif // _GNU_SOURCE

#endif // COMPAT_H