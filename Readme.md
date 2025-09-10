# EZLive: Self-hosted Serverless Livestream

(Still work-in-progress)

EZLive is a minimal self-hosted livestream solution built on top of S3-compatible object storage.

It runs a local RTMP server, receive live video, turns it into HLS segments (.m3u8 + .ts) and serves them as static files through S3 or a CDN. No dedicated streaming server is required — everything runs serverless.