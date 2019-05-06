//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package test.Ice.metrics;
import test.Ice.metrics.Test.*;

public final class MetricsI extends _MetricsDisp
{
    public
    MetricsI()
    {
    }

    @Override
    public void
    op(Ice.Current current)
    {
    }

    @Override
    public void
    fail(Ice.Current current)
    {
        current.con.close(Ice.ConnectionClose.Forcefully);
    }

    @Override
    public void
    opWithUserException(Ice.Current current)
        throws UserEx
    {
        throw new UserEx();
    }

    @Override
    public void
    opWithRequestFailedException(Ice.Current current)
    {
        throw new Ice.ObjectNotExistException();
    }

    @Override
    public void
    opWithLocalException(Ice.Current current)
    {
        throw new Ice.SyscallException();
    }

    @Override
    public void
    opWithUnknownException(Ice.Current current)
    {
        throw new IllegalArgumentException();
    }

    @Override
    public void
    opByteS(byte[] bs, Ice.Current current)
    {
    }

    @Override
    public Ice.ObjectPrx
    getAdmin(Ice.Current current)
    {
        return current.adapter.getCommunicator().getAdmin();
    }

    @Override
    public void
    shutdown(Ice.Current current)
    {
        current.adapter.getCommunicator().shutdown();
    }
}
