ARG GCC_BASE_IMAGE=gcc@sha256:8e6d66e2c6bd07552d7a9fc1e3f17ebc64fb4e92c122f6d793e0fd079c0cf417
ARG CLANG_BASE_IMAGE=silkeh/clang@sha256:3914c93a02e866795aafc80737488e515b96390eff3d2787cf8c5095997baea9

FROM ${GCC_BASE_IMAGE} AS zlib_development_files

COPY ci/.toolchain-cache/libmbedtls-dev_2.28.3-1_amd64.deb /tmp/
COPY ci/.toolchain-cache/libmbedtls14_2.28.3-1_amd64.deb /tmp/
COPY ci/.toolchain-cache/libmbedx509-1_2.28.3-1_amd64.deb /tmp/
COPY ci/.toolchain-cache/libmbedcrypto7_2.28.3-1_amd64.deb /tmp/
COPY ci/.toolchain-cache/cmake-4.4.3-py3-none-manylinux2014_x86_64.manylinux_2_17_x86_64.whl /tmp/cmake.whl
RUN mkdir -p /mbedtls-root && \
    dpkg-deb -x /tmp/libmbedtls-dev_2.28.3-1_amd64.deb /mbedtls-root && \
    dpkg-deb -x /tmp/libmbedtls14_2.28.3-1_amd64.deb /mbedtls-root && \
    dpkg-deb -x /tmp/libmbedx509-1_2.28.3-1_amd64.deb /mbedtls-root && \
    dpkg-deb -x /tmp/libmbedcrypto7_2.28.3-1_amd64.deb /mbedtls-root && \
    mkdir -p /cmake-wheel && unzip -q /tmp/cmake.whl -d /cmake-wheel

FROM zlib_development_files AS gcc_toolchain
RUN cp -a /mbedtls-root/usr/include/mbedtls /usr/include/ && \
    cp -a /mbedtls-root/usr/include/psa /usr/include/ && \
    cp -a /mbedtls-root/usr/lib/x86_64-linux-gnu/. /usr/lib/x86_64-linux-gnu/ && \
    cp -a /cmake-wheel/cmake/data/. /usr/local/
LABEL org.opencontainers.image.title="Kern GCC 11.4 host-test toolchain"

FROM ${CLANG_BASE_IMAGE} AS clang_toolchain

# The Clang base already contains the exact same zlib runtime. Copy only the
# headers and static linker input from the digest-pinned GCC image, then create
# the conventional development symlink. No package repository is contacted.
COPY --from=zlib_development_files /usr/include/zlib.h /usr/include/zconf.h /usr/include/
COPY --from=zlib_development_files /usr/lib/x86_64-linux-gnu/libz.a /usr/lib/x86_64-linux-gnu/
COPY --from=zlib_development_files /mbedtls-root/usr/include/mbedtls/ /usr/include/mbedtls/
COPY --from=zlib_development_files /mbedtls-root/usr/include/psa/ /usr/include/psa/
COPY --from=zlib_development_files /mbedtls-root/usr/lib/x86_64-linux-gnu/ /usr/lib/x86_64-linux-gnu/
COPY --from=zlib_development_files /cmake-wheel/cmake/data/ /usr/local/
RUN ln -s libz.so.1 /usr/lib/x86_64-linux-gnu/libz.so

LABEL org.opencontainers.image.title="Kern Clang 18 host-test toolchain" \
      org.opencontainers.image.description="Digest-pinned Clang/LLVM 18.1.8 host-test environment"

FROM gcc_toolchain AS sdl_builder
COPY ci/.toolchain-cache/SDL2-2.32.10.tar.gz /tmp/
RUN mkdir -p /tmp/sdl-src && \
    tar -xzf /tmp/SDL2-2.32.10.tar.gz -C /tmp/sdl-src --strip-components=1 && \
    cmake -S /tmp/sdl-src -B /tmp/sdl-build \
      -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_INSTALL_PREFIX=/opt/sdl2 \
      -DSDL_SHARED=ON -DSDL_STATIC=ON -DSDL_TEST=OFF \
      -DSDL_X11=OFF -DSDL_WAYLAND=OFF \
      -DSDL_ALSA=OFF -DSDL_PULSEAUDIO=OFF -DSDL_PIPEWIRE=OFF \
      -DSDL_JACK=OFF -DSDL_SNDIO=OFF && \
    cmake --build /tmp/sdl-build --parallel 4 && \
    cmake --install /tmp/sdl-build

FROM gcc_toolchain AS cppcheck_builder
COPY ci/.toolchain-cache/cppcheck-2.22.0.tar.gz /tmp/
RUN mkdir -p /tmp/cppcheck-src && \
    tar -xzf /tmp/cppcheck-2.22.0.tar.gz -C /tmp/cppcheck-src --strip-components=1 && \
    make -C /tmp/cppcheck-src -j4 MATCHCOMPILER=yes \
      FILESDIR=/usr/share/cppcheck CXXOPTS=-O2 CPPOPTS=-DNDEBUG && \
    make -C /tmp/cppcheck-src install \
      MATCHCOMPILER=yes FILESDIR=/usr/share/cppcheck \
      DESTDIR=/cppcheck-root PREFIX=/usr

FROM gcc_toolchain AS device_free_toolchain
COPY --from=sdl_builder /opt/sdl2/ /usr/local/
COPY --from=cppcheck_builder /cppcheck-root/ /
LABEL org.opencontainers.image.title="Kern device-free validation toolchain" \
      org.opencontainers.image.description="Digest-pinned GCC 11.4, SDL2 2.32.10, Cppcheck 2.22.0, CMake 4.4.3 and mbedTLS 2.28.3 environment"
