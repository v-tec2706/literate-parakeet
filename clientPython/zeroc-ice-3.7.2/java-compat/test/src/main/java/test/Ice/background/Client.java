//
// Copyright (c) ZeroC, Inc. All rights reserved.
//
package test.Ice.background;

import test.Ice.background.PluginFactory.PluginI;
import test.Ice.background.Test.BackgroundPrx;

public class Client extends test.TestHelper
{
    public void
    run(String[] args)
    {
        Ice.Properties properties = createTestProperties(args);

        // For this test, we want to disable retries.
        //
        properties.setProperty("Ice.RetryIntervals", "-1");

        //
        // This test kills connections, so we don't want warnings.
        //
        properties.setProperty("Ice.Warn.Connections", "0");

        // This test relies on filling the TCP send/recv buffer, so
        // we rely on a fixed value for these buffers.
        properties.setProperty("Ice.TCP.SndSize", "50000");

        //
        // Setup the test transport plug-in.
        //
        properties.setProperty("Ice.Plugin.Test", "test.Ice.background.PluginFactory");
        String defaultProtocol = properties.getPropertyWithDefault("Ice.Default.Protocol", "tcp");
        properties.setProperty("Ice.Default.Protocol", "test-" + defaultProtocol);

        properties.setProperty("Ice.Package.Test", "test.Ice.background");

        try(Ice.Communicator communicator = initialize(properties))
        {
            PluginI plugin = (PluginI)communicator.getPluginManager().getPlugin("Test");
            BackgroundPrx background = AllTests.allTests(this, plugin.getConfiguration());
            background.shutdown();
        }
    }
}
