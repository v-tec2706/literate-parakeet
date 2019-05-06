//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package test.Ice.invoke;

public class Server extends test.TestHelper
{
    public void run(String[] args)
    {
        Ice.Properties properties = createTestProperties(args);
        properties.setProperty("Ice.Package.Test", "test.Ice.invoke");
        boolean async = false;
        for(int i = 0; i < args.length; ++i)
        {
            if(args[i].equals("--async"))
            {
               async = true;
            }
        }

        try(Ice.Communicator communicator = initialize(properties))
        {
            communicator().getProperties().setProperty("TestAdapter.Endpoints", getTestEndpoint(0));
            Ice.ObjectAdapter adapter = communicator().createObjectAdapter("TestAdapter");
            adapter.addServantLocator(new ServantLocatorI(async), "");
            adapter.activate();

            serverReady();
            communicator.waitForShutdown();
        }
    }
}
