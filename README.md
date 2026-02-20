# EZLive: Self-hosted Serverless Livestream

[中文说明](./README_zh.md)

EZLive is a minimal self-hosted livestream solution built on top of S3-compatible object storage.


It runs a local SRT server, receive live video, turns it into HLS segments (.m3u8 + .ts) and serves them as static files through any S3-compatible object storage. No dedicated streaming server is required — everything runs serverlessly. Then you can easily setup a HTML5 HLS player to watch the stream.


# Build


Install dependencies:


- [SRT](https://github.com/Haivision/srt)
- FFMpeg (libavformat, libavutil, libavcodec)
- AWS C++ SDK (libaws-cpp-sdk-core, libaws-cpp-sdk-s3)

Build:


    make

# Usage


Setup a S3-compatible object storage bucket, for example, Cloudflare R2, AWS S3, Minio, DigitalOcean, etc.


Then create a config file `config`:


```
listening_addr=127.0.0.1
listening_port=61935
bucket=YOUR_BUCKET_NAME
endpoint=https://your-s3.com
s3_path=ezlive/
access_key=YOUR_S3_ACCESS_KEY
secret_key=YOUR_S3_SECRET_KEY
region=auto
key=your_live_key
```

In the dashboard of your object storage provider, enable public read, and add the domain name of your web HLS player to CORS setting. If you don't know how to setup a web HLS player, just add `https://mistivia.github.io`.

For AWS S3, Edit bucket setting, set "Permissions" -> "Bucket Policy" to:


    {
        "Version": "2012-10-17",
        "Statement": [
            {
                "Sid": "PublicReadOnly",
                "Effect": "Allow",
                "Principal": "*",
                "Action": "s3:GetObject",
                "Resource": "arn:aws:s3:::YOUR_BUCKET/*"
            }
        ]
    }

Then set "Permissions" -> "Cross-origin resource sharing (CORS)" to:

    [
        {
            "AllowedHeaders": [
                "*"
            ],
            "AllowedMethods": [
                "GET"
            ],
            "AllowedOrigins": [
                "https://your.hls.player.com"
            ],
            "ExposeHeaders": []
        }
    ]

Start EZLive:
```
./ezlive
```

Open OBS, streaming to `srt://127.0.0.1:61935`, with streaming key. The streaming format must be H.264 + AAC.

Then use a HLS player to load `https://YOUR_BUCKET_NAME.your-s3.com/ezlive/stream.m3u8` to watch the stream.

If you don't know how to setup a HLS player, then make sure you have added `https://mistivia.github.io` in your object storage's CORS setting, then open `https://mistivia.github.io/ezlive#https://YOUR_BUCKET_NAME.your-s3.com/ezlive/stream.m3u8`.

# Credits

Thank [@uonr](https://github.com/uonr) for making nix flake.
