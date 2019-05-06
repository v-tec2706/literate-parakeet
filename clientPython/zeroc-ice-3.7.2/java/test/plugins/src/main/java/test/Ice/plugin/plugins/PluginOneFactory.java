//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package test.Ice.plugin.plugins;

public class PluginOneFactory implements com.zeroc.Ice.PluginFactory
{
    @Override
    public com.zeroc.Ice.Plugin create(com.zeroc.Ice.Communicator communicator, String name, String[] args)
    {
        return new PluginOne(communicator);
    }

    static class PluginOne extends BasePlugin
    {
        public PluginOne(com.zeroc.Ice.Communicator communicator)
        {
            super(communicator);
        }

        @Override
        public void initialize()
        {
            _other = (BasePlugin)_communicator.getPluginManager().getPlugin("PluginTwo");
            test(!_other.isInitialized());
            _initialized = true;
        }

        @Override
        public void destroy()
        {
            _destroyed = true;
            test(_other.isDestroyed());
        }
    }
}
