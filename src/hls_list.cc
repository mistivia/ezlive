#include "hls_list.h"
#include "utils.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>

namespace ezlive {

#if defined(_WIN32)
    cosnt std::string TMP_PREFIX = "./tmp";
#else
    const std::string TMP_PREFIX  = "/tmp/ezlive";
#endif

hls_list::hls_list()
    : m_len(0)
{
    m_lst_files.reserve(max_len);
    m_lst_times.reserve(max_len);
}

hls_list::~hls_list()
{
    clear();
}

std::string hls_list::push(const std::string& name, double time)
{
    std::string name_copy = name;
    if (m_len < max_len) {
        m_lst_files.push_back(name_copy);
        m_lst_times.push_back(time);
        m_len++;
        return "";
    }
    std::string deleted = m_lst_files[0];
    for (int i = 0; i < max_len - 1; i++) {
        m_lst_files[i] = std::move(m_lst_files[i + 1]);
        m_lst_times[i] = std::move(m_lst_times[i + 1]);
    }
    m_lst_files[max_len - 1] = name_copy;
    m_lst_times[max_len - 1] = time;
    return deleted;
}

void hls_list::clear()
{
    m_lst_files.clear();
    m_lst_times.clear();
    m_len = 0;
}

void hls_list::update_m3u8(int last_seg)
{
    int first_seg = last_seg - m_len + 1;
    char out_filename[256] = {0};
    tmp_local_filename(TMP_PREFIX.c_str(), out_filename);
    FILE *fp = fopen(out_filename, "w");
    if (fp == NULL) {
        fprintf(stderr, "failed to open %s for output.\n", out_filename);
        exit(-1);
    }
    if (m_len == 0) {
        fprintf(fp, "\n");
    } else {
        fprintf(fp, "#EXTM3U\n");
        fprintf(fp, "#EXT-X-VERSION:3\n");
        fprintf(fp, "#EXT-X-TARGETDURATION:10\n");
        fprintf(fp, "#EXT-X-MEDIA-SEQUENCE:%d\n", first_seg);
        for (int i = 0; i < m_len; i++) {
            fprintf(fp, "#EXTINF:%lf\n", m_lst_times[i]);
            fprintf(fp, "%s\n", m_lst_files[i].c_str());
        }
    }
    fclose(fp);
    upload_file(out_filename, "stream.m3u8");
}

const char* hls_list::file(int index) const
{
    if (index < 0 || index >= m_len) {
        return "";
    }
    return m_lst_files[index].c_str();
}

double hls_list::time(int index) const
{
    if (index < 0 || index >= m_len) {
        return 0.0;
    }
    return m_lst_times[index];
}

} //namespace ezlive
