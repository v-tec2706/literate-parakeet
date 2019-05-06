//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Diagnostics;
using System.Threading;
using Test;

public sealed class TestI : TestIntfDisp_
{
    public TestI()
    {
        lock(this)
        {
            _p = Process.GetCurrentProcess();
            _pid = _p.Id;
        }
    }

    private void commitSuicide()
    {
        _p.Kill();
        Thread.Sleep(5000); // Give other threads time to die.
    }

    public override void abort(Ice.Current current)
    {
        commitSuicide();
    }

    public override void idempotentAbort(Ice.Current current)
    {
        commitSuicide();
    }

    public override int pid(Ice.Current current)
    {
        lock(this)
        {
            return _pid;
        }
    }

    public override void shutdown(Ice.Current current)
    {
        current.adapter.getCommunicator().shutdown();
    }

    private Process _p;
    private int _pid;
}
