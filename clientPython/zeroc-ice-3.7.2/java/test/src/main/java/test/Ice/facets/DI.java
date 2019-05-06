//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package test.Ice.facets;

import test.Ice.facets.Test.D;

public final class DI implements D
{
    public DI()
    {
    }

    @Override
    public String callA(com.zeroc.Ice.Current current)
    {
        return "A";
    }

    @Override
    public String callB(com.zeroc.Ice.Current current)
    {
        return "B";
    }

    @Override
    public String callC(com.zeroc.Ice.Current current)
    {
        return "C";
    }

    @Override
    public String callD(com.zeroc.Ice.Current current)
    {
        return "D";
    }
}
