// PS4 port: real SHA-1 + RC4 implementations backing ps4_crypto.h.
// SHA-1: straightforward FIPS 180-1 style implementation.
#include "ps4_crypto.h"
#include <stdlib.h>
#include <sched.h>

// libc++.a of the SDK lacks __libcpp_atomic_wait(void const volatile*, long)
// (only the int overload exists); provide a spin-yield fallback used by
// std::atomic::wait in thread.cc.
extern "C" void _ZNSt3__120__libcpp_atomic_waitEPVKvl(const volatile void* ptr, long val) {
    (void)ptr; (void)val;
    // libtorrent only uses atomic wait as a poll backoff; yield-spin briefly
    for (int i = 0; i < 64; i++)
        sched_yield();
}

// ---------------- SHA-1 ----------------
static const uint32_t K[4] = {0x5A827999u, 0x6ED9EBA1u, 0x8F1BBCDCu, 0xCA62C1D6u};

#define ROL(v, n) (((v) << (n)) | ((v) >> (32 - (n))))

static void sha1_block(SHA1_CTX *ctx, const uint8_t *p) {
    uint32_t w[80];
    for (int i = 0; i < 16; i++)
        w[i] = ((uint32_t)p[i*4] << 24) | ((uint32_t)p[i*4+1] << 16) |
               ((uint32_t)p[i*4+2] << 8) | (uint32_t)p[i*4+3];
    for (int i = 16; i < 80; i++)
        w[i] = ROL(w[i-3] ^ w[i-8] ^ w[i-14] ^ w[i-16], 1);

    uint32_t a = ctx->state[0], b = ctx->state[1], c = ctx->state[2],
             d = ctx->state[3], e = ctx->state[4];

    for (int i = 0; i < 80; i++) {
        uint32_t f, k = K[i / 20];
        if (i < 20)      f = (b & c) | ((~b) & d);
        else if (i < 40) f = b ^ c ^ d;
        else if (i < 60) f = (b & c) | (b & d) | (c & d);
        else             f = b ^ c ^ d;
        uint32_t t = ROL(a, 5) + f + e + k + w[i];
        e = d; d = c; c = ROL(b, 30); b = a; a = t;
    }

    ctx->state[0] += a; ctx->state[1] += b; ctx->state[2] += c;
    ctx->state[3] += d; ctx->state[4] += e;
}

void SHA1_Init(SHA1_CTX *ctx) {
    ctx->state[0] = 0x67452301u; ctx->state[1] = 0xEFCDAB89u;
    ctx->state[2] = 0x98BADCFEu; ctx->state[3] = 0x10325476u;
    ctx->state[4] = 0xC3D2E1F0u;
    ctx->bitlen = 0; ctx->buflen = 0;
}

void SHA1_Update(SHA1_CTX *ctx, const void *data, size_t len) {
    const uint8_t *p = (const uint8_t *)data;
    ctx->bitlen += (uint64_t)len * 8;
    while (len > 0) {
        size_t take = 64 - ctx->buflen;
        if (take > len) take = len;
        memcpy(ctx->buffer + ctx->buflen, p, take);
        ctx->buflen += take; p += take; len -= take;
        if (ctx->buflen == 64) { sha1_block(ctx, ctx->buffer); ctx->buflen = 0; }
    }
}

void SHA1_Final(unsigned char out[20], SHA1_CTX *ctx) {
    uint64_t bits = ctx->bitlen;
    uint8_t pad = 0x80;
    SHA1_Update(ctx, &pad, 1);
    uint8_t zero = 0;
    while (ctx->buflen != 56) SHA1_Update(ctx, &zero, 1);
    uint8_t lenb[8];
    for (int i = 0; i < 8; i++) lenb[i] = (uint8_t)(bits >> (56 - 8*i));
    SHA1_Update(ctx, lenb, 8);
    for (int i = 0; i < 5; i++) {
        out[i*4]   = (uint8_t)(ctx->state[i] >> 24);
        out[i*4+1] = (uint8_t)(ctx->state[i] >> 16);
        out[i*4+2] = (uint8_t)(ctx->state[i] >> 8);
        out[i*4+3] = (uint8_t)(ctx->state[i]);
    }
}

// ---------------- EVP wrappers ----------------
static EVP_MD g_sha1_md = {0};

EVP_MD_CTX *EVP_MD_CTX_new(void)              { return (EVP_MD_CTX *)calloc(1, sizeof(SHA1_CTX)); }
void EVP_MD_CTX_free(EVP_MD_CTX *ctx)         { free(ctx); }
void EVP_MD_CTX_reset(EVP_MD_CTX *ctx)        { SHA1_Init(ctx); }
int  EVP_DigestInit(EVP_MD_CTX *ctx, const EVP_MD *type) { (void)type; SHA1_Init(ctx); return 1; }
int  EVP_DigestUpdate(EVP_MD_CTX *ctx, const void *d, size_t n) { SHA1_Update(ctx, d, n); return 1; }
int  EVP_DigestFinal_ex(EVP_MD_CTX *ctx, unsigned char *md, unsigned int *s) {
    SHA1_Final(md, ctx);
    if (s) *s = 20;
    return 1;
}
int  EVP_DigestFinal(EVP_MD_CTX *ctx, unsigned char *md, unsigned int *s) {
    return EVP_DigestFinal_ex(ctx, md, s);
}
const EVP_MD *EVP_sha1(void)                  { return &g_sha1_md; }

// ---------------- Base64 (EVP_EncodeBlock/EVP_DecodeBlock) ----------------
int EVP_EncodeBlock(unsigned char *out, const unsigned char *in, int inlen) {
    static const char tbl[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    int i = 0, o = 0;
    for (; i + 2 < inlen; i += 3) {
        uint32_t v = ((uint32_t)in[i] << 16) | ((uint32_t)in[i+1] << 8) | in[i+2];
        out[o++] = tbl[(v >> 18) & 63]; out[o++] = tbl[(v >> 12) & 63];
        out[o++] = tbl[(v >> 6) & 63];  out[o++] = tbl[v & 63];
    }
    int rem = inlen - i;
    if (rem == 1) {
        uint32_t v = (uint32_t)in[i] << 16;
        out[o++] = tbl[(v >> 18) & 63]; out[o++] = tbl[(v >> 12) & 63];
        out[o++] = '='; out[o++] = '=';
    } else if (rem == 2) {
        uint32_t v = ((uint32_t)in[i] << 16) | ((uint32_t)in[i+1] << 8);
        out[o++] = tbl[(v >> 18) & 63]; out[o++] = tbl[(v >> 12) & 63];
        out[o++] = tbl[(v >> 6) & 63]; out[o++] = '=';
    }
    out[o] = 0;
    return o;
}

static int b64_val(char c) {
    if (c >= 'A' && c <= 'Z') return c - 'A';
    if (c >= 'a' && c <= 'z') return c - 'a' + 26;
    if (c >= '0' && c <= '9') return c - '0' + 52;
    if (c == '+') return 62;
    if (c == '/') return 63;
    return -1;
}

int EVP_DecodeBlock(unsigned char *out, const unsigned char *in, int inlen) {
    // strip trailing '=' padding
    while (inlen > 0 && in[inlen-1] == '=') inlen--;
    int o = 0, i = 0;
    for (; i + 3 < inlen; i += 4) {
        int a = b64_val(in[i]), b = b64_val(in[i+1]), c = b64_val(in[i+2]), d = b64_val(in[i+3]);
        if (a < 0 || b < 0 || c < 0 || d < 0) return -1;
        uint32_t v = ((uint32_t)a << 18) | ((uint32_t)b << 12) | ((uint32_t)c << 6) | d;
        out[o++] = (v >> 16) & 0xff; out[o++] = (v >> 8) & 0xff; out[o++] = v & 0xff;
    }
    int rem = inlen - i;
    if (rem == 2) {
        int a = b64_val(in[i]), b = b64_val(in[i+1]);
        if (a < 0 || b < 0) return -1;
        uint32_t v = ((uint32_t)a << 18) | ((uint32_t)b << 12);
        out[o++] = (v >> 16) & 0xff;
    } else if (rem == 3) {
        int a = b64_val(in[i]), b = b64_val(in[i+1]), c = b64_val(in[i+2]);
        if (a < 0 || b < 0 || c < 0) return -1;
        uint32_t v = ((uint32_t)a << 18) | ((uint32_t)b << 12) | ((uint32_t)c << 6);
        out[o++] = (v >> 16) & 0xff; out[o++] = (v >> 8) & 0xff;
    } else if (rem != 0) {
        return -1;
    }
    return o;
}

// ---------------- RC4 ----------------
void RC4_set_key(RC4_KEY *key, int len, const unsigned char *data) {
    for (int i = 0; i < 256; i++) key->data[i] = (uint8_t)i;
    key->x = 0; key->y = 0;
    uint32_t j = 0;
    for (int i = 0; i < 256; i++) {
        j = (j + key->data[i] + data[i % len]) & 0xff;
        uint8_t t = key->data[i]; key->data[i] = key->data[j]; key->data[j] = t;
    }
}

void RC4(RC4_KEY *key, size_t len, const unsigned char *indata, unsigned char *outdata) {
    for (size_t n = 0; n < len; n++) {
        key->x = (key->x + 1) & 0xff;
        key->y = (key->y + key->data[key->x]) & 0xff;
        uint8_t t = key->data[key->x];
        key->data[key->x] = key->data[key->y];
        key->data[key->y] = t;
        outdata[n] = indata[n] ^ key->data[(key->data[key->x] + key->data[key->y]) & 0xff];
    }
}
