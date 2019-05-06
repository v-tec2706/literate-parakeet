//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package Ice;

/**
 * Callback object for {@link ObjectPrx#begin_ice_ids}.
 **/
public abstract class Callback_Object_ice_ids extends IceInternal.TwowayCallback
    implements Ice.TwowayCallbackArg1<String[]>
{
    /**
     * Called when the invocation completes successfully.
     *
     * @param ret The Slice type ids of the interfaces supported by the target object.
     **/
    @Override
    public abstract void response(String[] ret);

    @Override
    public final void _iceCompleted(AsyncResult result)
    {
        ObjectPrxHelperBase._iceI_ice_ids_completed(this, result);
    }
}
