//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package test.Ice.interrupt;

public class TestI implements test.Ice.interrupt.Test.TestIntf
{
    TestI(TestControllerI controller)
    {
        _controller = controller;
    }

    @Override
    public void op(com.zeroc.Ice.Current current)
    {
    }

    @Override
    public void opIdempotent(com.zeroc.Ice.Current current)
    {
        throw new com.zeroc.Ice.UnknownException();
    }

    @Override
    public void sleep(int to, com.zeroc.Ice.Current current)
        throws test.Ice.interrupt.Test.InterruptedException
    {
        _controller.addUpcallThread();
        try
        {
            Thread.sleep(to);
        }
        catch(InterruptedException ex)
        {
            throw new test.Ice.interrupt.Test.InterruptedException();
        }
        finally
        {
            _controller.removeUpcallThread();
        }
    }

    @Override
    public void opWithPayload(byte[] seq, com.zeroc.Ice.Current current)
    {
    }

    @Override
    public void shutdown(com.zeroc.Ice.Current current)
    {
        current.adapter.getCommunicator().shutdown();
    }

    private TestControllerI _controller;
}
