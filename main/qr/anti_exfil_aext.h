/* Detached AEXT v1 package and canonical CBOR byte-string adapter. */

#ifndef KERN_QR_ANTI_EXFIL_AEXT_H
#define KERN_QR_ANTI_EXFIL_AEXT_H

#include "anti_exfil_aexb.h"

#define ANTI_EXFIL_AEXT_UR_TYPE "x-btc-anti-exfil"
#define ANTI_EXFIL_AEXT_HEADER_LEN 48u
#define ANTI_EXFIL_AEXT_MAX_PSBT_LEN 2000000u
#define ANTI_EXFIL_AEXT_MAX_PACKAGE_LEN                                      \
  (ANTI_EXFIL_AEXT_HEADER_LEN + ANTI_EXFIL_AEXB_MAX_LEN +                    \
   ANTI_EXFIL_AEXT_MAX_PSBT_LEN)

/* psbt points into the caller-owned encoded package and is never copied. */
typedef struct {
  anti_exfil_message_t message;
  const uint8_t *psbt;
  size_t psbt_len;
} anti_exfil_aext_view_t;

size_t anti_exfil_aext_encoded_len(const anti_exfil_message_t *message,
                                   size_t psbt_len);

anti_exfil_result_t anti_exfil_aext_encode(
    const anti_exfil_message_t *message, const uint8_t *psbt, size_t psbt_len,
    uint8_t *output, size_t output_capacity, size_t *output_len);

anti_exfil_result_t anti_exfil_aext_decode(const uint8_t *encoded,
                                           size_t encoded_len,
                                           anti_exfil_aext_view_t *view);

size_t anti_exfil_aext_cbor_encoded_len(size_t package_len);

anti_exfil_result_t anti_exfil_aext_cbor_encode(
    const uint8_t *package, size_t package_len, uint8_t *output,
    size_t output_capacity, size_t *output_len);

/* package points into the caller-owned CBOR buffer. */
anti_exfil_result_t anti_exfil_aext_cbor_decode(
    const uint8_t *cbor, size_t cbor_len, const uint8_t **package,
    size_t *package_len);

#endif // KERN_QR_ANTI_EXFIL_AEXT_H
