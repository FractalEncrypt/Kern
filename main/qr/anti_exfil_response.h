/* Policy-neutral signer response and animated UR ownership. */

#ifndef KERN_QR_ANTI_EXFIL_RESPONSE_H
#define KERN_QR_ANTI_EXFIL_RESPONSE_H

#include "anti_exfil_request.h"

#include <stdbool.h>

typedef struct anti_exfil_response anti_exfil_response_t;

/*
 * Perform the stateless signer transformation for an already retained
 * message-1 or message-3 request and create its canonical animated response.
 *
 * This function performs signing. The caller must invoke it only after the
 * future workflow has applied its setting and explicit user-approval policy.
 * No such policy is selected here. The request is borrowed and remains owned
 * by the caller. On every failure, *response_out is NULL.
 */
anti_exfil_result_t anti_exfil_response_create(
    const anti_exfil_request_t *request, size_t max_fragment_len,
    anti_exfil_response_t **response_out);

anti_exfil_network_t
anti_exfil_response_network(const anti_exfil_response_t *response);

anti_exfil_stage_t
anti_exfil_response_stage(const anti_exfil_response_t *response);

/* Canonical AEXT CBOR retained for transcript evidence and exact tests. */
const uint8_t *anti_exfil_response_cbor(const anti_exfil_response_t *response,
                                        size_t *cbor_len_out);

size_t anti_exfil_response_ur_part_count(
    const anti_exfil_response_t *response);

/* Caller owns the returned cUR string and must free it. */
anti_exfil_result_t anti_exfil_response_next_part(
    anti_exfil_response_t *response, char **part_out);

void anti_exfil_response_destroy(anti_exfil_response_t **response);

#endif // KERN_QR_ANTI_EXFIL_RESPONSE_H
