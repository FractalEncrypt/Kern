/* Narrow signer-side wrapper for libwally ECDSA-S2C/anti-exfil primitives. */

#ifndef KERN_ANTI_EXFIL_CRYPTO_H
#define KERN_ANTI_EXFIL_CRYPTO_H

#include <stdbool.h>
#include <stdint.h>

#define ANTI_EXFIL_PRIVATE_KEY_LEN 32
#define ANTI_EXFIL_MESSAGE_HASH_LEN 32
#define ANTI_EXFIL_HOST_COMMITMENT_LEN 32
#define ANTI_EXFIL_HOST_ENTROPY_LEN 32
#define ANTI_EXFIL_SIGNER_OPENING_LEN 33
#define ANTI_EXFIL_COMPACT_SIGNATURE_LEN 64

/*
 * Compute the deterministic signer opening for message 2.
 *
 * The caller retains ownership of private_key and must wipe it after use.
 * signer_opening is cleared on every failure.
 */
bool anti_exfil_signer_commit(const uint8_t *private_key,
                              const uint8_t *message_hash,
                              const uint8_t *host_commitment,
                              uint8_t *signer_opening);

/*
 * Produce the compact protected signature for message 4.
 *
 * The caller retains ownership of private_key and host_entropy and must wipe
 * them after use. compact_signature is cleared on every failure.
 */
bool anti_exfil_sign(const uint8_t *private_key, const uint8_t *message_hash,
                     const uint8_t *host_entropy, uint8_t *compact_signature);

/* Run the pinned public known-answer vector and wipe all stack buffers. */
bool anti_exfil_crypto_self_test(void);

#endif // KERN_ANTI_EXFIL_CRYPTO_H
