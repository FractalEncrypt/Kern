#include "core/anti_exfil_crypto.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <wally_core.h>

static bool is_all_zero(const uint8_t *bytes, size_t len) {
  uint8_t aggregate = 0;
  for (size_t i = 0; i < len; ++i)
    aggregate |= bytes[i];
  return aggregate == 0;
}

int main(void) {
  uint8_t private_key[ANTI_EXFIL_PRIVATE_KEY_LEN];
  uint8_t message_hash[ANTI_EXFIL_MESSAGE_HASH_LEN];
  uint8_t commitment[ANTI_EXFIL_HOST_COMMITMENT_LEN] = {
      0x1b, 0xf6, 0xfb, 0x42, 0xf4, 0x1e, 0xb8, 0x76, 0xc4, 0xd7, 0xaa,
      0x0d, 0x67, 0x24, 0x2b, 0x00, 0xba, 0xab, 0x99, 0xdc, 0x20, 0x84,
      0x49, 0x3e, 0x4e, 0x63, 0x27, 0x7f, 0xa1, 0xf7, 0x7f, 0x22};
  const uint8_t expected_opening[ANTI_EXFIL_SIGNER_OPENING_LEN] = {
      0x02, 0xdf, 0x63, 0x75, 0x5d, 0x1f, 0x32, 0x92, 0xbf, 0xfe, 0xd8,
      0x29, 0x86, 0xb1, 0x06, 0x49, 0x7c, 0x93, 0xb1, 0xf8, 0xbd, 0xc0,
      0x45, 0x4b, 0x6b, 0x0b, 0x0a, 0x47, 0x79, 0xc0, 0xef, 0x71, 0x88};
  uint8_t output[ANTI_EXFIL_COMPACT_SIGNATURE_LEN];

  memset(private_key, 0x55, sizeof(private_key));
  memset(message_hash, 0x88, sizeof(message_hash));
  memset(output, 0xa5, sizeof(output));

  assert(wally_init(0) == WALLY_OK);
  assert(
      anti_exfil_signer_commit(private_key, message_hash, commitment, output));
  assert(memcmp(output, expected_opening, sizeof(expected_opening)) == 0);
  assert(anti_exfil_crypto_self_test());

  memset(private_key, 0, sizeof(private_key));
  memset(output, 0xa5, ANTI_EXFIL_SIGNER_OPENING_LEN);
  assert(
      !anti_exfil_signer_commit(private_key, message_hash, commitment, output));
  assert(is_all_zero(output, ANTI_EXFIL_SIGNER_OPENING_LEN));

  memset(output, 0xa5, sizeof(output));
  assert(!anti_exfil_sign(private_key, message_hash, commitment, output));
  assert(is_all_zero(output, sizeof(output)));

  memset(output, 0xa5, ANTI_EXFIL_SIGNER_OPENING_LEN);
  assert(!anti_exfil_signer_commit(NULL, message_hash, commitment, output));
  assert(is_all_zero(output, ANTI_EXFIL_SIGNER_OPENING_LEN));

  memset(output, 0xa5, sizeof(output));
  assert(!anti_exfil_sign(NULL, message_hash, commitment, output));
  assert(is_all_zero(output, sizeof(output)));
  assert(!anti_exfil_sign(private_key, message_hash, commitment, NULL));

  wally_cleanup(0);
  puts("anti-exfil crypto tests passed");
  return 0;
}
