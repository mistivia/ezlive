# EZLive: 自托管 Serverless 直播方案

EZLive 是一个构建在 S3 兼容对象存储之上的极简自托管直播解决方案。

它运行一个本地 SRT 服务器，接收实时视频流，将其转换为 HLS 切片（.m3u8 + .ts），并通过任何兼容 S3 的对象存储作为静态文件对外发布。EZLive不需要专用的流媒体服务器，一切都在 Serverless（无服务器）架构下运行。随后，你可以轻松设置一个 HTML5 HLS 播放器来观看直播。

# 构建

安装依赖：

- [SRT](https://github.com/Haivision/srt)
- FFMpeg (libavformat, libavutil, libavcodec)
- AWS C++ SDK (libaws-cpp-sdk-core, libaws-cpp-sdk-s3)

构建：

    make

# 使用方法

设置一个兼容 S3 的对象存储存储桶（Bucket），例如 Cloudflare R2、AWS S3、Minio、DigitalOcean 等。

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

在你的对象存储提供商的仪表板中，开启**公共读取（Public Read）**权限，并将你的 Web HLS 播放器的域名添加到 **CORS（跨域资源共享）** 设置中。如果你不知道如何设置 Web HLS 播放器，只需添加 `https://mistivia.github.io`。

对于 **AWS S3**，编辑存储桶设置，将“权限 (Permissions)” -> “存储桶策略 (Bucket Policy)”设置为：

```
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
```

然后将“权限 (Permissions)” -> “跨源资源共享 (CORS)”设置为：

```
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
```

启动 EZLive：

```bash
./ezlive
```

打开 OBS，推流到 `srt://127.0.0.1:61935`，并填写推流码。推流格式必须是 **H.264 + AAC**。

然后使用 HLS 播放器加载 `https://YOUR_BUCKET_NAME.your-s3.com/ezlive/stream.m3u8` 即可观看直播。

如果你不知道如何设置 HLS 播放器，请确保你已将 `https://mistivia.github.io` 添加到对象存储的 CORS 设置中，然后打开链接： `https://mistivia.github.io/ezlive#https://YOUR_BUCKET_NAME.your-s3.com/ezlive/stream.m3u8`。

# 致谢

感谢 [@uonr](https://github.com/uonr) 制作 nix flake。
