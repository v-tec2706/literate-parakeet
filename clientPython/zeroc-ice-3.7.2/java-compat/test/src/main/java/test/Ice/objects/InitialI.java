//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package test.Ice.objects;

import test.Ice.objects.Test.AMD_Initial_getAMDMB;
import test.Ice.objects.Test.B;
import test.Ice.objects.Test.BHolder;
import test.Ice.objects.Test.C;
import test.Ice.objects.Test.CHolder;
import test.Ice.objects.Test.D;
import test.Ice.objects.Test.DHolder;
import test.Ice.objects.Test.E;
import test.Ice.objects.Test.F;
import test.Ice.objects.Test.G;
import test.Ice.objects.Test.I;
import test.Ice.objects.Test.K;
import test.Ice.objects.Test.L;
import test.Ice.objects.Test.ValueSeqHolder;
import test.Ice.objects.Test.ValueMapHolder;
import test.Ice.objects.Test.A1;
import test.Ice.objects.Test.D1;
import test.Ice.objects.Test.EDerived;
import test.Ice.objects.Test.Base;
import test.Ice.objects.Test.BaseSeqHolder;
import test.Ice.objects.Test.Recursive;
import test.Ice.objects.Test.Initial;
import test.Ice.objects.Test.Compact;
import test.Ice.objects.Test.CompactExt;
import test.Ice.objects.Test.M;
import test.Ice.objects.Test.MHolder;

public final class InitialI extends Initial
{
    public
    InitialI(Ice.ObjectAdapter adapter)
    {
        _adapter = adapter;
        _b1 = new BI();
        _b2 = new BI();
        _c = new CI();
        _d = new DI();
        _e = new EI();
        _f = new FI(_e);

        _b1.theA = _b2; // Cyclic reference to another B
        _b1.theB = _b1; // Self reference.
        _b1.theC = null; // Null reference.

        _b2.theA = _b2; // Self reference, using base.
        _b2.theB = _b1; // Cyclic reference to another B
        _b2.theC = _c; // Cyclic reference to a C.

        _c.theB = _b2; // Cyclic reference to a B.

        _d.theA = _b1; // Reference to a B.
        _d.theB = _b2; // Reference to a B.
        _d.theC = null; // Reference to a C.
    }

    @Override
    public void
    getAll(BHolder b1, BHolder b2, CHolder c, DHolder d, Ice.Current current)
    {
        _b1.preMarshalInvoked = false;
        _b2.preMarshalInvoked = false;
        _c.preMarshalInvoked = false;
        _d.preMarshalInvoked = false;
        b1.value = _b1;
        b2.value = _b2;
        c.value = _c;
        d.value = _d;
    }

    @Override
    public B
    getB1(Ice.Current current)
    {
        _b1.preMarshalInvoked = false;
        _b2.preMarshalInvoked = false;
        _c.preMarshalInvoked = false;
        return _b1;
    }

    @Override
    public B
    getB2(Ice.Current current)
    {
        _b1.preMarshalInvoked = false;
        _b2.preMarshalInvoked = false;
        _c.preMarshalInvoked = false;
        return _b2;
    }

    @Override
    public C
    getC(Ice.Current current)
    {
        _b1.preMarshalInvoked = false;
        _b2.preMarshalInvoked = false;
        _c.preMarshalInvoked = false;
        return _c;
    }

    @Override
    public D
    getD(Ice.Current current)
    {
        _b1.preMarshalInvoked = false;
        _b2.preMarshalInvoked = false;
        _c.preMarshalInvoked = false;
        _d.preMarshalInvoked = false;
        return _d;
    }

    @Override
    public E
    getE(Ice.Current current)
    {
        return _e;
    }

    @Override
    public F
    getF(Ice.Current current)
    {
        return _f;
    }

    @Override
    public void
    setRecursive(Recursive r, Ice.Current current)
    {
    }

    @Override
    public boolean supportsClassGraphDepthMax(Ice.Current current)
    {
        return false;
    }

    @Override
    public B
    getMB(Ice.Current current)
    {
        return _b1;
    }

    @Override
    public void
    getAMDMB_async(AMD_Initial_getAMDMB cb, Ice.Current current)
    {
        cb.ice_response(_b1);
    }

    @Override
    public I
    getI(Ice.Current current)
    {
        return new II();
    }

    @Override
    public I
    getJ(Ice.Current current)
    {
        return new JI();
    }

    @Override
    public I
    getH(Ice.Current current)
    {
        return new HI();
    }

    @Override
    public K
    getK(Ice.Current current)
    {
        return new K(new L("l"));
    }

    @Override
    public Ice.Object
    opValue(Ice.Object v1, Ice.ObjectHolder v2, Ice.Current current)
    {
        v2.value = v1;
        return v1;
    }

    @Override
    public Ice.Object[]
    opValueSeq(Ice.Object[] v1, ValueSeqHolder v2, Ice.Current current)
    {
        v2.value = v1;
        return v1;
    }

    @Override
    public java.util.Map<String, Ice.Object>
    opValueMap(java.util.Map<String, Ice.Object> v1, ValueMapHolder v2, Ice.Current current)
    {
        v2.value = v1;
        return v1;
    }

    @Override
    public D1
    getD1(D1 d1, Ice.Current current)
    {
        return d1;
    }

    @Override
    public void
    throwEDerived(Ice.Current current) throws EDerived
    {
        throw new EDerived(new A1("a1"), new A1("a2"), new A1("a3"), new A1("a4"));
    }

    @Override
    public void
    setG(G theG, Ice.Current current)
    {
    }

    @Override
    public void
    setI(I theI, Ice.Current current)
    {
    }

    @Override
    public Base[]
    opBaseSeq(Base[] inS, BaseSeqHolder outS, Ice.Current current)
    {
        outS.value = inS;
        return inS;
    }

    @Override
    public Compact
    getCompact(Ice.Current current)
    {
        return new CompactExt();
    }

    @Override
    public test.Ice.objects.Test.Inner.A
    getInnerA(Ice.Current current)
    {
        return new test.Ice.objects.Test.Inner.A(_b1);
    }

    @Override
    public test.Ice.objects.Test.Inner.Sub.A
    getInnerSubA(Ice.Current current)
    {
        return new test.Ice.objects.Test.Inner.Sub.A(new test.Ice.objects.Test.Inner.A(_b1));
    }

    @Override
    public void throwInnerEx(Ice.Current current)
        throws test.Ice.objects.Test.Inner.Ex
    {
        throw new test.Ice.objects.Test.Inner.Ex("Inner::Ex");
    }

    @Override
    public void throwInnerSubEx(Ice.Current current)
        throws test.Ice.objects.Test.Inner.Sub.Ex
    {
        throw new test.Ice.objects.Test.Inner.Sub.Ex("Inner::Sub::Ex");
    }

    @Override
    public void
    shutdown(Ice.Current current)
    {
        _adapter.getCommunicator().shutdown();
    }

    @Override
    public M
    opM(M v1, MHolder v2, Ice.Current current)
    {
        v2.value = v1;
        return v1;
    }

    private Ice.ObjectAdapter _adapter;
    private B _b1;
    private B _b2;
    private C _c;
    private D _d;
    private E _e;
    private F _f;
}
