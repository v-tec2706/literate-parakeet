//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package test.Ice.info;

public class Server extends test.TestHelper
{
    public void run(String[] args)
    {
        com.zeroc.Ice.Properties properties = createTestProperties(args);
        properties.setProperty("Ice.Package.Test", "test.Ice.proxy");
        try(com.zeroc.Ice.Communicator communicator = initialize(properties))
        {
            properties.setProperty("TestAdapter.Endpoints",
                                   getTestEndpoint(0) + ":" + getTestEndpoint(0, "udp"));
            com.zeroc.Ice.ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter");
            adapter.add(new TestI(), com.zeroc.Ice.Util.stringToIdentity("test"));
            adapter.activate();
            serverReady();
            communicator.waitForShutdown();
        }
    }
}
