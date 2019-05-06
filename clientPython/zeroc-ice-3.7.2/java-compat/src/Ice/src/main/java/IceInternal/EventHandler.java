//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package IceInternal;

public abstract class EventHandler
{
    //
    // Called when there's a message ready to be processed.
    //
    abstract public void message(ThreadPoolCurrent current);

    //
    // Called when the event handler is unregistered.
    //
    abstract public void finished(ThreadPoolCurrent current, boolean close);

    //
    // Get a textual representation of the event handler.
    //
    @Override
    abstract public String toString();

    //
    // Get the native information of the handler, this is used by the selector.
    //
    abstract public java.nio.channels.SelectableChannel fd();

    //
    // Set the ready callback
    //
    abstract public void setReadyCallback(ReadyCallback callback);

    public int _disabled = 0;
    public int _registered = 0;
    public int _ready = 0;
    public java.nio.channels.SelectionKey _key = null;
}
