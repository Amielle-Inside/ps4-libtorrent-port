// PS4 port: OpenSSL-compatible SHA-1/RC4 used by rakshasa's utils/sha1.h and
// rc4.h. BitTorrent *requires* SHA-1 (piece hashes), so we ship a real
// implementation instead of stubs. Phase 2 swaps this for ps4-openssl.
#ifndef PS4_PORT_CRYPTO_SHIM_H
#define PS4_PORT_CRYPTO_SHIM_H

#include <stdint.h>
#include <stddef.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

// --- SHA-1 ---
typedef struct SHA1_CTX_st {
    uint32_t state[5];
    uint64_t bitlen;
    uint8_t  buffer[64];
    size_t   buflen;
} SHA1_CTX;

void SHA1_Init(SHA1_CTX *ctx);
void SHA1_Update(SHA1_CTX *ctx, const void *data, size_t len);
void SHA1_Final(unsigned char out[20], SHA1_CTX *ctx);

// --- OpenSSL EVP-style API surface used by utils/sha1.h ---
typedef SHA1_CTX EVP_MD_CTX;
typedef struct { int dummy; } EVP_MD;

EVP_MD_CTX *EVP_MD_CTX_new(void);
void        EVP_MD_CTX_free(EVP_MD_CTX *ctx);
void        EVP_MD_CTX_reset(EVP_MD_CTX *ctx);
int         EVP_DigestInit(EVP_MD_CTX *ctx, const EVP_MD *type);
int         EVP_DigestUpdate(EVP_MD_CTX *ctx, const void *d, size_t n);
int         EVP_DigestFinal_ex(EVP_MD_CTX *ctx, unsigned char *md, unsigned int *s);
const EVP_MD *EVP_sha1(void);

// --- RC4 ---
typedef struct RC4_KEY_st {
    uint32_t x, y;
    uint8_t  data[256];
} RC4_KEY;

void RC4_set_key(RC4_KEY *key, int len, const unsigned char *data);
void RC4(RC4_KEY *key, size_t len, const unsigned char *indata, unsigned char *outdata);

#ifdef __cplusplus
}
#endif
#endif // PS4_PORT_CRYPTO_SHIM_H
