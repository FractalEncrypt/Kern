# Host sanitizer toolchain

The authoritative CI sanitizer lane uses Clang 18.1.8 from
`silkeh/clang@sha256:3914c93a02e866795aafc80737488e515b96390eff3d2787cf8c5095997baea9`.
The committed `ci/clang18-toolchain.Dockerfile` adds only zlib development files
copied from the digest-pinned GCC 11.4 image; its build performs no package or
network-dependent installation.

Run the Clang lane locally with:

```sh
./scripts/run-pinned-toolchain.sh sanitize-clang /absolute/output/path 2
```

The GCC 11.4 continuity baseline is separately reproducible with:

```sh
./scripts/run-pinned-toolchain.sh sanitize-gcc /absolute/output/path 2
```

The GCC image is
`gcc@sha256:8e6d66e2c6bd07552d7a9fc1e3f17ebc64fb4e92c122f6d793e0fd079c0cf417`.
Both lanes compile with `-g -O1 -fno-omit-frame-pointer -fsanitize=address`,
enable leak detection, and run the same out-of-bounds, use-after-free, and leak
positive controls before any real test target is accepted.

## Classification contract

Every real executable is linked through a test-only `main` wrapper which emits
`KERN_SANITIZER_MAIN_ENTERED`. Raw output and a JSON summary distinguish:

- `SANITIZER_INFRASTRUCTURE_STARTUP_FAILURE`: the marker was not reached;
- `SANITIZER_POSITIVE_CONTROL_FAILURE`: a deliberate defect was not detected;
- `PRODUCT_SANITIZER_FINDING`: ASan/LSan reported after the marker;
- `PRODUCT_TEST_FAILURE`: a build, assertion, timeout, or other test failure; and
- `PASS`: the runtime started, controls worked, and the target passed.

The scripts only read `vm.mmap_rnd_bits` and `randomize_va_space`. They never
modify ASLR or any host security setting, never retry a failed process, and fail
the job if the sanitizer runtime cannot start.

## Coverage boundary

The lane builds and executes the host tests in `components/deflate_codec/test`,
`components/bbqr/test`, and `main/core/test`, including all current anti-exfil
crypto, semantic, slot, signer, transport, and response targets. It does not
claim coverage for ESP32-only code paths, board drivers, display/camera paths,
or firmware-only integrations that these host harnesses cannot execute.
