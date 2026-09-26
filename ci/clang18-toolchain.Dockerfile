ARG GCC_BASE_IMAGE=gcc@sha256:8e6d66e2c6bd07552d7a9fc1e3f17ebc64fb4e92c122f6d793e0fd079c0cf417
ARG CLANG_BASE_IMAGE=silkeh/clang@sha256:3914c93a02e866795aafc80737488e515b96390eff3d2787cf8c5095997baea9

FROM ${GCC_BASE_IMAGE} AS zlib_development_files

FROM ${CLANG_BASE_IMAGE}

# The Clang base already contains the exact same zlib runtime. Copy only the
# headers and static linker input from the digest-pinned GCC image, then create
# the conventional development symlink. No package repository is contacted.
COPY --from=zlib_development_files /usr/include/zlib.h /usr/include/zconf.h /usr/include/
COPY --from=zlib_development_files /usr/lib/x86_64-linux-gnu/libz.a /usr/lib/x86_64-linux-gnu/
RUN ln -s libz.so.1 /usr/lib/x86_64-linux-gnu/libz.so

LABEL org.opencontainers.image.title="Kern Clang 18 host-test toolchain" \
      org.opencontainers.image.description="Digest-pinned Clang/clang-format 18.1.8 with zlib development files"
