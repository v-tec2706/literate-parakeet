//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package test.Ice.adapterDeactivation;

import test.Ice.adapterDeactivation.Test.*;

public final class TestI implements TestIntf
{
    public void _transient(com.zeroc.Ice.Current current)
    {
        com.zeroc.Ice.Communicator communicator = current.adapter.getCommunicator();

        com.zeroc.Ice.ObjectAdapter adapter =
            communicator.createObjectAdapterWithEndpoints("TransientTestAdapter", "default");
        adapter.activate();
        adapter.destroy();
    }

    public void deactivate(com.zeroc.Ice.Current current)
    {
        current.adapter.deactivate();
        try
        {
            Thread.sleep(100);
        }
        catch(InterruptedException ex)
        {
        }
    }
}
