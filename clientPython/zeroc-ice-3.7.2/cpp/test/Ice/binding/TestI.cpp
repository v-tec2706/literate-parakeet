//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <Ice/Ice.h>
#include <TestI.h>
#include <TestHelper.h>

using namespace std;
using namespace Ice;
using namespace Test;

RemoteCommunicatorI::RemoteCommunicatorI() :
    _nextPort(1)
{
}

#ifdef ICE_CPP11_MAPPING
shared_ptr<Test::RemoteObjectAdapterPrx>
RemoteCommunicatorI::createObjectAdapter(string name, string endpts, const Ice::Current& current)
#else
RemoteObjectAdapterPrx
RemoteCommunicatorI::createObjectAdapter(const string& name, const string& endpts, const Current& current)
#endif
{
    Ice::CommunicatorPtr com = current.adapter->getCommunicator();
    const string defaultProtocol = com->getProperties()->getProperty("Ice.Default.Protocol");
    int retry = 5;
    while(true)
    {
        try
        {
            string endpoints = endpts;
            if(defaultProtocol != "bt")
            {
                if(endpoints.find("-p") == string::npos)
                {
                    endpoints = TestHelper::getTestEndpoint(com->getProperties(), _nextPort++, endpoints);
                }
            }
            com->getProperties()->setProperty(name + ".ThreadPool.Size", "1");
            ObjectAdapterPtr adapter = com->createObjectAdapterWithEndpoints(name, endpoints);
            return ICE_UNCHECKED_CAST(RemoteObjectAdapterPrx,
                                      current.adapter->addWithUUID(ICE_MAKE_SHARED(RemoteObjectAdapterI, adapter)));
        }
        catch(const Ice::SocketException&)
        {
            if(--retry == 0)
            {
                throw;
            }
        }
    }
}

#ifdef ICE_CPP11_MAPPING
void
RemoteCommunicatorI::deactivateObjectAdapter(shared_ptr<RemoteObjectAdapterPrx> adapter, const Current&)
#else
void
RemoteCommunicatorI::deactivateObjectAdapter(const RemoteObjectAdapterPrx& adapter, const Current&)
#endif
{
    adapter->deactivate(); // Collocated call
}

void
RemoteCommunicatorI::shutdown(const Ice::Current& current)
{
    current.adapter->getCommunicator()->shutdown();
}

RemoteObjectAdapterI::RemoteObjectAdapterI(const Ice::ObjectAdapterPtr& adapter) :
    _adapter(adapter),
    _testIntf(ICE_UNCHECKED_CAST(TestIntfPrx,
                    _adapter->add(ICE_MAKE_SHARED(TestI),
                                  stringToIdentity("test"))))
{
    _adapter->activate();
}

TestIntfPrxPtr
RemoteObjectAdapterI::getTestIntf(const Ice::Current&)
{
    return _testIntf;
}

void
RemoteObjectAdapterI::deactivate(const Ice::Current&)
{
    try
    {
        _adapter->destroy();
    }
    catch(const ObjectAdapterDeactivatedException&)
    {
    }
}

std::string
TestI::getAdapterName(const Ice::Current& current)
{
    return current.adapter->getName();
}
