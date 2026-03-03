#pragma once

namespace Aws { namespace S3 { class S3Client; } }

namespace ezlive {

class s3_client
{
public:
    static s3_client& get_instance();

    void init();

    void put(const char *filename, const char *object_name);

    void remove(const char *object_name);

    void clear();

private:
    s3_client();
    ~s3_client();
    s3_client(s3_client&&) = delete;
    s3_client& operator=(s3_client&&) = delete;
    s3_client(const s3_client&) = delete;
    s3_client& operator=(const s3_client&) = delete;

    Aws::S3::S3Client *m_s3client;
}; //class s3_client

} // namespace ezlive
