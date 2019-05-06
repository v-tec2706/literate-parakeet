//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <Ice/RequestHandlerFactory.h>
#include <Ice/CollocatedRequestHandler.h>
#include <Ice/ConnectRequestHandler.h>
#include <Ice/CollocatedRequestHandler.h>
#include <Ice/Reference.h>
#include <Ice/ObjectAdapterFactory.h>
#include <Ice/Instance.h>

using namespace std;
using namespace IceInternal;

RequestHandlerFactory::RequestHandlerFactory(const InstancePtr& instance) : _instance(instance)
{
}

RequestHandlerPtr
IceInternal::RequestHandlerFactory::getRequestHandler(const RoutableReferencePtr& ref, const Ice::ObjectPrxPtr& proxy)
{
    if(ref->getCollocationOptimized())
    {
        Ice::ObjectAdapterPtr adapter = _instance->objectAdapterFactory()->findObjectAdapter(proxy);
        if(adapter)
        {
            return proxy->_setRequestHandler(ICE_MAKE_SHARED(CollocatedRequestHandler, ref, adapter));
        }
    }

    ConnectRequestHandlerPtr handler;
    bool connect = false;
    if(ref->getCacheConnection())
    {
        Lock sync(*this);
        map<ReferencePtr, ConnectRequestHandlerPtr>::iterator p = _handlers.find(ref);
        if(p == _handlers.end())
        {
            handler = ICE_MAKE_SHARED(ConnectRequestHandler, ref, proxy);
            _handlers.insert(make_pair(ref, handler));
            connect = true;
        }
        else
        {
            handler = p->second;
        }
    }
    else
    {
        handler = ICE_MAKE_SHARED(ConnectRequestHandler, ref, proxy);
        connect = true;
    }
    if(connect)
    {
#ifdef ICE_CPP11_MAPPING
        ref->getConnection(handler);
#else
        ref->getConnection(handler.get());
#endif
    }
    return proxy->_setRequestHandler(handler->connect(proxy));
}

void
IceInternal::RequestHandlerFactory::removeRequestHandler(const ReferencePtr& ref, const RequestHandlerPtr& handler)
{
    if(ref->getCacheConnection())
    {
        Lock sync(*this);
        map<ReferencePtr, ConnectRequestHandlerPtr>::iterator p = _handlers.find(ref);
        if(p != _handlers.end() && p->second.get() == handler.get())
        {
            _handlers.erase(p);
        }
    }
}
