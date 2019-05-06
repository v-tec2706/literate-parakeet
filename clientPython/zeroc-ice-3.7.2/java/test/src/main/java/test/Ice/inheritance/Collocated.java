//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package test.Ice.inheritance;

public class Collocated extends test.TestHelper
{
    @Override
    public void run(String[] args)
    {
        com.zeroc.Ice.Properties properties = createTestProperties(args);
        properties.setProperty("Ice.Package.Test", "test.Ice.inheritance");
        try(com.zeroc.Ice.Communicator communicator = initialize(properties))
        {
            communicator.getProperties().setProperty("TestAdapter.Endpoints", getTestEndpoint(0));
            com.zeroc.Ice.ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter");
            adapter.add(new InitialI(adapter), com.zeroc.Ice.Util.stringToIdentity("initial"));
            AllTests.allTests(this);
        }
    }
}
