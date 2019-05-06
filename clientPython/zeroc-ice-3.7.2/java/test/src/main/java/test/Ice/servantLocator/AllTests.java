//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package test.Ice.servantLocator;

import java.io.PrintWriter;

import test.Ice.servantLocator.Test.TestImpossibleException;
import test.Ice.servantLocator.Test.TestIntfUserException;
import test.Ice.servantLocator.Test.TestIntfPrx;
import test.Ice.servantLocator.Test.TestActivationPrx;

import com.zeroc.Ice.ObjectNotExistException;
import com.zeroc.Ice.ObjectPrx;
import com.zeroc.Ice.UnknownException;
import com.zeroc.Ice.UnknownLocalException;
import com.zeroc.Ice.UnknownUserException;

public class AllTests
{
    private static void test(boolean b)
    {
        if(!b)
        {
            throw new RuntimeException();
        }
    }

    public static void testExceptions(TestIntfPrx obj)
    {
        try
        {
            obj.requestFailedException();
            test(false);
        }
        catch(ObjectNotExistException ex)
        {
            test(ex.id.equals(obj.ice_getIdentity()));
            test(ex.facet.equals(obj.ice_getFacet()));
            test(ex.operation.equals("requestFailedException"));
        }

        try
        {
            obj.unknownUserException();
            test(false);
        }
        catch(UnknownUserException ex)
        {
            test(ex.unknown.equals("reason"));
        }

        try
        {
            obj.unknownLocalException();
            test(false);
        }
        catch(UnknownLocalException ex)
        {
            test(ex.unknown.equals("reason"));
        }

        try
        {
            obj.unknownException();
            test(false);
        }
        catch(UnknownException ex)
        {
            test(ex.unknown.equals("reason"));
        }

        //
        // User exceptions are checked exceptions
        //
//      try
//      {
//          obj.userException();
//          test(false);
//      }
//      catch(UnknownUserException ex)
//      {
//          //System.err.println(ex.unknown);
//          test(!collocated);
//          test(ex.unknown.equals("Test::TestIntfUserException"));
//      }
//      catch(TestIntfUserException ex)
//      {
//          test(collocated);
//      }

        try
        {
            obj.localException();
            test(false);
        }
        catch(UnknownLocalException ex)
        {
            test(ex.unknown.indexOf("Ice::SocketException") >= 0 ||
                 ex.unknown.indexOf("Ice.SocketException") >= 0);
        }
        catch(Throwable ex)
        {
            test(false);
        }

        try
        {
            obj.javaException();
            test(false);
        }
        catch(UnknownException ex)
        {
            test(ex.unknown.indexOf("java.lang.RuntimeException: message") >= 0);
        }
        catch(com.zeroc.Ice.OperationNotExistException ex)
        {
        }
        catch(Throwable ex)
        {
            //System.err.println(ex);
            test(false);
        }

        try
        {
            obj.unknownExceptionWithServantException();
            test(false);
        }
        catch(UnknownException ex)
        {
            test(ex.unknown.equals("reason"));
        }
        catch(Throwable ex)
        {
            test(false);
        }

        try
        {
            obj.impossibleException(false);
            test(false);
        }
        catch(UnknownUserException ex)
        {
            // Operation doesn't throw, but locate() and finished() throw TestIntfUserException.
        }
        catch(Throwable ex)
        {
            //System.err.println(ex);
            test(false);
        }

        try
        {
            obj.impossibleException(true);
            test(false);
        }
        catch(UnknownUserException ex)
        {
            // Operation throws TestImpossibleException, but locate() and finished() throw TestIntfUserException.
        }
        catch(Throwable ex)
        {
            //System.err.println(ex);
            test(false);
        }

        try
        {
            obj.intfUserException(false);
            test(false);
        }
        catch(TestImpossibleException ex)
        {
            // Operation doesn't throw, but locate() and finished() throw TestImpossibleException.
        }
        catch(Throwable ex)
        {
            //System.err.println(ex);
            test(false);
        }

        try
        {
            obj.intfUserException(true);
            test(false);
        }
        catch(TestImpossibleException ex)
        {
            // Operation throws TestIntfUserException, but locate() and finished() throw TestImpossibleException.
        }
        catch(Throwable ex)
        {
            //System.err.println(ex);
            test(false);
        }
    }

    public static TestIntfPrx allTests(test.TestHelper helper)
    {
        com.zeroc.Ice.Communicator communicator = helper.communicator();
        PrintWriter out = helper.getWriter();
        out.print("testing stringToProxy... ");
        out.flush();
        String ref = "asm:" + helper.getTestEndpoint(0);
        com.zeroc.Ice.ObjectPrx base = communicator.stringToProxy(ref);
        test(base != null);
        out.println("ok");

        out.print("testing checked cast... ");
        out.flush();
        TestIntfPrx obj = TestIntfPrx.checkedCast(base);
        test(obj != null);
        test(obj.equals(base));
        out.println("ok");

        out.print("testing ice_ids... ");
        out.flush();
        try
        {
            ObjectPrx o = communicator.stringToProxy("category/locate:" + helper.getTestEndpoint(0));
            o.ice_ids();
            test(false);
        }
        catch(UnknownUserException ex)
        {
            test(ex.unknown.equals("::Test::TestIntfUserException"));
        }
        catch(Throwable ex)
        {
            test(false);
        }

        try
        {
            ObjectPrx o = communicator.stringToProxy("category/finished:" + helper.getTestEndpoint(0));
            o.ice_ids();
            test(false);
        }
        catch(UnknownUserException ex)
        {
            test(ex.unknown.equals("::Test::TestIntfUserException"));
        }
        catch(Throwable ex)
        {
            test(false);
        }
        out.println("ok");

        out.print("testing servant locator... ");
        out.flush();
        base = communicator.stringToProxy("category/locate:" + helper.getTestEndpoint(0));
        obj = TestIntfPrx.checkedCast(base);
        try
        {
            TestIntfPrx.checkedCast(communicator.stringToProxy("category/unknown:" + helper.getTestEndpoint(0)));
        }
        catch(ObjectNotExistException ex)
        {
        }
        out.println("ok");

        out.print("testing default servant locator... ");
        out.flush();
        base = communicator.stringToProxy("anothercat/locate:" + helper.getTestEndpoint(0));
        obj = TestIntfPrx.checkedCast(base);
        base = communicator.stringToProxy("locate:" + helper.getTestEndpoint(0));
        obj = TestIntfPrx.checkedCast(base);
        try
        {
            TestIntfPrx.checkedCast(communicator.stringToProxy("anothercat/unknown:" + helper.getTestEndpoint(0)));
        }
        catch(ObjectNotExistException ex)
        {
        }
        try
        {
            TestIntfPrx.checkedCast(communicator.stringToProxy("unknown:" + helper.getTestEndpoint(0)));
        }
        catch(ObjectNotExistException ex)
        {
        }
        out.println("ok");

        out.print("testing locate exceptions... ");
        out.flush();
        base = communicator.stringToProxy("category/locate:" + helper.getTestEndpoint(0));
        obj = TestIntfPrx.checkedCast(base);
        testExceptions(obj);
        out.println("ok");

        out.print("testing finished exceptions... ");
        out.flush();
        base = communicator.stringToProxy("category/finished:" + helper.getTestEndpoint(0));
        obj = TestIntfPrx.checkedCast(base);
        testExceptions(obj);

        //
        // Only call these for category/finished.
        //
        try
        {
            obj.asyncResponse();
        }
        catch(TestIntfUserException ex)
        {
            test(false);
        }
        catch(TestImpossibleException ex)
        {
            //
            // Called by finished().
            //
        }

        //
        // Only call these for category/finished.
        //
        try
        {
            obj.asyncException();
        }
        catch(TestIntfUserException ex)
        {
            test(false);
        }
        catch(TestImpossibleException ex)
        {
            //
            // Called by finished().
            //
        }

        out.println("ok");

        out.print("testing servant locator removal... ");
        out.flush();
        base = communicator.stringToProxy("test/activation:" + helper.getTestEndpoint(0));
        TestActivationPrx activation = TestActivationPrx.checkedCast(base);
        activation.activateServantLocator(false);
        try
        {
            obj.ice_ping();
            test(false);
        }
        catch(ObjectNotExistException ex)
        {
            out.println("ok");
        }
        out.print("testing servant locator addition... ");
        out.flush();
        activation.activateServantLocator(true);
        try
        {
            obj.ice_ping();
            out.println("ok");
        }
        catch(Exception ex)
        {
            test(false);
        }

        return obj;
    }
}
