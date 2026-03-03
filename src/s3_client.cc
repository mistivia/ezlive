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

namespace ezlive {

s3_client::s3_client()
    : m_s3client(nullptr)
{
}

s3_client::~s3_client()
{
    delete m_s3client;
}

s3_client& s3_client::get_instance()
{
    static s3_client instance;
    return instance;
}

void s3_client::init()
{
    Aws::SDKOptions aws_options;
    Aws::InitAPI(aws_options);
    Aws::S3::S3ClientConfiguration config;
    Aws::Auth::AWSCredentials credentials;
    config.endpointOverride = g_config->endpoint;
    config.region = g_config->region;
#if !defined(_WIN32)
    config.checksumConfig.requestChecksumCalculation = Aws::Client::RequestChecksumCalculation::WHEN_REQUIRED;
    config.checksumConfig.responseChecksumValidation = Aws::Client::ResponseChecksumValidation::WHEN_REQUIRED;
#endif
    credentials = Aws::Auth::AWSCredentials(g_config->access_key, g_config->secret_key);
    m_s3client = new Aws::S3::S3Client(credentials, nullptr, config);
}

void s3_client::put(const char *filename, const char *object_name)
{
    while (1) {
        Aws::S3::Model::PutObjectRequest request;
        request.SetBucket(g_config->bucket);
        request.SetKey(object_name);
        std::shared_ptr<Aws::IOStream> inputData =
                std::make_shared<Aws::FStream>(filename, std::ios_base::in | std::ios_base::binary);

        if (!*inputData) {
            fprintf(stderr, "Error unable to read file: %s\n", filename);
            return;
        }

        request.SetBody(inputData);

        Aws::S3::Model::PutObjectOutcome outcome =
                m_s3client->PutObject(request);

        if (!outcome.IsSuccess()) {
            fprintf(stderr, "Error: putObject: %s.\n", outcome.GetError().GetMessage().c_str());
            sleep(3);
            continue;
        } else {
            printf("Added object '%s' to bucket '%s'.\n", object_name, g_config->bucket.c_str());
            break;
        }
    }
}

void s3_client::remove(const char *object_name)
{
    Aws::S3::Model::DeleteObjectRequest request;

    request.WithKey(object_name)
            .WithBucket(g_config->bucket);

    Aws::S3::Model::DeleteObjectOutcome outcome =
            m_s3client->DeleteObject(request);

    if (!outcome.IsSuccess()) {
        auto err = outcome.GetError();
        fprintf(stderr, "Error: deleteObject: %s: %s\n",
            err.GetExceptionName().c_str(), err.GetMessage().c_str());
    } else {
        fprintf(stdout, "Successfully deleted the object: %s\n", object_name);
    }
}

void s3_client::clear()
{
    Aws::S3::Model::ListObjectsV2Request list_req;
    list_req.WithBucket(g_config->bucket).WithPrefix(g_config->s3_path);

    auto list_outcome = m_s3client->ListObjectsV2(list_req);
    if (!list_outcome.IsSuccess()) {
        fprintf(stderr, "ListObjectsV2 error: %s\n", 
            list_outcome.GetError().GetMessage().c_str());
        return;
    }

    std::vector<std::string> objects_to_delete;

    for (const auto& obj : list_outcome.GetResult().GetContents()) {
        auto key = obj.GetKey();
        if (key.size() >= 3 && key.substr(key.size() - 3) == ".ts") {
            objects_to_delete.push_back(key);
            printf("Marking for delete: %s\n", key.c_str());
        }
    }
    if (!objects_to_delete.empty()) {
        for (auto &x : objects_to_delete) {
            remove(x.c_str());
        }
    } else {
        std::cout << "No .ts files found. No need to clear." << std::endl;
    }
}

} // namespace ezlive