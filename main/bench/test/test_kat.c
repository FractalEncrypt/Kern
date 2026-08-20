/*
 * Host-side known-answer test for the SLH-DSA-SHA2-128s port.
 *
 * The reference implementation is portable C with no ESP-IDF dependency, so
 * the same vectors and the same calls the on-device gate uses can run here.
 * That leaves only device-specific behaviour -- timing, stack, watchdog -- to
 * be confirmed on hardware.
 */

#include <stdio.h>
#include <string.h>

#include "../kat_slhdsa_sha2_128s.h"
#include "slh_dsa.h"

static int failures = 0;

static void check(const char *what, int pass) {
  printf("  %-28s %s\n", what, pass ? "PASS" : "FAIL");
  if (!pass)
    failures++;
}

int main(void) {
  const slh_param_t *prm = &slh_dsa_sha2_128s;
  static uint8_t sk[64], pk[32], sig[7856];

  printf("SLH-DSA-SHA2-128s KAT (%s)\n", slh_alg_id(prm));

  check("sizes", slh_sk_sz(prm) == sizeof(sk) && slh_pk_sz(prm) == sizeof(pk) &&
                     slh_sig_sz(prm) == sizeof(sig));

  for (int i = 0; i < KAT_KEYGEN_COUNT; i++) {
    const kat_keygen_t *t = &kat_keygen[i];
    char label[48];
    memset(sk, 0, sizeof(sk));
    memset(pk, 0, sizeof(pk));
    int rc =
        slh_keygen_internal(sk, pk, t->sk_seed, t->sk_prf, t->pk_seed, prm);
    snprintf(label, sizeof(label), "keyGen tcId %d", t->tc_id);
    check(label, rc == 0 && memcmp(sk, t->sk, t->sk_len) == 0 &&
                     memcmp(pk, t->pk, t->pk_len) == 0);
  }

  memset(sig, 0, sizeof(sig));
  size_t n = slh_sign_internal(sig, kat_sg_int0_msg, sizeof(kat_sg_int0_msg),
                               kat_sg_int0_sk, NULL, prm);
  check("sigGen internal (deterministic)",
        n == sizeof(kat_sg_int0_sig) && memcmp(sig, kat_sg_int0_sig, n) == 0);

  memset(sig, 0, sizeof(sig));
  n = slh_sign(sig, kat_sg_ext0_msg, sizeof(kat_sg_ext0_msg), kat_sg_ext0_ctx,
               sizeof(kat_sg_ext0_ctx), kat_sg_ext0_sk, NULL, prm);
  check("sigGen external (with ctx)",
        n == sizeof(kat_sg_ext0_sig) && memcmp(sig, kat_sg_ext0_sig, n) == 0);

  printf(failures ? "\n%d check(s) FAILED\n" : "\nall checks passed\n",
         failures);
  return failures ? 1 : 0;
}
