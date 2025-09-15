#include "s3_client.h"

#include <stdio.h>
#include <fstream>
#include <iostream>

#include <aws/core/Aws.h>
#include <aws/s3/S3Client.h>
#include <aws/s3/model/PutObjectRequest.h>
#include <aws/s3/model/DeleteObjectRequest.h>
#include <aws/s3/model/DeleteObjectsRequest.h>
#include <aws/core/auth/AWSCredentials.h>
#include <aws/s3/model/ListObjectsV2Request.h>

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
    config.checksumConfig.requestChecksumCalculation = Aws::Client::RequestChecksumCalculation::WHEN_REQUIRED;
    config.checksumConfig.responseChecksumValidation = Aws::Client::ResponseChecksumValidation::WHEN_REQUIRED;
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

void S3Client_clear() {
    Aws::S3::Model::ListObjectsV2Request list_req;
    list_req.WithBucket(ezlive_config->bucket).WithPrefix(ezlive_config->s3_path);

    auto list_outcome = s3client->ListObjectsV2(list_req);
    if (!list_outcome.IsSuccess()) {
        fprintf(stderr, "ListObjectsV2 error: %s\n", 
            list_outcome.GetError().GetMessage().c_str());
        return;
    }

    Aws::Vector<Aws::S3::Model::ObjectIdentifier> objects_to_delete;

    for (const auto& obj : list_outcome.GetResult().GetContents()) {
        auto key = obj.GetKey();
        if (key.size() >= 3 && key.substr(key.size() - 3) == ".ts") {
            Aws::S3::Model::ObjectIdentifier oid;
            oid.SetKey(key);
            objects_to_delete.push_back(oid);
            printf("Marking for delete: %s\n", key.c_str());
        }
    }
    if (!objects_to_delete.empty()) {
        Aws::S3::Model::DeleteObjectsRequest del_req;
        del_req.WithBucket(ezlive_config->bucket)
                .WithDelete(Aws::S3::Model::Delete().WithObjects(objects_to_delete));

        auto del_outcome = s3client->DeleteObjects(del_req);
        if (!del_outcome.IsSuccess()) {
            std::cerr << "DeleteObjects error: " 
                        << del_outcome.GetError().GetMessage() << std::endl;
            return;
        } else {
            std::cout << "Deleted " 
                        << del_outcome.GetResult().GetDeleted().size() 
                        << " objects." << std::endl;
        }
    } else {
        std::cout << "No .ts files found. No need to clear." << std::endl;
    }
}