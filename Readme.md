# EZLive: Self-hosted Serverless Livestream

EZLive is a minimal self-hosted livestream solution built on top of S3-compatible object storage.

It runs a local RTMP server, receive live video, turns it into HLS segments (.m3u8 + .ts) and serves them as static files through any S3-compatible OSS. No dedicated streaming server is required — everything runs serverlessly. Then you can easily setup a HTML5 HLS player to watch the stream.

# Build

Install dependencies:

- FFMpeg (libavformat, libavutil, libavcodec)
- AWS C++ SDK (libaws-cpp-sdk-core, libaws-cpp-sdk-s3)

Build:

    make


# Usage

Setup a S3-compatible OSS bucket, for example, Cloudflare R2, AWS S3, Minio, DigitalOcean, etc.

Then create a config file `config`:

```
listening_addr=127.0.0.1
listening_port=1935
bucket=YOUR_BUCKET_NAME
endpoint=https://your-oss.com
s3_path=ezlive/
access_key=YOUR_S3_ACCESS_KEY
secret_key=YOUR_S3_SECRET_KEY
region=auto
```

In the dashboard of your OSS cloud provider, add the domain name of your web HLS player to CORS setting. If you don't know how to setup a web HLS player, just add `https://mistivia.github.io`.

Start EZLive:

```
./ezlive
```

Open OBS, streaming to `rtmp://127.0.0.1/live`, no streaming key needed. The streaming format must be H.264 + AAC.

Then use a HLS player to load `https://YOUR_BUCKET_NAME.our-oss.com/ezlive/stream.m3u8` to watch the stream.

If you don't know how to setup a HLS player, then make sure you have added `https://mistivia.github.io` in your OSS's CORS setting, then open `https://mistivia.github.io/ezlive#https://YOUR_BUCKET_NAME.our-oss.com/ezlive/stream.m3u8`.


# Credits

The built-in RTMP server is modified from [pine](https://github.com/deboot/pine).
