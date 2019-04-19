/* Sha256.h -- SHA-256 Hash
2010-06-11 : Igor Pavlov : Public domain */

#ifndef __CRYPTO_SHA256_H
#define __CRYPTO_SHA256_H

#include <stdlib.h>
#include <stdint.h>

#define SHA256_DIGEST_SIZE 32

void
sha256_init_bytes(uint8_t *state);

void
sha256_transform_bytes(uint8_t *state, const uint8_t *buffer);

#endif
