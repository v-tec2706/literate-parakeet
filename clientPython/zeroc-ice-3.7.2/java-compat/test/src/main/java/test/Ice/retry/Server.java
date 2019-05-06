//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package test.Ice.retry;

public class Server extends test.TestHelper
{
    public void run(String[] args)
    {
        Ice.Properties properties = createTestProperties(args);
        properties.setProperty("Ice.Package.Test", "test.Ice.retry");
        properties.setProperty("Ice.Warn.Dispatch", "0");
        try(Ice.Communicator communicator = initialize(properties))
        {
            communicator.getProperties().setProperty("TestAdapter.Endpoints", getTestEndpoint(0));
            Ice.ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter");
            adapter.add(new RetryI(), Ice.Util.stringToIdentity("retry"));
            adapter.activate();
            serverReady();
            communicator.waitForShutdown();
        }
    }
}
