ARG CLANG_BASE_IMAGE=silkeh/clang@sha256:3914c93a02e866795aafc80737488e515b96390eff3d2787cf8c5095997baea9
ARG ESP_IDF_IMAGE=espressif/idf@sha256:81893c71bb5e570088901f21def8684c25cd2a9020281bd01b843a7655edb18c

FROM ${CLANG_BASE_IMAGE} AS clang_analyzer
RUN mkdir -p /clang-root/usr/lib /clang-root/opt/clang-libs && \
    cp -a /usr/lib/llvm-18 /clang-root/usr/lib/ && \
    for lib in libLLVM-18.so.18.1 libffi.so.8 libedit.so.2 libz3.so.4 \
               libzstd.so.1 libtinfo.so.6 libxml2.so.2 libbsd.so.0 \
               libicuuc.so.72 liblzma.so.5 libmd.so.0 libicudata.so.72; do \
      cp -aL "/lib/x86_64-linux-gnu/$lib" /clang-root/opt/clang-libs/; \
    done

FROM ${ESP_IDF_IMAGE} AS static_analysis
COPY --from=clang_analyzer /clang-root/usr/lib/llvm-18/ /usr/lib/llvm-18/
COPY --from=clang_analyzer /clang-root/opt/clang-libs/ /opt/clang-libs/
RUN ln -s /usr/lib/llvm-18/bin/clang-tidy /usr/local/bin/clang-tidy
ENV LD_LIBRARY_PATH=/opt/clang-libs
LABEL org.opencontainers.image.title="Kern ESP-IDF static-analysis toolchain" \
      org.opencontainers.image.description="Digest-pinned ESP-IDF 6.1 with Clang-Tidy 18.1.8"
