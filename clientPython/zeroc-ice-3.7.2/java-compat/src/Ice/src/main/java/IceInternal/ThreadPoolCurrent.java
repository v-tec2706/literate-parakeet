//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package IceInternal;

public final class ThreadPoolCurrent
{
    ThreadPoolCurrent(Instance instance, ThreadPool threadPool, ThreadPool.EventHandlerThread thread)
    {
        operation = SocketOperation.None;
        stream = new Ice.InputStream(instance, Protocol.currentProtocolEncoding);

        _threadPool = threadPool;
        _thread = thread;
        _ioCompleted = false;
        _leader = false;
    }

    public int operation;
    public Ice.InputStream stream; // A per-thread stream to be used by event handlers for optimization.

    public boolean
    ioReady()
    {
        return (_handler._registered & operation) > 0;
    }

    public void
    ioCompleted()
    {
        _threadPool.ioCompleted(this);
    }

    public void
    dispatchFromThisThread(DispatchWorkItem workItem)
    {
        _threadPool.dispatchFromThisThread(workItem);
    }

    final ThreadPool _threadPool;
    final ThreadPool.EventHandlerThread _thread;
    EventHandler _handler;
    boolean _ioCompleted;
    boolean _leader;
}
