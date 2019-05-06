//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package test.Ice.faultTolerance;

import test.Ice.faultTolerance.Test.TestIntf;

public final class TestI implements TestIntf
{
    public TestI(int port)
    {
        _pseudoPid = port; // We use the port number instead of the process ID in Java.
    }

    @Override
    public void shutdown(com.zeroc.Ice.Current current)
    {
        current.adapter.getCommunicator().shutdown();
    }

    @Override
    public void abort(com.zeroc.Ice.Current current)
    {
        Runtime.getRuntime().halt(0);
    }

    @Override
    public void idempotentAbort(com.zeroc.Ice.Current current)
    {
        Runtime.getRuntime().halt(0);
    }

    @Override
    public int pid(com.zeroc.Ice.Current current)
    {
        return _pseudoPid;
    }

    private int _pseudoPid;
}
