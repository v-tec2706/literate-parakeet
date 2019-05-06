//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package test.Ice.adapterDeactivation;

public class Collocated extends test.TestHelper
{
    public void run(String[] args)
    {
        com.zeroc.Ice.Properties properties = createTestProperties(args);
        properties.setProperty("Ice.Package.Test", "test.Ice.adapterDeactivation");

        try(com.zeroc.Ice.Communicator communicator = initialize(properties))
        {
            //
            // 2 threads are necessary to dispatch the collocated transient() call with AMI
            //
            communicator.getProperties().setProperty("TestAdapter.ThreadPool.Size", "2");
            communicator.getProperties().setProperty("TestAdapter.Endpoints", getTestEndpoint());
            com.zeroc.Ice.ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter");
            adapter.addServantLocator(new ServantLocatorI(), "");
            AllTests.allTests(this);
            adapter.waitForDeactivate();
        }
    }
}
