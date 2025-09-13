#ifndef FS_UTILS_H_
#define FS_UTILS_H_

void tmp_local_filename(const char *prefix, char *buf);

void tmp_ts_prefix(char *buf);

void ts_filename(const char *prefix, int num, char *buf);

void upload_file(const char *local, const char *remote);

void remove_remote(const char *remote);

char ** list_file();

#endif