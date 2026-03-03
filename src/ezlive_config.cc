#include "ezlive_config.h"

#include <cstdio>
#include <cctype>
#include <string>

namespace ezlive {

std::unique_ptr<config> g_config;

config::config()
    : listening_addr("127.0.0.1")
    , listening_port(1935)
    , bucket()
    , endpoint()
    , s3_path("ezlive/")
    , access_key()
    , secret_key()
    , region("auto")
    , key()
{
}

static std::string trim(const std::string& s)
{
    size_t start = 0;
    while (start < s.size() && isspace(static_cast<unsigned char>(s[start]))) start++;
    
    if (start == s.size()) {
        return "";
    }
    
    size_t end = s.size() - 1;
    while (end > start && isspace(static_cast<unsigned char>(s[end]))) end--;
    
    return s.substr(start, end - start + 1);
}

int config::load(const char *filename)
{
    if (!filename) {
        fprintf(stderr, "config filename is null\n");
        return -1;
    }

    FILE *fp = fopen(filename, "r");
    if (!fp) {
        fprintf(stderr, "failed to open config file: %s\n", filename);
        return -1;
    }

    char buf[1024];
    while (fgets(buf, sizeof(buf), fp)) {
        std::string line = buf;
        line.erase(line.find_last_not_of("\r\n") + 1);
        
        size_t hash_pos = line.find('#');
        if (hash_pos != std::string::npos) {
            line = line.substr(0, hash_pos);
        }

        std::string trimmed = trim(line);
        if (trimmed.empty()) {
            continue;
        }

        size_t eq_pos = trimmed.find('=');
        if (eq_pos == std::string::npos) {
            continue;
        }

        std::string key = trim(trimmed.substr(0, eq_pos));
        std::string val = trim(trimmed.substr(eq_pos + 1));

        if (key == "listening_addr") {
            listening_addr = val;
        } else if (key == "listening_port") {
            listening_port = std::stoi(val);
        } else if (key == "bucket") {
            bucket = val;
        } else if (key == "endpoint") {
            endpoint = val;
        } else if (key == "s3_path") {
            s3_path = val;
        } else if (key == "access_key") {
            access_key = val;
        } else if (key == "secret_key") {
            secret_key = val;
        } else if (key == "region") {
            region = val;
        } else if (key == "key") {
            this->key = val;
        }
    }

    fclose(fp);
    if (validate() != 0) {
        return -1;
    }
    return 0;
}

int config::validate()
{
    if (listening_addr.empty()) {
        fprintf(stderr, "config error: listening_addr is empty\n");
        return -2;
    }
    if (bucket.empty()) {
        fprintf(stderr, "config error: bucket is empty\n");
        return -3;
    }
    if (endpoint.empty()) {
        fprintf(stderr, "config error: endpoint is empty\n");
        return -4;
    }
    if (s3_path.empty() || s3_path.size() < 2) {
        fprintf(stderr, "config error: s3_path is too short\n");
        return -5;
    }
    if (s3_path.back() != '/') {
        fprintf(stderr, "config error: s3_path should end with '/'\n");
        return -10;
    }
    if (access_key.empty()) {
        fprintf(stderr, "config error: access_key is empty\n");
        return -6;
    }
    if (secret_key.empty()) {
        fprintf(stderr, "config error: secret_key is empty\n");
        return -7;
    }
    if (listening_port <= 0 || listening_port > 65535) {
        fprintf(stderr, "config error: listening_port is invalid\n");
        return -8;
    }
    if (region.empty()) {
        fprintf(stderr, "config error: region is empty\n");
        return -9;
    }
    return 0;
}

} // namespace ezlive
