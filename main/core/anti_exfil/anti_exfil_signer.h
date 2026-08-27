/* Transport-free, stateless two-round anti-exfil signer. */

#ifndef KERN_ANTI_EXFIL_SIGNER_H
#define KERN_ANTI_EXFIL_SIGNER_H

#include "anti_exfil_slots.h"
#include "anti_exfil_types.h"

/*
 * Preflight performs every semantic, PSBT, key-ownership, sighash, and exact
 * slot-set check required before user review. It never derives an opening or
 * creates a signature. scratch is cleared before return on success or failure.
 */
anti_exfil_result_t anti_exfil_signer_preflight(
    const anti_exfil_message_t *input, const uint8_t *psbt_bytes,
    size_t psbt_bytes_len, anti_exfil_slot_set_t *scratch);

/*
 * All pointers must be distinct. The large output and scratch records belong
 * in caller-owned static or heap storage, never on an ESP-IDF task stack.
 * output and scratch are cleared on every failure.
 */
anti_exfil_result_t anti_exfil_signer_prepare(
    const anti_exfil_message_t *host_commit, const uint8_t *psbt_bytes,
    size_t psbt_bytes_len, anti_exfil_message_t *output,
    anti_exfil_slot_set_t *scratch);

/*
 * Complete message 3 without cached process state: re-enumerate the PSBT,
 * rederive every key, and deterministically recompute every accepted opening.
 * Session continuity and exact-session retry are coordinator responsibilities;
 * this stateless signer validates and echoes the supplied session_id.
 * psbt_bytes must be the exact frozen bytes, never a re-serialized equivalent.
 * No PSBT or signed-PSBT output is produced.
 */
anti_exfil_result_t anti_exfil_signer_complete(
    const anti_exfil_message_t *host_reveal, const uint8_t *psbt_bytes,
    size_t psbt_bytes_len, anti_exfil_message_t *output,
    anti_exfil_slot_set_t *scratch);

#endif // KERN_ANTI_EXFIL_SIGNER_H
