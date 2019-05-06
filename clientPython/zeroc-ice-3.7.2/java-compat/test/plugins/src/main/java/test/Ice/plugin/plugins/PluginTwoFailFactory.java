//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package test.Ice.plugin.plugins;

public class PluginTwoFailFactory implements Ice.PluginFactory
{
    @Override
    public Ice.Plugin create(Ice.Communicator communicator, String name, String[] args)
    {
        return new PluginTwoFail(communicator);
    }

    static class PluginTwoFail extends BasePluginFail
    {
        public PluginTwoFail(Ice.Communicator communicator)
        {
            super(communicator);
        }

        @Override
        public void initialize()
        {
            _one = (BasePluginFail)_communicator.getPluginManager().getPlugin("PluginOneFail");
            test(_one.isInitialized());
            _three = (BasePluginFail)_communicator.getPluginManager().getPlugin("PluginThreeFail");
            test(!_three.isInitialized());
            _initialized = true;
        }

        @Override
        public void destroy()
        {
            test(!_one.isDestroyed());
            //
            // Not destroyed because initialize fails.
            //
            test(!_three.isDestroyed());
            _destroyed = true;
        }

        @SuppressWarnings("deprecation")
        @Override
        protected void finalize() throws Throwable
        {
            try
            {
                if(!_initialized)
                {
                    System.out.println(getClass().getName() + " not initialized");
                }
                if(!_destroyed)
                {
                    System.out.println(getClass().getName() + " not destroyed");
                }
            }
            finally
            {
                super.finalize();
            }
        }
    }
}
