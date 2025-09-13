#include "s3_client.h"

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
        //We are using the name of the file as the key for the object in the bucket.
        //However, this is just a string and can be set according to your retrieval needs.
        request.SetKey(object_name);

        std::shared_ptr<Aws::IOStream> inputData =
                std::make_shared<Aws::FStream>(filename, std::ios_base::in | std::ios_base::binary);

        if (!*inputData) {
            std::cerr << "Error unable to read file " << filename << std::endl;
            return;
        }

        request.SetBody(inputData);

        Aws::S3::Model::PutObjectOutcome outcome =
                s3client->PutObject(request);

        if (!outcome.IsSuccess()) {
            std::cerr << "Error: putObject: " <<
                    outcome.GetError().GetMessage() << std::endl;
            sleep(3);
            continue;
        } else {
            std::cout << "Added object '" << object_name << "' to bucket '"
                    << ezlive_config->bucket << "'.";
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
        std::cerr << "Error: deleteObject: " <<
                  err.GetExceptionName() << ": " << err.GetMessage() << std::endl;
    } else {
        std::cout << "Successfully deleted the object: " << object_name << std::endl;
    }
}