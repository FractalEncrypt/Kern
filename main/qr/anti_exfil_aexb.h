/* Canonical AEXB v1 codec for transport-neutral anti-exfil records. */

#ifndef KERN_QR_ANTI_EXFIL_AEXB_H
#define KERN_QR_ANTI_EXFIL_AEXB_H

#include "core/anti_exfil/anti_exfil_types.h"

#define ANTI_EXFIL_AEXB_HEADER_LEN 78u
#define ANTI_EXFIL_AEXB_MAX_LEN 65536u

size_t anti_exfil_aexb_encoded_len(const anti_exfil_message_t *message);

anti_exfil_result_t anti_exfil_aexb_encode(
    const anti_exfil_message_t *message, uint8_t *output,
    size_t output_capacity, size_t *output_len);

anti_exfil_result_t anti_exfil_aexb_decode(
    const uint8_t *encoded, size_t encoded_len, anti_exfil_message_t *message);

#endif // KERN_QR_ANTI_EXFIL_AEXB_H
