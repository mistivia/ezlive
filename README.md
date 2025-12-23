# EZLive: Self-hosted Serverless Livestream

# EZLive：自托管无服务器直播

EZLive is a minimal self-hosted livestream solution built on top of S3-compatible object storage.

EZLive 是一个基于 S3 兼容对象存储构建的极简自托管直播解决方案。

It runs a local SRT server, receive live video, turns it into HLS segments (.m3u8 + .ts) and serves them as static files through any S3-compatible object storage. No dedicated streaming server is required — everything runs serverlessly. Then you can easily setup a HTML5 HLS player to watch the stream.

它运行一个本地 SRT 服务器，接收直播视频流，将其转换为 HLS 片段（.m3u8 + .ts），并通过任何 S3 兼容对象存储将其作为静态文件提供服务。它无需专用的流媒体服务器：一切都以无服务器方式运行。随后，你可以轻松设置一个 HTML5 HLS 播放器来观看直播流。

# Build

# 构建

Install dependencies:

安装依赖项：

- [SRT](https://github.com/Haivision/srt)
- FFMpeg (libavformat, libavutil, libavcodec)
- AWS C++ SDK (libaws-cpp-sdk-core, libaws-cpp-sdk-s3)

Build:

构建：

    make

# Usage

# 使用方法

Setup a S3-compatible object storage bucket, for example, Cloudflare R2, AWS S3, Minio, DigitalOcean, etc.

设置一个 S3 兼容的对象存储桶，例如 Cloudflare R2、AWS S3、Minio、DigitalOcean 等。

Then create a config file `config`:

然后创建一个配置文件 `config`：

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

在你的对象存储提供商的面板中，启用公共读取权限，并将您的 Web HLS 播放器的域名添加到 CORS 设置中。如果你不知道如何设置 Web HLS 播放器，那么只需添加 `https://mistivia.github.io`。

For AWS S3, Edit bucket setting, set "Permissions" -> "Bucket Policy" to:

对于 AWS S3，编辑存储桶设置，将“权限” -> “存储桶策略”设置为：

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

然后将“权限” -> “跨源资源共享 (CORS)”设置为：

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

启动 EZLive：

```
./ezlive
```

Open OBS, streaming to `srt://127.0.0.1:61935`, with streaming key. The streaming format must be H.264 + AAC.

Then use a HLS player to load `https://YOUR_BUCKET_NAME.your-s3.com/ezlive/stream.m3u8` to watch the stream.

If you don't know how to setup a HLS player, then make sure you have added `https://mistivia.github.io` in your object storage's CORS setting, then open `https://mistivia.github.io/ezlive#https://YOUR_BUCKET_NAME.your-s3.com/ezlive/stream.m3u8`.

# WARNING

The Docker build was done on my laptop. While I promise I have no malicious intent, this does not guarantee that my build environment is secure, so is the build target. If you truly care about your privacy and security, please make sure to build from source yourself.

# Docker Usage

Download the docker image tarball in [release](https://github.com/mistivia/ezlive/releases).

Load the docker image:

    cat ezlive-docker-image.tar.gz | gzip -d | sudo docker load

Create a directory `conf`:

    mkdir conf

Create a config file `conf/config`, the config file is nearly the same as the config above. But for docker, the `listening_addr` should be `0.0.0.0`:

    listening_addr=0.0.0.0
    listening_port=61935
    bucket=YOUR_BUCKET_NAME
    endpoint=https://your-s3.com
    s3_path=ezlive/
    access_key=YOUR_S3_ACCESS_KEY
    secret_key=YOUR_S3_SECRET_KEY
    region=auto
    key=your_live_key

Start docker container:

    sudo docker run -it --rm \
        -v ./conf:/etc/ezlive/ \
        -p 127.0.0.1:61935:61935/udp \
        localhost/ezlive

# Credits

Thank [@uonr](https://github.com/uonr) for making nix flake.
