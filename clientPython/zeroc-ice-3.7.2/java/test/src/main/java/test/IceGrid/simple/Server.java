//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package test.IceGrid.simple;

public class Server extends test.TestHelper
{
    public void run(String[] args)
    {
        com.zeroc.Ice.Properties properties = createTestProperties(args);
        //
        // It's possible to have batch oneway requests dispatched
        // after the adapter is deactivated due to thread
        // scheduling so we suppress this warning.
        //
        properties.setProperty("Ice.Warn.Dispatch", "0");
        properties.setProperty("Ice.Package.Test", "test.IceGrid.simple");
        try(com.zeroc.Ice.Communicator communicator = initialize(args))
        {
            communicator.getProperties().parseCommandLineOptions("TestAdapter", args);

            com.zeroc.Ice.ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter");
            com.zeroc.Ice.Object object = new TestI();
            String id = communicator.getProperties().getPropertyWithDefault("Identity", "test");
            adapter.add(object, com.zeroc.Ice.Util.stringToIdentity(id));
            try
            {
                adapter.activate();
            }
            catch(com.zeroc.Ice.ObjectAdapterDeactivatedException ex)
            {
            }
            communicator().waitForShutdown();
        }
    }
}
