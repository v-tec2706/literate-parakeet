//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package Ice;

/**
 * Callback object for {@link ObjectPrx#begin_ice_id}.
 **/
public abstract class Callback_Object_ice_id extends IceInternal.TwowayCallback
    implements Ice.TwowayCallbackArg1<String>
{
    /**
     * Called when the invocation completes successfully.
     *
     * @param ret The Slice type id of the most-derived interface supported by the target object.
     **/
    @Override
    public abstract void response(String ret);

    @Override
    public final void _iceCompleted(AsyncResult result)
    {
        ObjectPrxHelperBase._iceI_ice_id_completed(this, result);
    }
}
