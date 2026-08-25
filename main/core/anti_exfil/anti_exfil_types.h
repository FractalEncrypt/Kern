/* Transport-neutral, bounded records for anti-exfil protocol v1. */

#ifndef KERN_ANTI_EXFIL_TYPES_H
#define KERN_ANTI_EXFIL_TYPES_H

#include <stddef.h>
#include <stdint.h>

#define ANTI_EXFIL_PROTOCOL_VERSION 1
#define ANTI_EXFIL_MAX_SLOTS 128
#define ANTI_EXFIL_MAX_SLOTS_PER_INPUT 16
#define ANTI_EXFIL_SESSION_ID_LEN 32
#define ANTI_EXFIL_PSBT_DIGEST_LEN 32
#define ANTI_EXFIL_PUBKEY_LEN 33
#define ANTI_EXFIL_MESSAGE_HASH_LEN 32
#define ANTI_EXFIL_HOST_COMMITMENT_LEN 32
#define ANTI_EXFIL_OPENING_LEN 33
#define ANTI_EXFIL_HOST_REVEAL_LEN 32
#define ANTI_EXFIL_SIGNATURE_LEN 64
#define ANTI_EXFIL_SIGHASH_ALL 1

/* Values are stable conformance identifiers. Do not renumber. */
typedef enum {
  ANTI_EXFIL_OK = 0,
  ANTI_EXFIL_INVALID_MESSAGE = 1,
  ANTI_EXFIL_WRONG_STAGE = 2,
  ANTI_EXFIL_SESSION_MISMATCH = 3,
  ANTI_EXFIL_TRANSACTION_MISMATCH = 4,
  ANTI_EXFIL_SIGNATURE_SLOT_MISMATCH = 5,
  ANTI_EXFIL_COMMITMENT_MISMATCH = 6,
  ANTI_EXFIL_OPENING_MISMATCH = 7,
  ANTI_EXFIL_SIGNATURE_INVALID = 8,
  ANTI_EXFIL_RETRY_CONFLICT = 9,
  ANTI_EXFIL_STATE_INVALID = 10,
  ANTI_EXFIL_OUTPUT_EXISTS = 11,
  ANTI_EXFIL_TEST_KEY_MISMATCH = 12,
  ANTI_EXFIL_UNEXPECTED_RETURN_DATA = 13,
  ANTI_EXFIL_NATIVE_BACKEND = 14,
} anti_exfil_result_t;

typedef enum {
  ANTI_EXFIL_NETWORK_MAINNET = 0,
  ANTI_EXFIL_NETWORK_TESTNET3 = 1,
  ANTI_EXFIL_NETWORK_REGTEST = 2,
  ANTI_EXFIL_NETWORK_SIGNET = 3,
  ANTI_EXFIL_NETWORK_TESTNET4 = 4,
} anti_exfil_network_t;

typedef enum {
  ANTI_EXFIL_STAGE_HOST_COMMIT = 1,
  ANTI_EXFIL_STAGE_SIGNER_OPENINGS = 2,
  ANTI_EXFIL_STAGE_HOST_REVEAL = 3,
  ANTI_EXFIL_STAGE_SIGNER_SIGNATURES = 4,
} anti_exfil_stage_t;

enum {
  ANTI_EXFIL_FIELD_OPENING = 1u << 0,
  ANTI_EXFIL_FIELD_HOST_REVEAL = 1u << 1,
  ANTI_EXFIL_FIELD_SIGNATURE = 1u << 2,
};

typedef struct {
  uint32_t input_index;
  uint32_t sighash_type;
  uint8_t signer_pubkey[ANTI_EXFIL_PUBKEY_LEN];
  uint8_t message_hash[ANTI_EXFIL_MESSAGE_HASH_LEN];
  uint8_t host_commitment[ANTI_EXFIL_HOST_COMMITMENT_LEN];
  uint8_t opening[ANTI_EXFIL_OPENING_LEN];
  uint8_t host_reveal[ANTI_EXFIL_HOST_REVEAL_LEN];
  uint8_t signature[ANTI_EXFIL_SIGNATURE_LEN];
  uint8_t present_fields;
} anti_exfil_slot_t;

/*
 * This fixed-capacity record is roughly 30 KiB. Embedded callers must keep it
 * in an owned static or heap allocation, never on an ESP-IDF task stack.
 */
typedef struct {
  uint8_t version;
  anti_exfil_network_t network;
  anti_exfil_stage_t stage;
  uint8_t flags;
  uint8_t session_id[ANTI_EXFIL_SESSION_ID_LEN];
  uint8_t psbt_digest[ANTI_EXFIL_PSBT_DIGEST_LEN];
  size_t slot_count;
  anti_exfil_slot_t slots[ANTI_EXFIL_MAX_SLOTS];
} anti_exfil_message_t;

const char *anti_exfil_result_name(anti_exfil_result_t result);

#endif // KERN_ANTI_EXFIL_TYPES_H
