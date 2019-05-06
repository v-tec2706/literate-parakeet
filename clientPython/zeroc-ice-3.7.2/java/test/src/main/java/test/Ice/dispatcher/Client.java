//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package test.Ice.dispatcher;

public class Client extends test.TestHelper
{
    public void run(String[] args)
    {
        com.zeroc.Ice.InitializationData initData = new com.zeroc.Ice.InitializationData();
        initData.properties = createTestProperties(args);
        initData.properties.setProperty("Ice.Package.Test", "test.Ice.dispatcher");

        Dispatcher dispatcher = new Dispatcher();
        //
        // Limit the send buffer size, this test relies on the socket
        // send() blocking after sending a given amount of data.
        //
        initData.properties.setProperty("Ice.TCP.SndSize", "50000");

        initData.dispatcher = dispatcher;
        try(com.zeroc.Ice.Communicator communicator = initialize(initData))
        {
            AllTests.allTests(this, dispatcher);
        }
        dispatcher.terminate();
    }
}
