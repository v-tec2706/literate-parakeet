//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package test.Ice.checksum;

import test.Ice.checksum.Test.ChecksumPrx;

public class Client extends test.TestHelper
{
    public void
    run(String[] args)
    {
        Ice.Properties properties = createTestProperties(args);
        properties.setProperty("Ice.Package.Test", "test.Ice.checksum");
        try(Ice.Communicator communicator = initialize(properties))
        {
            ChecksumPrx checksum = AllTests.allTests(this, false);
            checksum.shutdown();
        }
    }
}
