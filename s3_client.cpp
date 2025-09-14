#include "s3_client.h"

#include <stdio.h>
#include <fstream>
#include <iostream>

#include <aws/core/Aws.h>
#include <aws/s3/S3Client.h>
#include <aws/s3/model/PutObjectRequest.h>
#include <aws/s3/model/DeleteObjectRequest.h>
#include <aws/core/auth/AWSCredentials.h>

#include "ezlive_config.h"

#include <unistd.h>

namespace {

Aws::S3::S3Client *s3client;

}

void S3Client_init() {
    Aws::SDKOptions aws_options;
    Aws::InitAPI(aws_options);
    Aws::S3::S3ClientConfiguration config;
    Aws::Auth::AWSCredentials credentials;
    config.endpointOverride = ezlive_config->endpoint;
    config.region = ezlive_config->region;
    credentials = Aws::Auth::AWSCredentials(ezlive_config->access_key, ezlive_config->secret_key);
    s3client = new Aws::S3::S3Client(credentials, nullptr, config);
}

void S3Client_put(const char *filename, const char *object_name) {
    while (1) {
        Aws::S3::Model::PutObjectRequest request;
        request.SetBucket(ezlive_config->bucket);
        request.SetKey(object_name);
        std::shared_ptr<Aws::IOStream> inputData =
                std::make_shared<Aws::FStream>(filename, std::ios_base::in | std::ios_base::binary);

        if (!*inputData) {
            fprintf(stderr, "Error unable to read file: %s\n", filename);
            return;
        }

        request.SetBody(inputData);

        Aws::S3::Model::PutObjectOutcome outcome =
                s3client->PutObject(request);

        if (!outcome.IsSuccess()) {
            fprintf(stderr, "Error: putObject: %s.\n", outcome.GetError().GetMessage().c_str());
            sleep(3);
            continue;
        } else {
            printf("Added object '%s' to bucket '%s'.\n", object_name, ezlive_config->bucket);
            break;
        }
    }
}

void S3Client_delete(const char *object_name) {
    Aws::S3::Model::DeleteObjectRequest request;

    request.WithKey(object_name)
            .WithBucket(ezlive_config->bucket);

    Aws::S3::Model::DeleteObjectOutcome outcome =
            s3client->DeleteObject(request);

    if (!outcome.IsSuccess()) {
        auto err = outcome.GetError();
        fprintf(stderr, "Error: deleteObject: %s: %s\n",
            err.GetExceptionName().c_str(), err.GetMessage().c_str());
    } else {
        fprintf(stdout, "Successfully deleted the object: %s\n", object_name);
    }
}