//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package test.Ice.operations;

import test.Ice.operations.Test.MyClassPrx;

class Oneways
{
    private static void test(boolean b)
    {
        if(!b)
        {
            throw new RuntimeException();
        }
    }

    static void oneways(test.TestHelper helper, MyClassPrx p)
    {
        p = p.ice_oneway();

        p.ice_ping();
        p.opVoid();
        p.opIdempotent();
        p.opNonmutating();

        try
        {
            p.opByte((byte)0xff, (byte)0x0f);
            test(false);
        }
        catch(com.zeroc.Ice.TwowayOnlyException ex)
        {
        }
    }
}
