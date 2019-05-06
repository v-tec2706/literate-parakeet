//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package test.Ice.proxy;

import test.Ice.proxy.Test.MyClassPrx;

public class Client extends test.TestHelper
{
    public void run(String[] args)
    {
        com.zeroc.Ice.Properties properties = createTestProperties(args);
        properties.setProperty("Ice.Package.Test", "test.Ice.proxy");
        try(com.zeroc.Ice.Communicator communicator = initialize(properties))
        {
            MyClassPrx myClass = AllTests.allTests(this);
            myClass.shutdown();
        }
    }
}
