//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package Ice;

/**
 * An application can optionally supply an instance of this class in an
 * asynchronous invocation. The application must create a subclass and
 * implement the completed method.
 **/
public abstract class Callback extends IceInternal.CallbackBase
{
    /**
     * Invoked when the invocation completes. The subclass should
     * call the matching <code>end_OP</code> method on the proxy and
     * must be prepared to handle exceptions.
     *
     * @param r The asynchronous result object returned by the <code>begin_OP</code> method.
     **/
    public abstract void completed(AsyncResult r);

    /**
     * Invoked when the Ice run time has passed the outgoing message
     * buffer to the transport. The default implementation does nothing,
     * a subclass can override it if it needs to take action when the
     * message is successfully sent.
     *
     * @param r The asynchronous result object returned by the <code>begin_OP</code> method.
     **/
    public void sent(AsyncResult r)
    {
    }

    @Override
    public final void _iceCompleted(AsyncResult r)
    {
        completed(r);
    }

    @Override
    public final void _iceSent(AsyncResult r)
    {
        sent(r);
    }

    @Override
    public final boolean _iceHasSentCallback()
    {
        return true;
    }
}
