//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package Ice;

/**
 * Callback object for {@link ObjectPrx#begin_ice_isA}.
 **/
public abstract class Callback_Object_ice_isA extends IceInternal.TwowayCallback implements Ice.TwowayCallbackBool
{
    /**
     * Called when the invocation completes successfully.
     *
     * @param ret True if the target object supports the given interface, false otherwise.
     **/
    @Override
    public abstract void response(boolean ret);

    @Override
    public final void _iceCompleted(AsyncResult result)
    {
        ObjectPrxHelperBase._iceI_ice_isA_completed(this, result);
    }
}
