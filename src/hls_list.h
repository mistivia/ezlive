#pragma once

#include <vector>
#include <string>

namespace ezlive {

extern const std::string TMP_PREFIX;

class hls_list {
public:
    explicit hls_list();
    ~hls_list();
    std::string push(const std::string& name, double time);
    void clear();
    void update_m3u8(int last_seg);
    int len() const { return m_len; }
    const char* file(int index) const;
    double time(int index) const;
private:
    static constexpr int max_len = 15;
    std::vector<std::string> m_lst_files;
    std::vector<double> m_lst_times;
    int m_len;
};

} //namespace ezlive
