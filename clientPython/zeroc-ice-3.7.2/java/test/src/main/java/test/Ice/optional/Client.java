//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package test.Ice.optional;

import test.Ice.optional.Test.InitialPrx;

public class Client extends test.TestHelper
{
    public void run(String[] args)
    {
        com.zeroc.Ice.Properties properties = createTestProperties(args);
        properties.setProperty("Ice.Package.Test", "test.Ice.optional");
        try(com.zeroc.Ice.Communicator communicator = initialize(properties))
        {
            InitialPrx initial = AllTests.allTests(this, false);
            initial.shutdown();
        }
    }
}
