//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <Ice/Ice.h>
#include <TestHelper.h>
#include <Test.h>

using namespace std;
using namespace Ice;
using namespace Test;

void
allTests(Test::TestHelper* helper, int num)
{
    CommunicatorPtr communicator = helper->communicator();
    vector<ControllerPrxPtr> proxies;
    vector<ControllerPrxPtr> indirectProxies;
    for(int i = 0; i < num; ++i)
    {
        {
            ostringstream os;
            os << "controller" << i;
            proxies.push_back(ICE_UNCHECKED_CAST(ControllerPrx, communicator->stringToProxy(os.str())));
        }
        {
            ostringstream os;
            os << "controller" << i << "@control" << i;
            indirectProxies.push_back(ICE_UNCHECKED_CAST(ControllerPrx, communicator->stringToProxy(os.str())));
        }
    }

    cout << "testing indirect proxies... " << flush;
    {
        for(vector<ControllerPrxPtr>::const_iterator p = indirectProxies.begin(); p != indirectProxies.end(); ++p)
        {
            (*p)->ice_ping();
        }
    }
    cout << "ok" << endl;

    cout << "testing well-known proxies... " << flush;
    {
        for(vector<ControllerPrxPtr>::const_iterator p = proxies.begin(); p != proxies.end(); ++p)
        {
            (*p)->ice_ping();
        }
    }
    cout << "ok" << endl;

    cout << "testing object adapter registration... " << flush;
    {
        try
        {
            communicator->stringToProxy("object @ oa1")->ice_ping();
            test(false);
        }
        catch(const Ice::NoEndpointException&)
        {
        }

        proxies[0]->activateObjectAdapter("oa", "oa1", "");

        try
        {
            communicator->stringToProxy("object @ oa1")->ice_ping();
            test(false);
        }
        catch(const Ice::ObjectNotExistException&)
        {
        }

        proxies[0]->deactivateObjectAdapter("oa");

        try
        {
            communicator->stringToProxy("object @ oa1")->ice_ping();
            test(false);
        }
        catch(const Ice::NoEndpointException&)
        {
        }
    }
    cout << "ok" << endl;

    cout << "testing object adapter migration..." << flush;
    {
        proxies[0]->activateObjectAdapter("oa", "oa1", "");
        proxies[0]->addObject("oa", "object");
        communicator->stringToProxy("object @ oa1")->ice_ping();
        proxies[0]->removeObject("oa", "object");
        proxies[0]->deactivateObjectAdapter("oa");

        proxies[1]->activateObjectAdapter("oa", "oa1", "");
        proxies[1]->addObject("oa", "object");
        communicator->stringToProxy("object @ oa1")->ice_ping();
        proxies[1]->removeObject("oa", "object");
        proxies[1]->deactivateObjectAdapter("oa");
    }
    cout << "ok" << endl;

    cout << "testing object migration..." << flush;
    {
        proxies[0]->activateObjectAdapter("oa", "oa1", "");
        proxies[1]->activateObjectAdapter("oa", "oa2", "");

        proxies[0]->addObject("oa", "object");
        communicator->stringToProxy("object @ oa1")->ice_ping();
        communicator->stringToProxy("object")->ice_ping();
        proxies[0]->removeObject("oa", "object");

        proxies[1]->addObject("oa", "object");
        communicator->stringToProxy("object @ oa2")->ice_ping();
        communicator->stringToProxy("object")->ice_ping();
        proxies[1]->removeObject("oa", "object");

        try
        {
            communicator->stringToProxy("object @ oa1")->ice_ping();
        }
        catch(const Ice::ObjectNotExistException&)
        {
        }
        try
        {
            communicator->stringToProxy("object @ oa2")->ice_ping();
        }
        catch(const Ice::ObjectNotExistException&)
        {
        }

        proxies[0]->deactivateObjectAdapter("oa");
        proxies[1]->deactivateObjectAdapter("oa");
    }
    cout << "ok" << endl;

    cout << "testing replica groups..." << flush;
    {
        proxies[0]->activateObjectAdapter("oa", "oa1", "rg");
        proxies[1]->activateObjectAdapter("oa", "oa2", "rg");
        proxies[2]->activateObjectAdapter("oa", "oa3", "rg");

        proxies[0]->addObject("oa", "object");
        proxies[1]->addObject("oa", "object");
        proxies[2]->addObject("oa", "object");

        communicator->stringToProxy("object @ oa1")->ice_ping();
        communicator->stringToProxy("object @ oa2")->ice_ping();
        communicator->stringToProxy("object @ oa3")->ice_ping();

        communicator->stringToProxy("object @ rg")->ice_ping();

        set<string> adapterIds;
        adapterIds.insert("oa1");
        adapterIds.insert("oa2");
        adapterIds.insert("oa3");
        TestIntfPrxPtr intf = ICE_UNCHECKED_CAST(TestIntfPrx, communicator->stringToProxy("object"));
        intf = intf->ice_connectionCached(false)->ice_locatorCacheTimeout(0);
        while(!adapterIds.empty())
        {
            adapterIds.erase(intf->getAdapterId());
        }

        while(true)
        {
            adapterIds.insert("oa1");
            adapterIds.insert("oa2");
            adapterIds.insert("oa3");
            intf = ICE_UNCHECKED_CAST(TestIntfPrx, communicator->stringToProxy("object @ rg"))->ice_connectionCached(false);
            int nRetry = 100;
            while(!adapterIds.empty() && --nRetry > 0)
            {
                adapterIds.erase(intf->getAdapterId());
            }
            if(nRetry > 0)
            {
                break;
            }

            // The previous locator lookup probably didn't return all the replicas... try again.
            communicator->stringToProxy("object @ rg")->ice_locatorCacheTimeout(0)->ice_ping();
        }

        proxies[0]->deactivateObjectAdapter("oa");
        proxies[1]->deactivateObjectAdapter("oa");
        test(ICE_UNCHECKED_CAST(TestIntfPrx, communicator->stringToProxy("object @ rg"))->getAdapterId() == "oa3");
        proxies[2]->deactivateObjectAdapter("oa");

        proxies[0]->activateObjectAdapter("oa", "oa1", "rg");
        proxies[0]->addObject("oa", "object");
        test(ICE_UNCHECKED_CAST(TestIntfPrx, communicator->stringToProxy("object @ rg"))->getAdapterId() == "oa1");
        proxies[0]->deactivateObjectAdapter("oa");
    }
    cout << "ok" << endl;

    cout << "testing invalid lookup endpoints... " << flush;
    {
        string multicast;
        if(communicator->getProperties()->getProperty("Ice.IPv6") == "1")
        {
            multicast = "\"ff15::1\"";
        }
        else
        {
            multicast = "239.255.0.1";
        }

        {

            Ice::InitializationData initData;
            initData.properties = communicator->getProperties()->clone();
            initData.properties->setProperty("IceDiscovery.Lookup", "udp -h " + multicast + " --interface unknown");
            Ice::CommunicatorPtr com = Ice::initialize(initData);
            test(com->getDefaultLocator());
            try
            {
                com->stringToProxy("controller0@control0")->ice_ping();
                test(false);
            }
            catch(const Ice::LocalException&)
            {
            }
            com->destroy();
        }
        {
            Ice::InitializationData initData;
            initData.properties = communicator->getProperties()->clone();
            string intf = initData.properties->getProperty("IceDiscovery.Interface");
            if(!intf.empty())
            {
                intf = " --interface \"" + intf + "\"";
            }
            string port = initData.properties->getProperty("IceDiscovery.Port");
            initData.properties->setProperty("IceDiscovery.Lookup",
                                             "udp -h " + multicast + " --interface unknown:" +
                                             "udp -h " + multicast + " -p " + port + intf);
            Ice::CommunicatorPtr com = Ice::initialize(initData);
            test(com->getDefaultLocator());
            com->stringToProxy("controller0@control0")->ice_ping();
            com->destroy();
        }
    }
    cout << "ok" << endl;

    cout << "shutting down... " << flush;
    for(vector<ControllerPrxPtr>::const_iterator p = proxies.begin(); p != proxies.end(); ++p)
    {
        (*p)->shutdown();
    }
    cout << "ok" << endl;
}
