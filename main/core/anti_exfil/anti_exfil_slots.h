/* Authoritative PSBT-v0 signing-slot enumeration for anti-exfil protocol v1. */

#ifndef KERN_ANTI_EXFIL_SLOTS_H
#define KERN_ANTI_EXFIL_SLOTS_H

#include "anti_exfil_types.h"

#define ANTI_EXFIL_MAX_DERIVATION_DEPTH 10

typedef enum {
  ANTI_EXFIL_SCRIPT_P2WPKH = 1,
  ANTI_EXFIL_SCRIPT_P2SH_P2WPKH = 2,
  ANTI_EXFIL_SCRIPT_P2WSH_MULTISIG = 3,
  ANTI_EXFIL_SCRIPT_P2SH_P2WSH_MULTISIG = 4,
} anti_exfil_script_kind_t;

typedef struct {
  uint32_t input_index;
  uint8_t signer_pubkey[ANTI_EXFIL_PUBKEY_LEN];
  uint8_t message_hash[ANTI_EXFIL_MESSAGE_HASH_LEN];
  uint32_t sighash_type;
  uint32_t derivation_path[ANTI_EXFIL_MAX_DERIVATION_DEPTH];
  size_t derivation_path_len;
  anti_exfil_script_kind_t script_kind;
} anti_exfil_signing_slot_t;

/* Embedded callers must allocate this owned result statically or on the heap. */
typedef struct {
  anti_exfil_network_t network;
  uint8_t psbt_digest[ANTI_EXFIL_PSBT_DIGEST_LEN];
  size_t slot_count;
  anti_exfil_signing_slot_t slots[ANTI_EXFIL_MAX_SLOTS];
} anti_exfil_slot_set_t;

/*
 * Parse an exact canonical PSBT v0, validate every input, and enumerate every
 * ECDSA signing slot controlled by Kern's currently loaded key. The result is
 * cleared on every failure, so no partial slot set can escape.
 */
anti_exfil_result_t anti_exfil_slots_enumerate(
    const uint8_t *psbt_bytes, size_t psbt_bytes_len,
    anti_exfil_network_t network, anti_exfil_slot_set_t *out);

#endif // KERN_ANTI_EXFIL_SLOTS_H
