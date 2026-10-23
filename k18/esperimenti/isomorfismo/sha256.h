/* SHA-256 (FIPS 180-4), self-contained implementation for this package. */
#ifndef SHA256_H
#define SHA256_H

#include <stddef.h>
#include <stdint.h>

typedef struct {
    uint32_t h[8];
    uint64_t len;              /* total message length in bytes */
    unsigned char buf[64];
    size_t nbuf;
} sha256_ctx;

void sha256_init(sha256_ctx *c);
void sha256_update(sha256_ctx *c, const void *data, size_t n);
void sha256_final(sha256_ctx *c, unsigned char out[32]);

/* Lowercase hexadecimal digest of data[0..n), NUL-terminated. */
void sha256_hex(const unsigned char *data, size_t n, char out[65]);

#endif
