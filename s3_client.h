#ifndef S3_CLIENT_H_
#define S3_CLIENT_H_

#ifdef __cplusplus
extern "C" {
#endif

void S3Client_init();

void S3Client_put(const char *filename, const char *object_name);

void S3Client_delete(const char *object_name);

void S3Client_clear();

#ifdef __cplusplus
}
#endif

#endif
