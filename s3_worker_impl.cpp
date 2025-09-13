#include "s3_worker.h"

#include <aws/core/Aws.h>
#include <aws/s3/S3Client.h>
#include <aws/s3/model/PutObjectRequest.h>

void *s3client;

void s3client_init() {

}

void s3client_put(const char *filename, const char *object_name) {

}

void s3client_delete(const char *object_name) {

}