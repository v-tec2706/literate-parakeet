//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package IceInternal;

public class Functional_OnewayCallback extends IceInternal.Functional_CallbackBase
{
    public Functional_OnewayCallback(Functional_VoidCallback responseCb,
                                     Functional_GenericCallback1<Ice.Exception> exceptionCb,
                                     Functional_BoolCallback sentCb)
    {
        super(exceptionCb, sentCb);
        CallbackBase.check(responseCb != null || exceptionCb != null);
        _responseCb = responseCb;
    }

    @Override
    public final void _iceCompleted(Ice.AsyncResult result)
    {
        try
        {
            ((Ice.ObjectPrxHelperBase)result.getProxy())._end(result, result.getOperation());
            if(_responseCb != null)
            {
                _responseCb.apply();
            }
        }
        catch(Ice.Exception ex)
        {
            if(_exceptionCb != null)
            {
                _exceptionCb.apply(ex);
            }
        }
    }

    private final Functional_VoidCallback _responseCb;
}
