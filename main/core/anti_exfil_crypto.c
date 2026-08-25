#include "anti_exfil_crypto.h"

#include "../utils/secure_mem.h"
#include <string.h>
#include <wally_anti_exfil.h>
#include <wally_crypto.h>

_Static_assert(ANTI_EXFIL_PRIVATE_KEY_LEN == EC_PRIVATE_KEY_LEN,
               "private key length mismatch");
_Static_assert(ANTI_EXFIL_MESSAGE_HASH_LEN == EC_MESSAGE_HASH_LEN,
               "message hash length mismatch");
_Static_assert(ANTI_EXFIL_HOST_COMMITMENT_LEN == WALLY_HOST_COMMITMENT_LEN,
               "host commitment length mismatch");
_Static_assert(ANTI_EXFIL_HOST_ENTROPY_LEN == WALLY_S2C_DATA_LEN,
               "host entropy length mismatch");
_Static_assert(ANTI_EXFIL_SIGNER_OPENING_LEN == WALLY_S2C_OPENING_LEN,
               "signer opening length mismatch");
_Static_assert(ANTI_EXFIL_COMPACT_SIGNATURE_LEN == EC_SIGNATURE_LEN,
               "compact signature length mismatch");

bool anti_exfil_signer_commit(const uint8_t *private_key,
                              const uint8_t *message_hash,
                              const uint8_t *host_commitment,
                              uint8_t *signer_opening) {
  if (!signer_opening)
    return false;

  secure_memzero(signer_opening, ANTI_EXFIL_SIGNER_OPENING_LEN);
  if (!private_key || !message_hash || !host_commitment)
    return false;
  if (wally_ec_private_key_verify(private_key, ANTI_EXFIL_PRIVATE_KEY_LEN) !=
      WALLY_OK)
    return false;

  const int result = wally_ae_signer_commit_from_bytes(
      private_key, ANTI_EXFIL_PRIVATE_KEY_LEN, message_hash,
      ANTI_EXFIL_MESSAGE_HASH_LEN, host_commitment,
      ANTI_EXFIL_HOST_COMMITMENT_LEN, EC_FLAG_ECDSA, signer_opening,
      ANTI_EXFIL_SIGNER_OPENING_LEN);
  if (result != WALLY_OK) {
    secure_memzero(signer_opening, ANTI_EXFIL_SIGNER_OPENING_LEN);
    return false;
  }
  return true;
}

bool anti_exfil_sign(const uint8_t *private_key, const uint8_t *message_hash,
                     const uint8_t *host_entropy, uint8_t *compact_signature) {
  if (!compact_signature)
    return false;

  secure_memzero(compact_signature, ANTI_EXFIL_COMPACT_SIGNATURE_LEN);
  if (!private_key || !message_hash || !host_entropy)
    return false;
  if (wally_ec_private_key_verify(private_key, ANTI_EXFIL_PRIVATE_KEY_LEN) !=
      WALLY_OK)
    return false;

  const int result = wally_ae_sig_from_bytes(
      private_key, ANTI_EXFIL_PRIVATE_KEY_LEN, message_hash,
      ANTI_EXFIL_MESSAGE_HASH_LEN, host_entropy, ANTI_EXFIL_HOST_ENTROPY_LEN,
      EC_FLAG_ECDSA, compact_signature, ANTI_EXFIL_COMPACT_SIGNATURE_LEN);
  if (result != WALLY_OK) {
    secure_memzero(compact_signature, ANTI_EXFIL_COMPACT_SIGNATURE_LEN);
    return false;
  }
  return true;
}

bool anti_exfil_crypto_self_test(void) {
  uint8_t private_key[ANTI_EXFIL_PRIVATE_KEY_LEN];
  uint8_t message_hash[ANTI_EXFIL_MESSAGE_HASH_LEN];
  uint8_t host_entropy[ANTI_EXFIL_HOST_ENTROPY_LEN];
  uint8_t host_commitment[ANTI_EXFIL_HOST_COMMITMENT_LEN] = {
      0x82, 0xc8, 0xc9, 0x66, 0x9a, 0xfb, 0x1c, 0xb4, 0x0d, 0xd4, 0xb6,
      0x2e, 0x6c, 0x61, 0x30, 0xd5, 0x78, 0x70, 0x60, 0x8d, 0x01, 0x72,
      0x59, 0x32, 0xd3, 0xbe, 0x71, 0xe2, 0x0b, 0x8a, 0x4d, 0x4a};
  uint8_t expected_opening[ANTI_EXFIL_SIGNER_OPENING_LEN] = {
      0x02, 0xf5, 0x2a, 0x1b, 0x78, 0x96, 0x21, 0x92, 0x89, 0xfb, 0x3a,
      0xbc, 0x84, 0x31, 0xd5, 0x76, 0x60, 0xd1, 0xce, 0xd3, 0x51, 0xca,
      0x86, 0x1e, 0x73, 0xe4, 0x2d, 0xb8, 0xae, 0x9e, 0x42, 0x85, 0x1f};
  uint8_t expected_signature[ANTI_EXFIL_COMPACT_SIGNATURE_LEN] = {
      0x3f, 0x49, 0xa7, 0x4c, 0xec, 0x28, 0xd6, 0x3a, 0x7a, 0x52, 0xe0,
      0x91, 0xa8, 0x17, 0x30, 0x45, 0xea, 0x49, 0xf3, 0x4a, 0xb1, 0xc0,
      0xae, 0xb1, 0x95, 0xe1, 0x86, 0xe2, 0x34, 0xe8, 0xb7, 0x74, 0x73,
      0x09, 0x2b, 0x7b, 0x63, 0x19, 0x33, 0xe9, 0x05, 0x74, 0x21, 0xf3,
      0xc8, 0x83, 0x98, 0x13, 0x47, 0xab, 0x7f, 0xc8, 0x22, 0xeb, 0x06,
      0x9a, 0xc5, 0x3c, 0xb9, 0x86, 0x5d, 0x2e, 0x8b, 0xb3};
  uint8_t opening[ANTI_EXFIL_SIGNER_OPENING_LEN];
  uint8_t signature[ANTI_EXFIL_COMPACT_SIGNATURE_LEN];
  bool ok;

  memset(private_key, 0x55, sizeof(private_key));
  memset(message_hash, 0x88, sizeof(message_hash));
  memset(host_entropy, 0xa5, sizeof(host_entropy));

  ok = anti_exfil_signer_commit(private_key, message_hash, host_commitment,
                                opening) &&
       secure_memcmp(opening, expected_opening, sizeof(opening)) == 0 &&
       anti_exfil_sign(private_key, message_hash, host_entropy, signature) &&
       secure_memcmp(signature, expected_signature, sizeof(signature)) == 0;

  secure_memzero(private_key, sizeof(private_key));
  secure_memzero(message_hash, sizeof(message_hash));
  secure_memzero(host_entropy, sizeof(host_entropy));
  secure_memzero(host_commitment, sizeof(host_commitment));
  secure_memzero(expected_opening, sizeof(expected_opening));
  secure_memzero(expected_signature, sizeof(expected_signature));
  secure_memzero(opening, sizeof(opening));
  secure_memzero(signature, sizeof(signature));
  return ok;
}
