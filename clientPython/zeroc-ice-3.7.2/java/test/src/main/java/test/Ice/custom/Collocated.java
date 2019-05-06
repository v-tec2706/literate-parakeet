//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package test.Ice.custom;

public class Collocated extends test.TestHelper
{
    public void run(String[] args)
    {
        com.zeroc.Ice.Properties properties = createTestProperties(args);
        properties.setProperty("Ice.Package.Test", "test.Ice.custom");
        properties.setProperty("Ice.CacheMessageBuffers", "0");
        try(com.zeroc.Ice.Communicator communicator = initialize(properties))
        {
            communicator.getProperties().setProperty("TestAdapter.Endpoints", getTestEndpoint(0));
            com.zeroc.Ice.ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter");
            com.zeroc.Ice.Object test = new TestI(communicator);
            adapter.add(test, com.zeroc.Ice.Util.stringToIdentity("test"));
            AllTests.allTests(this);
        }
    }
}
