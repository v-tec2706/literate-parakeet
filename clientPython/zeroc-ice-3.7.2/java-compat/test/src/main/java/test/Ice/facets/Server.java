//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package test.Ice.facets;

public class Server extends test.TestHelper
{
    public void run(String[] args)
    {
        Ice.Properties properties = createTestProperties(args);
        properties.setProperty("Ice.Package.Test", "test.Ice.facets");
        try(Ice.Communicator communicator = initialize(properties))
        {
            communicator.getProperties().setProperty("TestAdapter.Endpoints", getTestEndpoint(0));
            Ice.ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter");
            Ice.Object d = new DI();
            adapter.add(d, Ice.Util.stringToIdentity("d"));
            adapter.addFacet(d, Ice.Util.stringToIdentity("d"), "facetABCD");
            Ice.Object f = new FI();
            adapter.addFacet(f, Ice.Util.stringToIdentity("d"), "facetEF");
            Ice.Object h = new HI(communicator);
            adapter.addFacet(h, Ice.Util.stringToIdentity("d"), "facetGH");

            adapter.activate();

            serverReady();
            communicator.waitForShutdown();
        }
    }
}
