/* x-btc-anti-exfil UR routing around the canonical AEXT/CBOR adapter. */

#ifndef KERN_QR_ANTI_EXFIL_UR_H
#define KERN_QR_ANTI_EXFIL_UR_H

#include "anti_exfil_aext.h"
#include "ur_decoder.h"
#include "ur_encoder.h"

#define ANTI_EXFIL_UR_MIN_FRAGMENT_LEN 10u
#define ANTI_EXFIL_UR_MAX_CBOR_LEN UR_MAX_MESSAGE_LEN

anti_exfil_result_t anti_exfil_ur_decode_result(
    const ur_result_t *result, anti_exfil_network_t expected_network,
    anti_exfil_stage_t expected_stage, anti_exfil_aext_view_t *view);

/*
 * canonical_cbor must contain exactly one canonical AEXT byte string.
 * scratch is cleared before return and keeps the ~30 KiB record off stack.
 */
anti_exfil_result_t anti_exfil_ur_encoder_create(
    const uint8_t *canonical_cbor, size_t cbor_len,
    anti_exfil_network_t expected_network, anti_exfil_stage_t expected_stage,
    size_t max_fragment_len, anti_exfil_aext_view_t *scratch,
    ur_encoder_t **encoder);

#endif // KERN_QR_ANTI_EXFIL_UR_H
