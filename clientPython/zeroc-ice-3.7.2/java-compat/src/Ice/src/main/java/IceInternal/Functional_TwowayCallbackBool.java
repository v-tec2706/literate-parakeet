//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package IceInternal;

public abstract class Functional_TwowayCallbackBool extends Functional_TwowayCallback implements Ice.TwowayCallbackBool
{
    public Functional_TwowayCallbackBool(Functional_BoolCallback responseCb,
                                         Functional_GenericCallback1<Ice.Exception> exceptionCb,
                                         Functional_BoolCallback sentCb)
    {
        super(responseCb != null, exceptionCb, sentCb);
        this._responseCb = responseCb;
    }

    protected Functional_TwowayCallbackBool(boolean userExceptionCb,
                                            Functional_BoolCallback responseCb,
                                            Functional_GenericCallback1<Ice.Exception> exceptionCb,
                                            Functional_BoolCallback sentCb)
    {
        super(exceptionCb, sentCb);
        CallbackBase.check(responseCb != null || (userExceptionCb && exceptionCb != null));
        this._responseCb = responseCb;
    }

    @Override
    public void response(boolean arg)
    {
        if(_responseCb != null)
        {
            _responseCb.apply(arg);
        }
    }

    final private Functional_BoolCallback _responseCb;
}
