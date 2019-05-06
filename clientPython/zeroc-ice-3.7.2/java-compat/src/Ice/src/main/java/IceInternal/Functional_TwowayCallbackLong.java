//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package IceInternal;

public abstract class Functional_TwowayCallbackLong
    extends Functional_TwowayCallback implements Ice.TwowayCallbackLong
{
    public Functional_TwowayCallbackLong(Functional_LongCallback responseCb,
                                         Functional_GenericCallback1<Ice.Exception> exceptionCb,
                                         Functional_BoolCallback sentCb)
    {
        super(responseCb != null, exceptionCb, sentCb);
        _responseCb = responseCb;
    }

    protected Functional_TwowayCallbackLong(boolean userExceptionCb,
                                            Functional_LongCallback responseCb,
                                            Functional_GenericCallback1<Ice.Exception> exceptionCb,
                                            Functional_BoolCallback sentCb)
    {
        super(exceptionCb, sentCb);
        CallbackBase.check(responseCb != null || (userExceptionCb && exceptionCb != null));
        _responseCb = responseCb;
    }

    @Override
    public void response(long arg)
    {
        if(_responseCb != null)
        {
            _responseCb.apply(arg);
        }
    }

    final private Functional_LongCallback _responseCb;
}
