//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package test.Ice.objects;

import test.Ice.objects.Test.Initial;

public class Collocated extends test.TestHelper
{
    private static class MyValueFactory implements com.zeroc.Ice.ValueFactory
    {
        @Override
        public com.zeroc.Ice.Value create(String type)
        {
            if(type.equals("::Test::B"))
            {
                return new BI();
            }
            else if(type.equals("::Test::C"))
            {
                return new CI();
            }
            else if(type.equals("::Test::D"))
            {
                return new DI();
            }
            else if(type.equals("::Test::E"))
            {
                return new EI();
            }
            else if(type.equals("::Test::F"))
            {
                return new FI();
            }
            else if(type.equals("::Test::I"))
            {
                return new II();
            }
            else if(type.equals("::Test::J"))
            {
                return new JI();
            }
            else if(type.equals("::Test::H"))
            {
                return new HI();
            }

            assert (false); // Should never be reached
            return null;
        }
    }

    @SuppressWarnings("deprecation")
    private static class MyObjectFactory implements com.zeroc.Ice.ObjectFactory
    {
        @Override
        public com.zeroc.Ice.Value create(String type)
        {
            return null;
        }

        @Override
        public void destroy()
        {
            //
        }
    }

    @SuppressWarnings("deprecation")
    public void run(String[] args)
    {
        com.zeroc.Ice.Properties properties = createTestProperties(args);
        properties.setProperty("Ice.Package.Test", "test.Ice.objects");
        properties.setProperty("Ice.Warn.Dispatch", "0");

        try(com.zeroc.Ice.Communicator communicator = initialize(properties))
        {
            com.zeroc.Ice.ValueFactory factory = new MyValueFactory();
            communicator.getValueFactoryManager().add(factory, "::Test::B");
            communicator.getValueFactoryManager().add(factory, "::Test::C");
            communicator.getValueFactoryManager().add(factory, "::Test::D");
            communicator.getValueFactoryManager().add(factory, "::Test::E");
            communicator.getValueFactoryManager().add(factory, "::Test::F");
            communicator.getValueFactoryManager().add(factory, "::Test::I");
            communicator.getValueFactoryManager().add(factory, "::Test::J");
            communicator.getValueFactoryManager().add(factory, "::Test::H");

            communicator.addObjectFactory(new MyObjectFactory(), "TestOF");

            communicator.getProperties().setProperty("TestAdapter.Endpoints", getTestEndpoint(0));
            com.zeroc.Ice.ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter");
            Initial initial = new InitialI(adapter);
            adapter.add(initial, com.zeroc.Ice.Util.stringToIdentity("initial"));
            UnexpectedObjectExceptionTestI object = new UnexpectedObjectExceptionTestI();
            adapter.add(object, com.zeroc.Ice.Util.stringToIdentity("uoet"));
            AllTests.allTests(this);
            //
            // We must call shutdown even in the collocated case for cyclic dependency cleanup.
            //
            initial.shutdown(null);
        }
    }
}
