//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package Ice;

/**
 * A class that encapsulates data to initialize a communicator.
 *
 * @see Util#initialize
 * @see Properties
 * @see Logger
 * @see ThreadNotification
 **/
public final class InitializationData implements Cloneable
{
    /**
     * Creates an instance with all members set to <code>null</code>.
     **/
    public
    InitializationData()
    {
    }

    /**
     * Creates and returns a copy of this object.
     **/
    @Override
    public InitializationData
    clone()
    {
        //
        // A member-wise copy is safe because the members are immutable.
        //
        InitializationData c = null;
        try
        {
            c = (InitializationData)super.clone();
        }
        catch(CloneNotSupportedException ex)
        {
            assert false;
        }
        return c;
    }

    /**
     * The properties for the communicator.
     **/
    public Properties properties;

    /**
     * The logger for the communicator.
     **/
    public Logger logger;

    /**
     * The communicator observer used by the Ice run-time.
     **/
    public Ice.Instrumentation.CommunicatorObserver observer;

    /**
     * The thread hook for the communicator.
     **/
    public ThreadNotification threadHook;

    /**
     * The custom class loader for the communicator.
     **/
    public ClassLoader classLoader;

    /**
     * The call dispatcher for the communicator.
     **/
    public Dispatcher dispatcher;

    /**
     * The compact type ID resolver.
     **/
    public CompactIdResolver compactIdResolver;

    /**
     * The batch request interceptor.
     **/
    public BatchRequestInterceptor batchRequestInterceptor;

    /**
     * The value factory manager.
     **/
    public ValueFactoryManager valueFactoryManager;
}
