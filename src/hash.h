// Copyright (c) 2009-2016 Satoshi Nakamoto
// Copyright (c) 2009-2016 The Bitcoin Developers
// Copyright (c) 2015-2016 The Silk Network Developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.
#ifndef DARKSILK_HASH_H
#define DARKSILK_HASH_H

#include "uint256.h"
#include "serialize.h"

#include <openssl/sha.h>
#include <openssl/ripemd.h>

#include "crypto/ripemd160.h"
#include "crypto/blake256.h"
#include "crypto/argon2/argon2.h"
#include "crypto/sha256.h"
#include <vector>

static const unsigned int OUT_BYTES = 32;

/// A hasher class for DarkSilk's 256-bit hash (double SHA-256).
class CHash256 {
private:
    CSHA256 sha;
public:
    static const size_t OUTPUT_SIZE = CSHA256::OUTPUT_SIZE;

    void Finalize(unsigned char hash[OUTPUT_SIZE]) {
        unsigned char buf[sha.OUTPUT_SIZE];
        sha.Finalize(buf);
        sha.Reset().Write(buf, sha.OUTPUT_SIZE).Finalize(hash);
    }

    CHash256& Write(const unsigned char *data, size_t len) {
        sha.Write(data, len);
        return *this;
    }

    CHash256& Reset() {
        sha.Reset();
        return *this;
    }
};

/// A hasher class for DarkSilk's 160-bit hash (SHA-256 + RIPEMD-160).
class CHash160 {
private:
    CBLAKE2B256 blake;
public:
    static const size_t OUTPUT_SIZE = CBLAKE2B256::OUTPUT_SIZE;

    void Finalize(unsigned char hash[OUTPUT_SIZE]) {
        unsigned char buf[blake.OUTPUT_SIZE];
        blake.Finalize(buf);
        CRIPEMD160().Write(buf, blake.OUTPUT_SIZE).Finalize(hash);
    }

    CHash160& Write(const unsigned char *data, size_t len) {
        blake.Write(data, len);
        return *this;
    }

    CHash160& Reset() {
        blake.Reset();
        return *this;
    }
};

template<typename T1>
inline uint256 Hash(const T1 pbegin, const T1 pend)
{
    static unsigned char pblank[1];
    uint256 hash1;
    blake2b_state S[1];
    blake2b_init( S, OUT_BYTES );
    blake2b_update( S, (pbegin == pend ? pblank : (unsigned char*)&pbegin[0]), (pend - pbegin) * sizeof(pbegin[0]) );
    blake2b_final( S, (unsigned char*)&hash1, OUT_BYTES );
    return hash1;
}

template<typename T1>
inline uint256 HashBlake2b(const T1 pbegin, const T1 pend)
{
    return Hash(pbegin, pend);
}

/// Argon2d Parameters
/// Salt and password are the block header.
/// Output length: 32 bytes.
/// Input length (in the case of a block header): 80 bytes.
/// Salt length (same note as input length): 80 bytes.
/// Input: Block header
/// Salt: Block header (SAME AS INPUT)
/// Secret data: None
/// Secret length: 0
/// Associated data: None
/// Associated data length: 0
/// Memory cost: 1024
/// Lanes: 64
/// TODO (Amir): test hashing:
/// 0x01 bytes --> 31EC4BC6A98D7CDE7CE9A3B542519225403F5E4573C4BD2F3D210BD2D3B08C9D
inline int Argon2d_Hash(void *out, size_t outlen, const void *in, size_t inlen, const void *salt, size_t saltlen, unsigned int t_cost,
        unsigned int m_cost) {

    argon2_context context;

    if (inlen > UINT32_MAX) {
        return ARGON2_PWD_TOO_LONG;
    }

    if (outlen > UINT32_MAX) {
        return ARGON2_OUTPUT_TOO_LONG;
    }

    if (saltlen > UINT32_MAX) {
        return ARGON2_SALT_TOO_LONG;
    }

    context.out = (uint8_t *)out;
    context.outlen = (uint32_t)outlen;
    context.pwd = (uint8_t *)in;
    context.pwdlen = (uint32_t)inlen;
    context.salt = (uint8_t *)salt;
    context.saltlen = (uint32_t)saltlen;
    context.secret = NULL;
    context.secretlen = 0;
    context.ad = NULL;
    context.adlen = 0;
    context.t_cost = t_cost;
    context.m_cost = m_cost;
    context.lanes = 64;
    context.threads = 2;
    context.allocate_cbk = NULL;
    context.free_cbk = NULL;
    context.flags = ARGON2_DEFAULT_FLAGS;

    return argon2_core(&context, Argon2_d);
}

inline uint256 hashArgon2d(const void* input)
{
    unsigned int t_costs = 8;
    unsigned int m_costs = 1024;
    size_t inputlen = 80;
    uint256 result = 0;

    Argon2d_Hash((uint8_t*)&result, OUT_BYTES, (const uint8_t*)input, inputlen,
                    (const uint8_t*)input, inputlen,  t_costs, m_costs);

    return result;
}

class CHashWriter
{
private:
    const size_t size;
    blake2b_state S[1];

public:
    int nType;
    int nVersion;

    void Init() {
        blake2b_init(S, size);
    }

    CHashWriter(int nTypeIn, int nVersionIn) : size(OUT_BYTES), nType(nTypeIn), nVersion(nVersionIn) {
        Init();
    }

    CHashWriter& write(const char *pch, size_t sizeIn) {
        blake2b_update(S, pch, sizeIn);
        return (*this);
    }

    // invalidates the object
    uint256 GetHash() {
        uint256 hash1;
        blake2b_final( S, (unsigned char*)&hash1, size );
        return hash1;
    }

    template<typename T>
    CHashWriter& operator<<(const T& obj) {
        // Serialize to this stream
        ::Serialize(*this, obj, nType, nVersion);
        return (*this);
    }
};

template<typename T1, typename T2>
inline uint256 Hash(const T1 p1begin, const T1 p1end,
                    const T2 p2begin, const T2 p2end)
{
    static unsigned char pblank[1];
    uint256 hash1;
    blake2b_state S[1];
    blake2b_init( S, OUT_BYTES );
    blake2b_update( S, (p1begin == p1end ? pblank : (unsigned char*)&p1begin[0]), (p1end - p1begin) * sizeof(p1begin[0]) );
    blake2b_update( S, (p2begin == p2end ? pblank : (unsigned char*)&p2begin[0]), (p2end - p2begin) * sizeof(p2begin[0]) );
    blake2b_final( S, (unsigned char*)&hash1, OUT_BYTES );
    return hash1;
}

template<typename T1, typename T2, typename T3>
inline uint256 Hash(const T1 p1begin, const T1 p1end,
                    const T2 p2begin, const T2 p2end,
                    const T3 p3begin, const T3 p3end)
{
    static unsigned char pblank[1];
    uint256 hash1;
    blake2b_state S[1];
    blake2b_init( S, OUT_BYTES );
    blake2b_update(S, (p1begin == p1end ? pblank : (unsigned char*)&p1begin[0]), (p1end - p1begin) * sizeof(p1begin[0]));
    blake2b_update(S, (p2begin == p2end ? pblank : (unsigned char*)&p2begin[0]), (p2end - p2begin) * sizeof(p2begin[0]));
    blake2b_update(S, (p3begin == p3end ? pblank : (unsigned char*)&p3begin[0]), (p3end - p3begin) * sizeof(p3begin[0]));
    blake2b_final( S, (unsigned char*)&hash1, OUT_BYTES );
    return hash1;
}

template<typename T>
uint256 SerializeHash(const T& obj, int nType=SER_GETHASH, int nVersion=PROTOCOL_VERSION)
{
    CHashWriter ss(nType, nVersion);
    ss << obj;
    return ss.GetHash();
}

template<typename T1>
inline uint160 Hash160(const T1 pbegin, const T1 pend)
{
    static unsigned char pblank[1];
    uint256 hash1;
    blake2b_state S[1];
    blake2b_init( S, OUT_BYTES );
    blake2b_update( S, (pbegin == pend ? pblank : (unsigned char*)&pbegin[0]), (pend - pbegin) * sizeof(pbegin[0]) );
    blake2b_final( S, (unsigned char*)&hash1, OUT_BYTES );
    uint160 hash2;
    RIPEMD160((unsigned char*)&hash1, sizeof(hash1), (unsigned char*)&hash2);
    return hash2;
}

inline uint160 Hash160(const std::vector<unsigned char>& vch)
{
    return Hash160(vch.begin(), vch.end());
}

typedef struct
{
    SHA512_CTX ctxInner;
    SHA512_CTX ctxOuter;
} HMAC_SHA512_CTX;

int HMAC_SHA512_Init(HMAC_SHA512_CTX *pctx, const void *pkey, size_t len);
int HMAC_SHA512_Update(HMAC_SHA512_CTX *pctx, const void *pdata, size_t len);
int HMAC_SHA512_Final(unsigned char *pmd, HMAC_SHA512_CTX *pctx);

void BIP32Hash(const unsigned char chainCode[32], unsigned int nChild, unsigned char header, const unsigned char data[32], unsigned char output[64]); 

/// Compute the 256-bit hash of the concatenation of three objects.
template<typename T1, typename T2, typename T3, typename T4>
inline uint256 Hash(const T1 p1begin, const T1 p1end,
                    const T2 p2begin, const T2 p2end,
                    const T3 p3begin, const T3 p3end,
                    const T4 p4begin, const T4 p4end) {
    static const unsigned char pblank[1] = {};
    uint256 result;
    CHash256().Write(p1begin == p1end ? pblank : (const unsigned char*)&p1begin[0], (p1end - p1begin) * sizeof(p1begin[0]))
              .Write(p2begin == p2end ? pblank : (const unsigned char*)&p2begin[0], (p2end - p2begin) * sizeof(p2begin[0]))
              .Write(p3begin == p3end ? pblank : (const unsigned char*)&p3begin[0], (p3end - p3begin) * sizeof(p3begin[0]))
              .Write(p4begin == p4end ? pblank : (const unsigned char*)&p4begin[0], (p4end - p4begin) * sizeof(p4begin[0]))
              .Finalize((unsigned char*)&result);
    return result;
}

/// Compute the 256-bit hash of the concatenation of three objects.
template<typename T1, typename T2, typename T3, typename T4, typename T5>
inline uint256 Hash(const T1 p1begin, const T1 p1end,
                    const T2 p2begin, const T2 p2end,
                    const T3 p3begin, const T3 p3end,
                    const T4 p4begin, const T4 p4end,
                    const T5 p5begin, const T5 p5end) {
    static const unsigned char pblank[1] = {};
    uint256 result;
    CHash256().Write(p1begin == p1end ? pblank : (const unsigned char*)&p1begin[0], (p1end - p1begin) * sizeof(p1begin[0]))
              .Write(p2begin == p2end ? pblank : (const unsigned char*)&p2begin[0], (p2end - p2begin) * sizeof(p2begin[0]))
              .Write(p3begin == p3end ? pblank : (const unsigned char*)&p3begin[0], (p3end - p3begin) * sizeof(p3begin[0]))
              .Write(p4begin == p4end ? pblank : (const unsigned char*)&p4begin[0], (p4end - p4begin) * sizeof(p4begin[0]))
              .Write(p5begin == p5end ? pblank : (const unsigned char*)&p5begin[0], (p5end - p5begin) * sizeof(p5begin[0]))
              .Finalize((unsigned char*)&result);
    return result;
}

/// Compute the 256-bit hash of the concatenation of three objects.
template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
inline uint256 Hash(const T1 p1begin, const T1 p1end,
                    const T2 p2begin, const T2 p2end,
                    const T3 p3begin, const T3 p3end,
                    const T4 p4begin, const T4 p4end,
                    const T5 p5begin, const T5 p5end,
                    const T6 p6begin, const T6 p6end) {
    static const unsigned char pblank[1] = {};
    uint256 result;
    CHash256().Write(p1begin == p1end ? pblank : (const unsigned char*)&p1begin[0], (p1end - p1begin) * sizeof(p1begin[0]))
              .Write(p2begin == p2end ? pblank : (const unsigned char*)&p2begin[0], (p2end - p2begin) * sizeof(p2begin[0]))
              .Write(p3begin == p3end ? pblank : (const unsigned char*)&p3begin[0], (p3end - p3begin) * sizeof(p3begin[0]))
              .Write(p4begin == p4end ? pblank : (const unsigned char*)&p4begin[0], (p4end - p4begin) * sizeof(p4begin[0]))
              .Write(p5begin == p5end ? pblank : (const unsigned char*)&p5begin[0], (p5end - p5begin) * sizeof(p5begin[0]))
              .Write(p6begin == p6end ? pblank : (const unsigned char*)&p6begin[0], (p6end - p6begin) * sizeof(p6begin[0]))
              .Finalize((unsigned char*)&result);
    return result;
}

#endif
