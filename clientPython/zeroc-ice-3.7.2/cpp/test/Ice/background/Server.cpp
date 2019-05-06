//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <Ice/Ice.h>
#include <TestHelper.h>
#include <TestI.h>
#include <Configuration.h>
#include <PluginI.h>

#include <Ice/Locator.h>
#include <Ice/Router.h>

#ifdef _MSC_VER
#   pragma comment(lib, ICE_LIBNAME("testtransport"))
#endif

using namespace std;

extern "C"
{

Ice::Plugin* createTestTransport(const Ice::CommunicatorPtr&, const std::string&, const Ice::StringSeq&);

};

class LocatorI : public Ice::Locator
{
public:

#ifdef ICE_CPP11_MAPPING
    virtual void
    findAdapterByIdAsync(string,
                         function<void(const shared_ptr<Ice::ObjectPrx>&)> response,
                         function<void(exception_ptr)>,
                         const Ice::Current& current) const
    {
        _controller->checkCallPause(current);
        Ice::CommunicatorPtr communicator = current.adapter->getCommunicator();
        response(current.adapter->createDirectProxy(Ice::stringToIdentity("dummy")));
    }

    virtual void
    findObjectByIdAsync(Ice::Identity id,
                        function<void(const shared_ptr<Ice::ObjectPrx>&)> response,
                        function<void(exception_ptr)>,
                        const Ice::Current& current) const
    {
        _controller->checkCallPause(current);
        Ice::CommunicatorPtr communicator = current.adapter->getCommunicator();
        response(current.adapter->createDirectProxy(id));
    }
#else
    virtual void
    findAdapterById_async(const Ice::AMD_Locator_findAdapterByIdPtr& response, const string&,
                          const Ice::Current& current) const
    {
        _controller->checkCallPause(current);
        Ice::CommunicatorPtr communicator = current.adapter->getCommunicator();
        response->ice_response(current.adapter->createDirectProxy(Ice::stringToIdentity("dummy")));
    }

    virtual void
    findObjectById_async(const Ice::AMD_Locator_findObjectByIdPtr& response, const Ice::Identity& id,
                         const Ice::Current& current) const
    {
        _controller->checkCallPause(current);
        Ice::CommunicatorPtr communicator = current.adapter->getCommunicator();
        response->ice_response(current.adapter->createDirectProxy(id));
    }
#endif
    virtual Ice::LocatorRegistryPrxPtr
    getRegistry(const Ice::Current&) const
    {
        return ICE_NULLPTR;
    }

    LocatorI(const BackgroundControllerIPtr& controller) : _controller(controller)
    {
    }

private:

    BackgroundControllerIPtr _controller;
};

class RouterI : public Ice::Router
{
public:

    virtual Ice::ObjectPrxPtr
    getClientProxy(IceUtil::Optional<bool>& hasRoutingTable, const Ice::Current& current) const
    {
        hasRoutingTable = true;
        _controller->checkCallPause(current);
        return ICE_NULLPTR;
    }

    virtual Ice::ObjectPrxPtr
    getServerProxy(const Ice::Current& current) const
    {
        _controller->checkCallPause(current);
        return ICE_NULLPTR;
    }

    virtual Ice::ObjectProxySeq
    addProxies(ICE_IN(Ice::ObjectProxySeq), const Ice::Current&)
    {
        return Ice::ObjectProxySeq();
    }

    RouterI(const BackgroundControllerIPtr& controller)
    {
        _controller = controller;
    }

private:

    BackgroundControllerIPtr _controller;
};

class Server : public Test::TestHelper
{
public:

    void run(int, char**);
};

void
Server::run(int argc, char** argv)
{
#ifdef ICE_STATIC_LIBS
    Ice::registerPluginFactory("Test", createTestTransport, false);
#endif
    Ice::PropertiesPtr properties = createTestProperties(argc, argv);

    //
    // This test kills connections, so we don't want warnings.
    //
    properties->setProperty("Ice.Warn.Connections", "0");

    properties->setProperty("Ice.MessageSizeMax", "50000");

    //
    // This test relies on filling the TCP send/recv buffer, so
    // we rely on a fixed value for these buffers.
    //
    properties->setProperty("Ice.TCP.RcvSize", "50000");

    //
    // Setup the test transport plug-in.
    //
    properties->setProperty("Ice.Plugin.Test", "TestTransport:createTestTransport");
    string defaultProtocol = properties->getPropertyWithDefault("Ice.Default.Protocol", "tcp");
    properties->setProperty("Ice.Default.Protocol", "test-" + defaultProtocol);

    Ice::CommunicatorHolder communicator = initialize(argc, argv, properties);

    communicator->getProperties()->setProperty("TestAdapter.Endpoints", getTestEndpoint(0));
    communicator->getProperties()->setProperty("ControllerAdapter.Endpoints", getTestEndpoint(1, "tcp"));
    communicator->getProperties()->setProperty("ControllerAdapter.ThreadPool.Size", "1");

    Ice::ObjectAdapterPtr adapter = communicator->createObjectAdapter("TestAdapter");
    Ice::ObjectAdapterPtr adapter2 = communicator->createObjectAdapter("ControllerAdapter");

#ifdef ICE_CPP11_MAPPING
    shared_ptr<PluginI> plugin = dynamic_pointer_cast<PluginI>(communicator->getPluginManager()->getPlugin("Test"));
#else
    PluginI* plugin = dynamic_cast<PluginI*>(communicator->getPluginManager()->getPlugin("Test").get());
#endif
    assert(plugin);
    ConfigurationPtr configuration = plugin->getConfiguration();
    BackgroundControllerIPtr backgroundController = ICE_MAKE_SHARED(BackgroundControllerI, adapter, configuration);

    adapter->add(ICE_MAKE_SHARED(BackgroundI, backgroundController), Ice::stringToIdentity("background"));
    adapter->add(ICE_MAKE_SHARED(LocatorI, backgroundController), Ice::stringToIdentity("locator"));
    adapter->add(ICE_MAKE_SHARED(RouterI, backgroundController), Ice::stringToIdentity("router"));
    adapter->activate();

    adapter2->add(backgroundController, Ice::stringToIdentity("backgroundController"));
    adapter2->activate();

    communicator->waitForShutdown();
}

DEFINE_TEST(Server)
