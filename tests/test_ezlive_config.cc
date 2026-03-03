#include "../src/ezlive_config.h"

#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unistd.h>

// Helper function to create temporary config file
static std::string create_temp_file(const char* content)
{
    char filename[] = "/tmp/ezlive_test_config_XXXXXX";
    int fd = mkstemp(filename);
    if (fd < 0) {
        return "";
    }
    write(fd, content, strlen(content));
    close(fd);
    return std::string(filename);
}

static void test_default_constructor()
{
    ezlive::config cfg;
    assert(cfg.listening_addr == "127.0.0.1");
    assert(cfg.listening_port == 1935);
    assert(cfg.s3_path == "ezlive/");
    assert(cfg.region == "auto");
    assert(cfg.bucket.empty());
    assert(cfg.endpoint.empty());
    assert(cfg.access_key.empty());
    assert(cfg.secret_key.empty());
    assert(cfg.key.empty());
}

static void test_load_null_filename()
{
    ezlive::config cfg;
    int ret = cfg.load(nullptr);
    assert(ret == -1);
}

static void test_load_nonexistent_file()
{
    ezlive::config cfg;
    int ret = cfg.load("/nonexistent/path/to/config.txt");
    assert(ret == -1);
}

static void test_load_valid_config()
{
    const char* content =
        "listening_addr=0.0.0.0\n"
        "listening_port=8080\n"
        "bucket=mybucket\n"
        "endpoint=s3.example.com\n"
        "s3_path=myapp/\n"
        "access_key=abc123\n"
        "secret_key=xyz789\n"
        "region=us-east-1\n"
        "key=mykey\n";
    std::string filename = create_temp_file(content);
    assert(!filename.empty());

    ezlive::config cfg;
    int ret = cfg.load(filename.c_str());
    assert(ret == 0);
    assert(cfg.listening_addr == "0.0.0.0");
    assert(cfg.listening_port == 8080);
    assert(cfg.bucket == "mybucket");
    assert(cfg.endpoint == "s3.example.com");
    assert(cfg.s3_path == "myapp/");
    assert(cfg.access_key == "abc123");
    assert(cfg.secret_key == "xyz789");
    assert(cfg.region == "us-east-1");
    assert(cfg.key == "mykey");

    std::remove(filename.c_str());
}

static void test_load_with_comments_and_whitespace()
{
    const char* content =
        "  # This is a comment\n"
        "listening_addr=192.168.1.1\n"
        "   \n"
        "  listening_port  =  9090  \n"
        "# another comment\n"
        "bucket=mybucket\n"
        "endpoint=s3.example.com\n"
        "s3_path=test/\n"
        "access_key=key\n"
        "secret_key=secret\n"
        "region=region\n"
        "key=k\n";
    std::string filename = create_temp_file(content);
    assert(!filename.empty());

    ezlive::config cfg;
    int ret = cfg.load(filename.c_str());
    assert(ret == 0);
    assert(cfg.listening_addr == "192.168.1.1");
    assert(cfg.listening_port == 9090);

    std::remove(filename.c_str());
}

static void test_load_missing_bucket()
{
    const char* content =
        "listening_addr=127.0.0.1\n"
        "listening_port=8080\n"
        "endpoint=s3.example.com\n"
        "s3_path=test/\n"
        "access_key=key\n"
        "secret_key=secret\n"
        "region=region\n"
        "key=k\n";
    std::string filename = create_temp_file(content);
    assert(!filename.empty());

    ezlive::config cfg;
    int ret = cfg.load(filename.c_str());
    assert(ret == -3);

    std::remove(filename.c_str());
}

static void test_load_missing_endpoint()
{
    const char* content =
        "listening_addr=127.0.0.1\n"
        "listening_port=8080\n"
        "bucket=mybucket\n"
        "s3_path=test/\n"
        "access_key=key\n"
        "secret_key=secret\n"
        "region=region\n"
        "key=k\n";
    std::string filename = create_temp_file(content);
    assert(!filename.empty());

    ezlive::config cfg;
    int ret = cfg.load(filename.c_str());
    assert(ret == -4);

    std::remove(filename.c_str());
}

static void test_load_invalid_s3_path_no_slash()
{
    const char* content =
        "listening_addr=127.0.0.1\n"
        "listening_port=8080\n"
        "bucket=mybucket\n"
        "endpoint=s3.example.com\n"
        "s3_path=test\n"
        "access_key=key\n"
        "secret_key=secret\n"
        "region=region\n"
        "key=k\n";
    std::string filename = create_temp_file(content);
    assert(!filename.empty());

    ezlive::config cfg;
    int ret = cfg.load(filename.c_str());
    assert(ret == -10);

    std::remove(filename.c_str());
}

static void test_load_invalid_s3_path_too_short()
{
    const char* content =
        "listening_addr=127.0.0.1\n"
        "listening_port=8080\n"
        "bucket=mybucket\n"
        "endpoint=s3.example.com\n"
        "s3_path=x\n"
        "access_key=key\n"
        "secret_key=secret\n"
        "region=region\n"
        "key=k\n";
    std::string filename = create_temp_file(content);
    assert(!filename.empty());

    ezlive::config cfg;
    int ret = cfg.load(filename.c_str());
    assert(ret == -5);

    std::remove(filename.c_str());
}

static void test_load_missing_access_key()
{
    const char* content =
        "listening_addr=127.0.0.1\n"
        "listening_port=8080\n"
        "bucket=mybucket\n"
        "endpoint=s3.example.com\n"
        "s3_path=test/\n"
        "secret_key=secret\n"
        "region=region\n"
        "key=k\n";
    std::string filename = create_temp_file(content);
    assert(!filename.empty());

    ezlive::config cfg;
    int ret = cfg.load(filename.c_str());
    assert(ret == -6);

    std::remove(filename.c_str());
}

static void test_load_missing_secret_key()
{
    const char* content =
        "listening_addr=127.0.0.1\n"
        "listening_port=8080\n"
        "bucket=mybucket\n"
        "endpoint=s3.example.com\n"
        "s3_path=test/\n"
        "access_key=key\n"
        "region=region\n"
        "key=k\n";
    std::string filename = create_temp_file(content);
    assert(!filename.empty());

    ezlive::config cfg;
    int ret = cfg.load(filename.c_str());
    assert(ret == -7);

    std::remove(filename.c_str());
}

static void test_load_invalid_port()
{
    const char* content =
        "listening_addr=127.0.0.1\n"
        "listening_port=99999\n"
        "bucket=mybucket\n"
        "endpoint=s3.example.com\n"
        "s3_path=test/\n"
        "access_key=key\n"
        "secret_key=secret\n"
        "region=region\n"
        "key=k\n";
    std::string filename = create_temp_file(content);
    assert(!filename.empty());

    ezlive::config cfg;
    int ret = cfg.load(filename.c_str());
    assert(ret == -8);

    std::remove(filename.c_str());
}

int main()
{
    test_default_constructor();
    test_load_null_filename();
    test_load_nonexistent_file();
    test_load_valid_config();
    test_load_with_comments_and_whitespace();
    test_load_missing_bucket();
    test_load_missing_endpoint();
    test_load_invalid_s3_path_no_slash();
    test_load_invalid_s3_path_too_short();
    test_load_missing_access_key();
    test_load_missing_secret_key();
    test_load_invalid_port();

    printf("All ezlive_config tests passed!\n");
    return 0;
}
