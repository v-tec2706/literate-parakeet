//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package test.Ice.packagemd;

public class Server extends test.TestHelper
{
    public void run(String[] args)
    {
        com.zeroc.Ice.Properties properties = createTestProperties(args);
        properties.setProperty("Ice.Package.Test", "test.Ice.packagemd");
        properties.setProperty("Ice.Package.Test1", "test.Ice.packagemd");
        try(com.zeroc.Ice.Communicator communicator = initialize(properties))
        {
            properties.setProperty("TestAdapter.Endpoints", getTestEndpoint(0));
            com.zeroc.Ice.ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter");
            com.zeroc.Ice.Object object = new InitialI();
            adapter.add(object, com.zeroc.Ice.Util.stringToIdentity("initial"));
            adapter.activate();
            serverReady();
            communicator.waitForShutdown();
        }
    }
}
