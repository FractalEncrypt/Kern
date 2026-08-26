/* Owned, policy-neutral handoff for a decoded x-btc-anti-exfil request. */

#ifndef KERN_QR_ANTI_EXFIL_REQUEST_H
#define KERN_QR_ANTI_EXFIL_REQUEST_H

#include "anti_exfil_ur.h"

typedef struct anti_exfil_request anti_exfil_request_t;

/*
 * Copy the exact canonical CBOR bytes out of the scanner-owned UR result and
 * decode the AEXT view against that copy. No setting, network, stage, retry,
 * or slot-coverage policy is applied here.
 */
anti_exfil_result_t
anti_exfil_request_create(const ur_result_t *result,
                          anti_exfil_request_t **request_out);

/* The returned pointers remain valid until anti_exfil_request_destroy(). */
const anti_exfil_aext_view_t *
anti_exfil_request_view(const anti_exfil_request_t *request);

const uint8_t *anti_exfil_request_cbor(const anti_exfil_request_t *request,
                                       size_t *cbor_len_out);

/* Heap payload retained by the handoff, excluding allocator metadata. */
size_t anti_exfil_request_retained_bytes(const anti_exfil_request_t *request);

/* Wipe the transport copy and decoded record, free, and NULL the owner. */
void anti_exfil_request_destroy(anti_exfil_request_t **request);

#endif // KERN_QR_ANTI_EXFIL_REQUEST_H
