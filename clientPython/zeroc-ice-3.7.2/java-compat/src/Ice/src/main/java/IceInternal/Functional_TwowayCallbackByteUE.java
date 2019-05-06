//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package IceInternal;

public abstract class Functional_TwowayCallbackByteUE
    extends Functional_TwowayCallbackByte implements Ice.TwowayCallbackByteUE
{
    public Functional_TwowayCallbackByteUE(
        Functional_ByteCallback responseCb,
        Functional_GenericCallback1<Ice.UserException> userExceptionCb,
        Functional_GenericCallback1<Ice.Exception> exceptionCb,
        Functional_BoolCallback sentCb)
    {
        super(userExceptionCb != null, responseCb, exceptionCb, sentCb);
        _userExceptionCb = userExceptionCb;
    }

    @Override
    public void exception(Ice.UserException ex)
    {
        if(_userExceptionCb != null)
        {
            _userExceptionCb.apply(ex);
        }
    }

    private final Functional_GenericCallback1<Ice.UserException> _userExceptionCb;
}
