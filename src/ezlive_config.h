#pragma once

#include <string>
#include <memory>

namespace ezlive {

class config {
public:
    explicit config();    
    int load(const char *filename);
    std::string listening_addr;
    int listening_port;
    std::string bucket;
    std::string endpoint;
    std::string s3_path;
    std::string access_key;
    std::string secret_key;
    std::string region;
    std::string key;
private:
    int validate();
};

extern std::unique_ptr<config> g_config;

} // namespace ezlive
