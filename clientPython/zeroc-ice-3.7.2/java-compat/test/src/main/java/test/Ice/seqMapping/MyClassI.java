//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package test.Ice.seqMapping;

import test.Ice.seqMapping.Test.*;
import test.Ice.seqMapping.Serialize.*;

public final class MyClassI extends _MyClassDisp
{
    @Override
    public void
    shutdown(Ice.Current current)
    {
        current.adapter.getCommunicator().shutdown();
    }

    @Override
    public Small opSerialSmallJava(Small i, Ice.Holder<Small> o, Ice.Current current)
    {
        o.value = i;
        return i;
    }

    @Override
    public Large opSerialLargeJava(Large i, Ice.Holder<Large> o, Ice.Current current)
    {
        o.value = i;
        return i;
    }

    @Override
    public Struct opSerialStructJava(Struct i, Ice.Holder<Struct> o, Ice.Current current)
    {
        o.value = i;
        return i;
    }
}
