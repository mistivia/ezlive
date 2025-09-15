FROM scratch
COPY rootfs /
ENTRYPOINT ["/ezlive", "/etc/ezlive/config"]
