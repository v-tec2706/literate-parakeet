//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package test.Ice.timeout;

import test.Ice.timeout.Test.Timeout;

class TimeoutI implements Timeout
{
    @Override
    public void op(com.zeroc.Ice.Current current)
    {
    }

    @Override
    public void sendData(byte[] seq, com.zeroc.Ice.Current current)
    {
    }

    @Override
    public void sleep(int to, com.zeroc.Ice.Current current)
    {
        try
        {
            Thread.sleep(to);
        }
        catch(InterruptedException ex)
        {
            System.err.println("sleep interrupted");
        }
    }
}
