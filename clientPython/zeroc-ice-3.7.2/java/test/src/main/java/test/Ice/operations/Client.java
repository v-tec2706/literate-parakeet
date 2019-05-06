//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package test.Ice.operations;

import test.Ice.operations.Test.MyClassPrx;

public class Client extends test.TestHelper
{
    public void run(String[] args)
    {
        com.zeroc.Ice.Properties properties = createTestProperties(args);
        properties.setProperty("Ice.ThreadPool.Client.Size", "2");
        properties.setProperty("Ice.ThreadPool.Client.SizeWarn", "0");
        properties.setProperty("Ice.Package.Test", "test.Ice.operations");
        properties.setProperty("Ice.BatchAutoFlushSize", "100");
        java.io.PrintWriter out = getWriter();
        try(com.zeroc.Ice.Communicator communicator = initialize(properties))
        {
            MyClassPrx myClass = AllTests.allTests(this);

            out.print("testing server shutdown... ");
            out.flush();
            myClass.shutdown();
            try
            {
                myClass.ice_timeout(100).ice_ping(); // Use timeout to speed up testing on Windows
                throw new RuntimeException();
            }
            catch(com.zeroc.Ice.LocalException ex)
            {
                out.println("ok");
            }
        }
    }
}
