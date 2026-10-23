/* Minimal JSON parser (DOM) for the fixed data formats of this package. */
#ifndef JSON_MIN_H
#define JSON_MIN_H

#include <stddef.h>

typedef enum { J_NULL, J_FALSE, J_TRUE, J_NUM, J_STR, J_ARR, J_OBJ } jtype;

typedef struct jnode {
    jtype t;
    char *s;              /* J_STR: decoded UTF-8 text; J_NUM: raw text */
    size_t slen;          /* length of s in bytes */
    int is_int;           /* J_NUM: no fraction and no exponent */
    int fits;             /* J_NUM integer that fits in a long */
    long ival;            /* J_NUM: value when is_int && fits */
    size_t n;             /* J_ARR / J_OBJ: number of children */
    struct jnode **items; /* children */
    char **keys;          /* J_OBJ: decoded keys, parallel to items */
} jnode;

/* Parse a whole file; NULL on error (message written to err). */
jnode *json_load_file(const char *path, char *err, size_t errlen);
/* Parse a buffer of len bytes; NULL on error. */
jnode *json_parse(const char *buf, size_t len, char *err, size_t errlen);
void json_free(jnode *n);

/* Member of an object by key (last occurrence wins); NULL if absent. */
jnode *json_get(const jnode *obj, const char *key);
/* Element of an array; NULL if out of range or not an array. */
jnode *json_at(const jnode *arr, size_t i);
/* 1 and the value in *v if n is an integer number that fits a long. */
int json_long(const jnode *n, long *v);

#endif
