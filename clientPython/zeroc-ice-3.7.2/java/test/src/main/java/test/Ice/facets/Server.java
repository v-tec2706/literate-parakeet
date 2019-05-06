//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package test.Ice.facets;

public class Server extends test.TestHelper
{
    public void run(String[] args)
    {
        com.zeroc.Ice.Properties properties = createTestProperties(args);
        properties.setProperty("Ice.Package.Test", "test.Ice.facets");
        try(com.zeroc.Ice.Communicator communicator = initialize(properties))
        {
            properties.setProperty("TestAdapter.Endpoints", getTestEndpoint(0));
            com.zeroc.Ice.ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter");
            com.zeroc.Ice.Object d = new DI();
            adapter.add(d, com.zeroc.Ice.Util.stringToIdentity("d"));
            adapter.addFacet(d, com.zeroc.Ice.Util.stringToIdentity("d"), "facetABCD");
            com.zeroc.Ice.Object f = new FI();
            adapter.addFacet(f, com.zeroc.Ice.Util.stringToIdentity("d"), "facetEF");
            com.zeroc.Ice.Object h = new HI(communicator);
            adapter.addFacet(h, com.zeroc.Ice.Util.stringToIdentity("d"), "facetGH");

            adapter.activate();
            serverReady();
            communicator.waitForShutdown();
        }
    }
}
