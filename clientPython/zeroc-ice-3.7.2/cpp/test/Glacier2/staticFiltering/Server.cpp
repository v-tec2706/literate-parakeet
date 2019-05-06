//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <Ice/Ice.h>
#include <BackendI.h>
#include <TestHelper.h>

using namespace std;
using namespace Ice;
using namespace Test;

//
// Dummy ServerLocatorRegistry, ServerLocator and ServantLocator. For
// simplicity, we essentially 'alias' all possible requests to a single
// object adapter and a single servant.
//
class ServerLocatorRegistry : public virtual LocatorRegistry
{
public:

    virtual void
    setAdapterDirectProxy_async(const AMD_LocatorRegistry_setAdapterDirectProxyPtr& cb, const string&,
                                const ObjectPrx&, const Current&)
    {
        cb->ice_response();
    }

    virtual void
    setReplicatedAdapterDirectProxy_async(const AMD_LocatorRegistry_setReplicatedAdapterDirectProxyPtr& cb,
                                          const string&, const string&, const ObjectPrx&, const Current&)
    {
        cb->ice_response();
    }

    virtual void
    setServerProcessProxy_async(const AMD_LocatorRegistry_setServerProcessProxyPtr& cb,
                                const string&, const ProcessPrx&, const Current&)
    {
        cb->ice_response();
    }
};

class ServerLocatorI : public virtual Locator
{
public:
    ServerLocatorI(const BackendPtr& backend, const ObjectAdapterPtr& adapter) :
        _backend(backend),
        _adapter(adapter),
        _registryPrx(
            LocatorRegistryPrx::uncheckedCast(
                adapter->add(new ServerLocatorRegistry, Ice::stringToIdentity("registry"))))
    {
    }

    virtual void
    findObjectById_async(const AMD_Locator_findObjectByIdPtr& cb, const Identity& id, const Current&) const
    {
        cb->ice_response(_adapter->createProxy(id));
    }

    virtual void
    findAdapterById_async(const AMD_Locator_findAdapterByIdPtr& cb, const string&, const Current&) const
    {
       cb->ice_response(_adapter->createDirectProxy(stringToIdentity("dummy")));
    }

    virtual LocatorRegistryPrx
    getRegistry(const Current&) const
    {
        return _registryPrx;
    }

private:
    const BackendPtr _backend;
    const ObjectAdapterPtr _adapter;
    const LocatorRegistryPrx _registryPrx;
};

class ServantLocatorI : public virtual ServantLocator
{
public:

    ServantLocatorI(const BackendPtr& backend) :
        _backend(backend)
    {
    }

    virtual ObjectPtr locate(const Current&, LocalObjectPtr&)
    {
        return _backend;
    }

    virtual void finished(const Current&, const ObjectPtr&, const LocalObjectPtr&)
    {
    }

    virtual void deactivate(const string&)
    {
    }

private:

    const BackendPtr _backend;
};

class BackendServer : public Test::TestHelper
{
public:

    void run(int, char**);
};

void
BackendServer::run(int argc, char** argv)
{
    Ice::CommunicatorHolder communicator = initialize(argc, argv);
    string endpoints = communicator->getProperties()->getPropertyWithDefault("BackendAdapter.Endpoints",
                                                                             "tcp -p 12010:ssl -p 12011");

    communicator->getProperties()->setProperty("BackendAdapter.Endpoints", endpoints);
    ObjectAdapterPtr adapter = communicator->createObjectAdapter("BackendAdapter");
    BackendPtr backend = new BackendI;
    Ice::LocatorPtr locator = new ServerLocatorI(backend, adapter);
    adapter->add(locator, Ice::stringToIdentity("locator"));
    adapter->addServantLocator(new ServantLocatorI(backend), "");
    adapter->activate();
    communicator->waitForShutdown();
}

DEFINE_TEST(BackendServer)
