/* Copyright (c) 2015, Vsevolod Stakhov
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *       * Redistributions of source code must retain the above copyright
 *         notice, this list of conditions and the following disclaimer.
 *       * Redistributions in binary form must reproduce the above copyright
 *         notice, this list of conditions and the following disclaimer in the
 *         documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED ''AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#ifndef CRYPTOBOX_H_
#define CRYPTOBOX_H_

#include "config.h"

struct rspamd_cryptobox_segment {
	guchar *data;
	gsize len;
};

#define rspamd_cryptobox_MAX_NONCEBYTES 24
#define rspamd_cryptobox_MAX_PKBYTES 65
#define rspamd_cryptobox_MAX_SKBYTES 32
#define rspamd_cryptobox_MAX_MACBYTES 16
#define rspamd_cryptobox_MAX_NMBYTES 32
#define rspamd_cryptobox_SIPKEYBYTES 16
#define rspamd_cryptobox_HASHBYTES 64
#define rspamd_cryptobox_HASHKEYBYTES 64
#define rspamd_cryptobox_HASHSTATEBYTES 256
#define rspamd_cryptobox_MAX_SIGSKBYTES 64
#define rspamd_cryptobox_MAX_SIGPKBYTES 32
#define rspamd_cryptobox_MAX_SIGBYTES 64

#define CPUID_AVX2 0x1
#define CPUID_AVX 0x2
#define CPUID_SSE2 0x4
#define CPUID_SSE3 0x8
#define CPUID_SSSE3 0x10
#define CPUID_SSE41 0x20
#define CPUID_RDRAND 0x40

typedef guchar rspamd_pk_t[rspamd_cryptobox_MAX_PKBYTES];
typedef guchar rspamd_sk_t[rspamd_cryptobox_MAX_SKBYTES];
typedef guchar rspamd_mac_t[rspamd_cryptobox_MAX_MACBYTES];
typedef guchar rspamd_nm_t[rspamd_cryptobox_MAX_NMBYTES];
typedef guchar rspamd_nonce_t[rspamd_cryptobox_MAX_NONCEBYTES];
typedef guchar rspamd_sipkey_t[rspamd_cryptobox_SIPKEYBYTES];

struct rspamd_cryptobox_library_ctx {
	gchar *cpu_extensions;
	const gchar *curve25519_impl;
	const gchar *chacha20_impl;
	const gchar *poly1305_impl;
	const gchar *siphash_impl;
	const gchar *blake2_impl;
	unsigned long cpu_config;
};

/**
 * Init cryptobox library
 */
struct rspamd_cryptobox_library_ctx* rspamd_cryptobox_init (void);

/**
 * Generate new keypair
 * @param pk public key buffer
 * @param sk secret key buffer
 */
void rspamd_cryptobox_keypair (rspamd_pk_t pk, rspamd_sk_t sk);

/**
 * Encrypt data inplace adding signature to sig afterwards
 * @param data input buffer
 * @param pk remote pubkey
 * @param sk local secret key
 * @param sig output signature
 */
void rspamd_cryptobox_encrypt_inplace (guchar *data, gsize len,
		const rspamd_nonce_t nonce,
		const rspamd_pk_t pk, const rspamd_sk_t sk, rspamd_mac_t sig);

/**
 * Encrypt segments of data inplace adding signature to sig afterwards
 * @param segments segments of data
 * @param cnt count of segments
 * @param pk remote pubkey
 * @param sk local secret key
 * @param sig output signature
 */
void rspamd_cryptobox_encryptv_inplace (struct rspamd_cryptobox_segment *segments,
		gsize cnt,
		const rspamd_nonce_t nonce,
		const rspamd_pk_t pk, const rspamd_sk_t sk, rspamd_mac_t sig);


/**
 * Decrypt and verify data chunk inplace
 * @param data data to decrypt
 * @param len lenght of data
 * @param pk remote pubkey
 * @param sk local privkey
 * @param sig signature input
 * @return TRUE if input has been verified successfully
 */
gboolean rspamd_cryptobox_decrypt_inplace (guchar *data, gsize len,
		const rspamd_nonce_t nonce,
		const rspamd_pk_t pk, const rspamd_sk_t sk, const rspamd_mac_t sig);

/**
 * Encrypt segments of data inplace adding signature to sig afterwards
 * @param segments segments of data
 * @param cnt count of segments
 * @param pk remote pubkey
 * @param sk local secret key
 * @param sig output signature
 */
void rspamd_cryptobox_encrypt_nm_inplace (guchar *data, gsize len,
		const rspamd_nonce_t nonce,
		const rspamd_nm_t nm, rspamd_mac_t sig);

/**
 * Encrypt segments of data inplace adding signature to sig afterwards
 * @param segments segments of data
 * @param cnt count of segments
 * @param pk remote pubkey
 * @param sk local secret key
 * @param sig output signature
 */
void rspamd_cryptobox_encryptv_nm_inplace (struct rspamd_cryptobox_segment *segments,
		gsize cnt,
		const rspamd_nonce_t nonce,
		const rspamd_nm_t nm, rspamd_mac_t sig);


/**
 * Decrypt and verify data chunk inplace
 * @param data data to decrypt
 * @param len lenght of data
 * @param pk remote pubkey
 * @param sk local privkey
 * @param sig signature input
 * @return TRUE if input has been verified successfully
 */
gboolean rspamd_cryptobox_decrypt_nm_inplace (guchar *data, gsize len,
		 const rspamd_nonce_t nonce,
		 const rspamd_nm_t nm, const rspamd_mac_t sig);

/**
 * Generate shared secret from local sk and remote pk
 * @param nm shared secret
 * @param pk remote pubkey
 * @param sk local privkey
 */
void rspamd_cryptobox_nm (rspamd_nm_t nm, const rspamd_pk_t pk, const rspamd_sk_t sk);

/**
 * Securely clear the buffer specified
 * @param buf buffer to zero
 * @param buflen length of buffer
 */
void rspamd_explicit_memzero (void * const buf, gsize buflen);

/**
 * Calculates siphash-2-4 for a message
 * @param out (8 bytes output)
 * @param in
 * @param inlen
 * @param k key (must be 16 bytes)
 */
void rspamd_cryptobox_siphash (unsigned char *out, const unsigned char *in,
		unsigned long long inlen,
		const rspamd_sipkey_t k);

/**
 * Derive key from password using PKCS#5 and HMAC-blake2
 * @param pass input password
 * @param pass_len length of the password
 * @param salt input salt
 * @param salt_len length of salt
 * @param key output key
 * @param key_len size of the key
 * @param rounds number of rounds (should be reasonably high)
 * @return TRUE in case of success and FALSE if failed
 */
gboolean rspamd_cryptobox_pbkdf(const char *pass, gsize pass_len,
		const guint8 *salt, gsize salt_len, guint8 *key, gsize key_len,
		unsigned int rounds);

/**
 * Enable openssl mode in rspamd_cryptobox
 * @param enable if TRUE then crypto code will use openssl, chacha20/poly1305 otherwize
 */
gboolean rspamd_cryptobox_openssl_mode (gboolean enable);

/**
 * Real size of rspamd cryptobox public key
 */
guint rspamd_cryptobox_pk_bytes (void);

/**
 * Real size of crypto nonce
 */
guint rspamd_cryptobox_nonce_bytes (void);

/**
 * Real size of rspamd cryptobox secret key
 */
guint rspamd_cryptobox_sk_bytes (void);

/**
 * Real size of rspamd cryptobox shared key
 */
guint rspamd_cryptobox_nm_bytes (void);

/**
 * Real size of rspamd cryptobox MAC signature
 */
guint rspamd_cryptobox_mac_bytes (void);

/* Hash IUF interface */
typedef struct RSPAMD_ALIGNED(32) rspamd_cryptobox_hash_state_s  {
	unsigned char opaque[256];
} rspamd_cryptobox_hash_state_t;

/**
 * Init cryptobox hash state using key if needed, `st` must point to the buffer
 * with at least rspamd_cryptobox_HASHSTATEBYTES bytes length. If keylen == 0, then
 * non-keyed hash is generated
 */
void rspamd_cryptobox_hash_init (void *st, const guchar *key, gsize keylen);

/**
 * Update hash with data portion
 */
void rspamd_cryptobox_hash_update (void *st, const guchar *data, gsize len);

/**
 * Output hash to the buffer of rspamd_cryptobox_HASHBYTES length
 */
void rspamd_cryptobox_hash_final (void *st, guchar *out);

/**
 * One in all function
 */
void rspamd_cryptobox_hash (guchar *out,
		const guchar *data,
		gsize len,
		const guchar *key,
		gsize keylen);

#endif /* CRYPTOBOX_H_ */
