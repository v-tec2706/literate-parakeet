//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_REQUEST_HANDLER_H
#define ICE_REQUEST_HANDLER_H

#include <IceUtil/Shared.h>
#include <Ice/UniquePtr.h>

#include <Ice/RequestHandlerF.h>
#include <Ice/ReferenceF.h>
#include <Ice/OutgoingAsyncF.h>
#include <Ice/ProxyF.h>
#include <Ice/ConnectionIF.h>

namespace Ice
{

class LocalException;

}

namespace IceInternal
{

//
// An exception wrapper, which is used to notify that the request
// handler should be cleared and the invocation retried.
//
class RetryException
{
public:

    RetryException(const Ice::LocalException&);
    RetryException(const RetryException&);

    const Ice::LocalException* get() const;

private:

    IceInternal::UniquePtr<Ice::LocalException> _ex;
};

class CancellationHandler
#ifndef ICE_CPP11_MAPPING
    : public virtual IceUtil::Shared
#endif
{
public:

    virtual void asyncRequestCanceled(const OutgoingAsyncBasePtr&, const Ice::LocalException&) = 0;
};

class RequestHandler : public CancellationHandler
{
public:

    RequestHandler(const ReferencePtr&);

    virtual RequestHandlerPtr update(const RequestHandlerPtr&, const RequestHandlerPtr&) = 0;

    virtual AsyncStatus sendAsyncRequest(const ProxyOutgoingAsyncBasePtr&) = 0;

    const ReferencePtr& getReference() const { return _reference; } // Inlined for performances.

    virtual Ice::ConnectionIPtr getConnection() = 0;
    virtual Ice::ConnectionIPtr waitForConnection() = 0;

protected:

    const ReferencePtr _reference;
    const bool _response;
};

}

#endif
