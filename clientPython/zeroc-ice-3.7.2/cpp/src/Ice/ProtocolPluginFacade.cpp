//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <Ice/ProtocolPluginFacade.h>
#include <Ice/Instance.h>
#include <Ice/EndpointFactoryManager.h>
#include <Ice/TraceLevels.h>
#include <Ice/Initialize.h>
#include <Ice/DefaultsAndOverrides.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

IceUtil::Shared* IceInternal::upCast(ProtocolPluginFacade* p) { return p; }

IceInternal::ProtocolPluginFacade::~ProtocolPluginFacade()
{
    // Out of line to avoid weak vtable
}

ProtocolPluginFacadePtr
IceInternal::getProtocolPluginFacade(const CommunicatorPtr& communicator)
{
    return new ProtocolPluginFacade(communicator);
}

CommunicatorPtr
IceInternal::ProtocolPluginFacade::getCommunicator() const
{
    return _communicator;
}

void
IceInternal::ProtocolPluginFacade::addEndpointFactory(const EndpointFactoryPtr& factory) const
{
    _instance->endpointFactoryManager()->add(factory);
}

EndpointFactoryPtr
IceInternal::ProtocolPluginFacade::getEndpointFactory(Ice::Short type) const
{
    return _instance->endpointFactoryManager()->get(type);
}

IceInternal::ProtocolPluginFacade::ProtocolPluginFacade(const CommunicatorPtr& communicator) :
    _instance(getInstance(communicator)),
    _communicator(communicator)
{
}
