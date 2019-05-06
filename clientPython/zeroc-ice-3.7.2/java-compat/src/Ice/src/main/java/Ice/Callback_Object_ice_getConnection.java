//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package Ice;

/**
 * Callback object for {@link ObjectPrx#begin_ice_getConnection}.
 **/
public abstract class Callback_Object_ice_getConnection extends IceInternal.TwowayCallback
    implements Ice.TwowayCallbackArg1<Ice.Connection>
{
    /**
     * Called when the invocation completes successfully.
     *
     * @param ret The connection being used by the proxy.
     **/
    @Override
    public abstract void response(Ice.Connection ret);

    @Override
    public final void _iceCompleted(AsyncResult result)
    {
        ObjectPrxHelperBase._iceI_ice_getConnection_completed(this, result);
    }
}
