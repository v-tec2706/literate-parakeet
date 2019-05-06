//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package com.zeroc.Ice;

/**
 * Base interface for listening to batch request queues.
 **/
@FunctionalInterface
public interface BatchRequestInterceptor
{
    /**
     * Called by the Ice run time when a batch request is about to be
     * added to the batch request queue of a proxy or connection.
     *
     * The implementation of this method must call enqueue() on the
     * request to confirm its addition to the queue, if not called
     * the request isn't added to the queue. The implementation can
     * raise an Ice local exception to notify the caller of a failure.
     *
     * @param request The request to be added.
     * @param queueBatchRequestCount The number of requests in the batch.
     * @param queueBatchRequestSize The number of bytes in the batch.
     **/
    void enqueue(BatchRequest request, int queueBatchRequestCount, int queueBatchRequestSize);
}
