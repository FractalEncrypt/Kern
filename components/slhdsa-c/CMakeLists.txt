# SLH-DSA (FIPS 205) reference implementation, benchmark-only.
#
# Sources come from the unmodified pq-code-package/slhdsa-c submodule under
# upstream/. Only the SHA-2 instantiation is built: SLH-DSA-SHA2-128s needs no
# SHAKE, so slh_shake.c, sha3_*.c and slh_prehash.c are left out. sha2_512.c is
# still required -- slh_sha2.c unconditionally defines the sha2_512_* hooks used
# by the 192/256 parameter sets, and slh_var_s embeds a sha2_512_t.
#
# Gated off by default so it never reaches a release build.

set(srcs "")
if(CONFIG_KERN_BENCH_HBS)
    list(APPEND srcs
        "upstream/slh_dsa.c"
        "upstream/slh_sha2.c"
        "upstream/sha2_256.c"
        "upstream/sha2_512.c"
    )
endif()

idf_component_register(
    SRCS ${srcs}
    INCLUDE_DIRS "upstream"
)
