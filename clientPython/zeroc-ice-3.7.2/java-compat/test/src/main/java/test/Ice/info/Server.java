//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package test.Ice.info;

public class Server extends test.TestHelper
{
    public void run(String[] args)
    {
        Ice.Properties properties = createTestProperties(args);
        properties.setProperty("Ice.Package.Test", "test.Ice.proxy");
        try(Ice.Communicator communicator = initialize(properties))
        {
            properties.setProperty("TestAdapter.Endpoints", getTestEndpoint(0) + ":" + getTestEndpoint(1, "udp"));

            Ice.ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter");
            adapter.add(new TestI(), Ice.Util.stringToIdentity("test"));
            adapter.activate();

            serverReady();
            communicator.waitForShutdown();
        }
    }
}
