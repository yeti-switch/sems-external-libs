/** **************************************************************************
 * object.c
 * 
 * Copyright 2008 Bryan Ischo <bryan@ischo.com>
 * 
 * This file is part of libs3.
 * 
 * libs3 is free software: you can redistribute it and/or modify it under the
 * terms of the GNU General Public License as published by the Free Software
 * Foundation, version 3 of the License.
 *
 * In addition, as a special exception, the copyright holders give
 * permission to link the code of this library and its programs with the
 * OpenSSL library, and distribute linked combinations including the two.
 *
 * libs3 is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License version 3
 * along with libs3, in a file named COPYING.  If not, see
 * <http://www.gnu.org/licenses/>.
 *
 ************************************************************************** **/

#include <stdlib.h>
#include <string.h>
#include "libs3.h"
#include "request.h"


// put object ----------------------------------------------------------------

void S3_put_object(const S3BucketContext *bucketContext, const char *key,
                   uint64_t contentLength,
                   const S3PutProperties *putProperties,
                   S3RequestContext *requestContext,
                   const S3PutObjectHandler *handler, void *callbackData)
{
    // Set up the RequestParams
    RequestParams params =
    {
        HttpRequestTypePUT,                           // httpRequestType
        { bucketContext->hostName,                    // hostName
          bucketContext->bucketName,                  // bucketName
          bucketContext->protocol,                    // protocol
          bucketContext->uriStyle,                    // uriStyle
          bucketContext->accessKeyId,                 // accessKeyId
          bucketContext->secretAccessKey },           // secretAccessKey
        key,                                          // key
        0,                                            // queryParams
        0,                                            // subResource
        0,                                            // copySourceBucketName
        0,                                            // copySourceKey
        0,                                            // getConditions
        0,                                            // startByte
        0,                                            // byteCount
        putProperties,                                // putProperties
        handler->responseHandler.propertiesCallback,  // propertiesCallback
        handler->putObjectDataCallback,               // toS3Callback
        contentLength,                                // toS3CallbackTotalSize
        0,                                            // fromS3Callback
        handler->responseHandler.completeCallback,    // completeCallback
        callbackData                                  // callbackData
    };

    // Perform the request
    request_perform(&params, requestContext);
}


// copy object ---------------------------------------------------------------


typedef struct CopyObjectData
{
    SimpleXml simpleXml;

    S3ResponsePropertiesCallback *responsePropertiesCallback;
    S3ResponseCompleteCallback *responseCompleteCallback;
    void *callbackData;

    int64_t *lastModifiedReturn;
    int eTagReturnSize;
    char *eTagReturn;
    int eTagReturnLen;
    
    string_buffer(lastModified, 256);
} CopyObjectData;


static S3Status copyObjectXmlCallback(const char *elementPath,
                                      const char *data, int dataLen,
                                      void *callbackData)
{
    CopyObjectData *coData = (CopyObjectData *) callbackData;

    int fit;

    if (data) {
        if (!strcmp(elementPath, "CopyObjectResult/LastModified")) {
            string_buffer_append(coData->lastModified, data, dataLen, fit);
        }
        else if (!strcmp(elementPath, "CopyObjectResult/ETag")) {
            if (coData->eTagReturnSize && coData->eTagReturn) {
                coData->eTagReturnLen +=
                    snprintf(&(coData->eTagReturn[coData->eTagReturnLen]),
                             coData->eTagReturnSize - 
                             coData->eTagReturnLen - 1,
                             "%.*s", dataLen, data);
                if (coData->eTagReturnLen >= coData->eTagReturnSize) {
                    return S3StatusXmlParseFailure;
                }
            }
        }
    }

    /* Avoid compiler error about variable set but not used */
    (void) fit;

    return S3StatusOK;
}


static S3Status copyObjectPropertiesCallback
    (const S3ResponseProperties *responseProperties, void *callbackData)
{
    CopyObjectData *coData = (CopyObjectData *) callbackData;
    
    return (*(coData->responsePropertiesCallback))
        (responseProperties, coData->callbackData);
}


static S3Status copyObjectDataCallback(int bufferSize, const char *buffer,
                                       void *callbackData)
{
    CopyObjectData *coData = (CopyObjectData *) callbackData;

    return simplexml_add(&(coData->simpleXml), buffer, bufferSize);
}


static void copyObjectCompleteCallback(S3Status requestStatus, 
                                       const S3ErrorDetails *s3ErrorDetails,
                                       void *callbackData)
{
    CopyObjectData *coData = (CopyObjectData *) callbackData;

    if (coData->lastModifiedReturn) {
        time_t lastModified = -1;
        if (coData->lastModifiedLen) {
            lastModified = parseIso8601Time(coData->lastModified);
        }

        *(coData->lastModifiedReturn) = lastModified;
    }

    (*(coData->responseCompleteCallback))
        (requestStatus, s3ErrorDetails, coData->callbackData);

    simplexml_deinitialize(&(coData->simpleXml));

    free(coData);
}


void S3_copy_object(const S3BucketContext *bucketContext, const char *key,
                    const char *destinationBucket, const char *destinationKey,
                    const S3PutProperties *putProperties,
                    int64_t *lastModifiedReturn, int eTagReturnSize,
                    char *eTagReturn, S3RequestContext *requestContext,
                    const S3ResponseHandler *handler, void *callbackData)
{
    // Create the callback data
    CopyObjectData *data = 
        (CopyObjectData *) malloc(sizeof(CopyObjectData));
    if (!data) {
        (*(handler->completeCallback))(S3StatusOutOfMemory, 0, callbackData);
        return;
    }

    simplexml_initialize(&(data->simpleXml), &copyObjectXmlCallback, data);

    data->responsePropertiesCallback = handler->propertiesCallback;
    data->responseCompleteCallback = handler->completeCallback;
    data->callbackData = callbackData;

    data->lastModifiedReturn = lastModifiedReturn;
    data->eTagReturnSize = eTagReturnSize;
    data->eTagReturn = eTagReturn;
    if (data->eTagReturnSize && data->eTagReturn) {
        data->eTagReturn[0] = 0;
    }
    data->eTagReturnLen = 0;
    string_buffer_initialize(data->lastModified);

    // Set up the RequestParams
    RequestParams params =
    {
        HttpRequestTypeCOPY,                          // httpRequestType
        { bucketContext->hostName,                    // hostName
          destinationBucket ? destinationBucket : 
          bucketContext->bucketName,                  // bucketName
          bucketContext->protocol,                    // protocol
          bucketContext->uriStyle,                    // uriStyle
          bucketContext->accessKeyId,                 // accessKeyId
          bucketContext->secretAccessKey },           // secretAccessKey
        destinationKey ? destinationKey : key,        // key
        0,                                            // queryParams
        0,                                            // subResource
        bucketContext->bucketName,                    // copySourceBucketName
        key,                                          // copySourceKey
        0,                                            // getConditions
        0,                                            // startByte
        0,                                            // byteCount
        putProperties,                                // putProperties
        &copyObjectPropertiesCallback,                // propertiesCallback
        0,                                            // toS3Callback
        0,                                            // toS3CallbackTotalSize
        &copyObjectDataCallback,                      // fromS3Callback
        &copyObjectCompleteCallback,                  // completeCallback
        data                                          // callbackData
    };

    // Perform the request
    request_perform(&params, requestContext);
}


// get object ----------------------------------------------------------------

void S3_get_object(const S3BucketContext *bucketContext, const char *key,
                   const char* versionId,
                   const S3GetConditions *getConditions,
                   uint64_t startByte, uint64_t byteCount,
                   S3RequestContext *requestContext,
                   const S3GetObjectHandler *handler, void *callbackData)
{
    char versionIdParamName[] = "versionId=";
    char *queryParams = 0;
    if(versionId) {
        queryParams = malloc(sizeof(versionIdParamName) + strlen(versionId));
        strcpy(queryParams, versionIdParamName);
        strcpy(queryParams + sizeof(versionIdParamName) - 1, versionId);
    }
    // Set up the RequestParams
    RequestParams params =
    {
        HttpRequestTypeGET,                           // httpRequestType
        { bucketContext->hostName,                    // hostName
          bucketContext->bucketName,                  // bucketName
          bucketContext->protocol,                    // protocol
          bucketContext->uriStyle,                    // uriStyle
          bucketContext->accessKeyId,                 // accessKeyId
          bucketContext->secretAccessKey },           // secretAccessKey
        key,                                          // key
        0,                                            // queryParams
        queryParams,                                  // subResource
        0,                                            // copySourceBucketName
        0,                                            // copySourceKey
        getConditions,                                // getConditions
        startByte,                                    // startByte
        byteCount,                                    // byteCount
        0,                                            // putProperties
        handler->responseHandler.propertiesCallback,  // propertiesCallback
        0,                                            // toS3Callback
        0,                                            // toS3CallbackTotalSize
        handler->getObjectDataCallback,               // fromS3Callback
        handler->responseHandler.completeCallback,    // completeCallback
        callbackData                                  // callbackData
    };

    // Perform the request
    request_perform(&params, requestContext);
    if(queryParams)
        free(queryParams);
}


// head object ---------------------------------------------------------------

void S3_head_object(const S3BucketContext *bucketContext, const char *key,
                    S3RequestContext *requestContext,
                    const S3ResponseHandler *handler, void *callbackData)
{
    // Set up the RequestParams
    RequestParams params =
    {
        HttpRequestTypeHEAD,                          // httpRequestType
        { bucketContext->hostName,                    // hostName
          bucketContext->bucketName,                  // bucketName
          bucketContext->protocol,                    // protocol
          bucketContext->uriStyle,                    // uriStyle
          bucketContext->accessKeyId,                 // accessKeyId
          bucketContext->secretAccessKey },           // secretAccessKey
        key,                                          // key
        0,                                            // queryParams
        0,                                            // subResource
        0,                                            // copySourceBucketName
        0,                                            // copySourceKey
        0,                                            // getConditions
        0,                                            // startByte
        0,                                            // byteCount
        0,                                            // putProperties
        handler->propertiesCallback,                  // propertiesCallback
        0,                                            // toS3Callback
        0,                                            // toS3CallbackTotalSize
        0,                                            // fromS3Callback
        handler->completeCallback,                    // completeCallback
        callbackData                                  // callbackData
    };

    // Perform the request
    request_perform(&params, requestContext);
}
                         

// delete object --------------------------------------------------------------

void S3_delete_object(const S3BucketContext *bucketContext, const char *key,
                      S3RequestContext *requestContext,
                      const S3ResponseHandler *handler, void *callbackData)
{
    // Set up the RequestParams
    RequestParams params =
    {
        HttpRequestTypeDELETE,                        // httpRequestType
        { bucketContext->hostName,                    // hostName
          bucketContext->bucketName,                  // bucketName
          bucketContext->protocol,                    // protocol
          bucketContext->uriStyle,                    // uriStyle
          bucketContext->accessKeyId,                 // accessKeyId
          bucketContext->secretAccessKey },           // secretAccessKey
        key,                                          // key
        0,                                            // queryParams
        0,                                            // subResource
        0,                                            // copySourceBucketName
        0,                                            // copySourceKey
        0,                                            // getConditions
        0,                                            // startByte
        0,                                            // byteCount
        0,                                            // putProperties
        handler->propertiesCallback,                  // propertiesCallback
        0,                                            // toS3Callback
        0,                                            // toS3CallbackTotalSize
        0,                                            // fromS3Callback
        handler->completeCallback,                    // completeCallback
        callbackData                                  // callbackData
    };

    // Perform the request
    request_perform(&params, requestContext);
}
