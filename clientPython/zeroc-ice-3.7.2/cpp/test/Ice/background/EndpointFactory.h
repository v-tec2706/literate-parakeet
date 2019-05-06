//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef TEST_ENDPOINT_FACTORY_H
#define TEST_ENDPOINT_FACTORY_H

#include <Ice/EndpointFactory.h>

class EndpointFactory : public IceInternal::EndpointFactory
{
public:

    virtual ~EndpointFactory() { }

    virtual ::Ice::Short type() const;
    virtual ::std::string protocol() const;
    virtual IceInternal::EndpointIPtr create(std::vector<std::string>&, bool) const;
    virtual IceInternal::EndpointIPtr read(Ice::InputStream*) const;
    virtual void destroy();

    virtual IceInternal::EndpointFactoryPtr clone(const IceInternal::ProtocolInstancePtr&) const;

protected:

    EndpointFactory(const IceInternal::EndpointFactoryPtr&);
    friend class TestPluginI;

    IceInternal::EndpointFactoryPtr _factory;
};

#endif
