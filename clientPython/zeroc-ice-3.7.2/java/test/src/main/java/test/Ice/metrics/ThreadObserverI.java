//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package test.Ice.metrics;

class ThreadObserverI extends ObserverI implements com.zeroc.Ice.Instrumentation.ThreadObserver
{
    @Override
    public synchronized void reset()
    {
        super.reset();
        states = 0;
    }

    @Override
    public synchronized void stateChanged(com.zeroc.Ice.Instrumentation.ThreadState o,
                                          com.zeroc.Ice.Instrumentation.ThreadState n)
    {
        ++states;
    }

    int states;
}
