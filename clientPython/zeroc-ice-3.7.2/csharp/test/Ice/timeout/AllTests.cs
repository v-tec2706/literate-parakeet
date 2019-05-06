//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Diagnostics;
using System.Threading;

namespace Ice
{
    namespace timeout
    {
        public class AllTests : global::Test.AllTests
        {
            private class Callback
            {
                internal Callback()
                {
                    _called = false;
                }

                public virtual void check()
                {
                    lock(this)
                    {
                        while(!_called)
                        {
                            Monitor.Wait(this);
                        }

                        _called = false;
                    }
                }

                public virtual void called()
                {
                    lock(this)
                    {
                        Debug.Assert(!_called);
                        _called = true;
                        Monitor.Pulse(this);
                    }
                }

                private bool _called;
            }

            private static Ice.Connection connect(Ice.ObjectPrx prx)
            {
                int nRetry = 10;
                while(--nRetry > 0)
                {
                    try
                    {
                        prx.ice_getConnection();
                        break;
                    }
                    catch(Ice.ConnectTimeoutException)
                    {
                        // Can sporadically occur with slow machines
                    }
                }
                return prx.ice_getConnection();
            }

            public static void allTests(global::Test.TestHelper helper)
            {
                var communicator = helper.communicator();
                string sref = "timeout:" + helper.getTestEndpoint(0);
                var obj = communicator.stringToProxy(sref);
                test(obj != null);

                Test.TimeoutPrx timeout = Test.TimeoutPrxHelper.checkedCast(obj);
                test(timeout != null);

                Test.ControllerPrx controller =
                    Test.ControllerPrxHelper.checkedCast(communicator.stringToProxy("controller:" + helper.getTestEndpoint(1)));
                test(controller != null);

                var output = helper.getWriter();
                output.Write("testing connect timeout... ");
                output.Flush();
                {
                    //
                    // Expect ConnectTimeoutException.
                    //
                    Test.TimeoutPrx to = Test.TimeoutPrxHelper.uncheckedCast(obj.ice_timeout(100));
                    controller.holdAdapter(-1);
                    try
                    {
                        to.op();
                        test(false);
                    }
                    catch(Ice.ConnectTimeoutException)
                    {
                        // Expected.
                    }
                    controller.resumeAdapter();
                    timeout.op(); // Ensure adapter is active.
                }
                {
                    //
                    // Expect success.
                    //
                    Test.TimeoutPrx to = Test.TimeoutPrxHelper.uncheckedCast(obj.ice_timeout(-1));
                    controller.holdAdapter(100);
                    try
                    {
                        to.op();
                    }
                    catch(Ice.ConnectTimeoutException)
                    {
                        test(false);
                    }
                }
                output.WriteLine("ok");

                // The sequence needs to be large enough to fill the write/recv buffers
                byte[] seq = new byte[2000000];

                output.Write("testing connection timeout... ");
                output.Flush();
                {
                    //
                    // Expect TimeoutException.
                    //
                    Test.TimeoutPrx to = Test.TimeoutPrxHelper.uncheckedCast(obj.ice_timeout(250));
                    connect(to);
                    controller.holdAdapter(-1);
                    try
                    {
                        to.sendData(seq);
                        test(false);
                    }
                    catch(Ice.TimeoutException)
                    {
                        // Expected.
                    }
                    controller.resumeAdapter();
                    timeout.op(); // Ensure adapter is active.
                }
                {
                    //
                    // Expect success.
                    //
                    Test.TimeoutPrx to = Test.TimeoutPrxHelper.uncheckedCast(obj.ice_timeout(2000));
                    controller.holdAdapter(100);
                    try
                    {
                        to.sendData(new byte[1000000]);
                    }
                    catch(Ice.TimeoutException)
                    {
                        test(false);
                    }
                }
                output.WriteLine("ok");

                output.Write("testing invocation timeout... ");
                output.Flush();
                {
                    var connection = obj.ice_getConnection();
                    Test.TimeoutPrx to = Test.TimeoutPrxHelper.uncheckedCast(obj.ice_invocationTimeout(100));
                    test(connection == to.ice_getConnection());
                    try
                    {
                        to.sleep(500);
                        test(false);
                    }
                    catch(Ice.InvocationTimeoutException)
                    {
                    }
                    obj.ice_ping();
                    to = Test.TimeoutPrxHelper.checkedCast(obj.ice_invocationTimeout(1000));
                    test(connection == to.ice_getConnection());
                    try
                    {
                        to.sleep(100);
                    }
                    catch(Ice.InvocationTimeoutException)
                    {
                        test(false);
                    }
                    test(connection == to.ice_getConnection());
                }
                {
                    //
                    // Expect InvocationTimeoutException.
                    //
                    Test.TimeoutPrx to = Test.TimeoutPrxHelper.uncheckedCast(obj.ice_invocationTimeout(100));
                    Callback cb = new Callback();
                    to.begin_sleep(500).whenCompleted(
                       () =>
                        {
                            test(false);
                        },
                       (Ice.Exception ex) =>
                        {
                            test(ex is Ice.InvocationTimeoutException);
                            cb.called();
                        });
                    cb.check();
                    obj.ice_ping();
                }
                {
                    //
                    // Expect success.
                    //
                    Test.TimeoutPrx to = Test.TimeoutPrxHelper.uncheckedCast(obj.ice_invocationTimeout(1000));
                    Callback cb = new Callback();
                    to.begin_sleep(100).whenCompleted(
                       () =>
                        {
                            cb.called();

                        },
                       (Ice.Exception ex) =>
                        {
                            test(false);
                        });
                    cb.check();
                }
                {
                    //
                    // Backward compatible connection timeouts
                    //
                    Test.TimeoutPrx to = Test.TimeoutPrxHelper.uncheckedCast(obj.ice_invocationTimeout(-2).ice_timeout(250));
                    var con = connect(to);
                    try
                    {
                        to.sleep(750);
                        test(false);
                    }
                    catch(Ice.TimeoutException)
                    {
                        try
                        {
                            con.getInfo();
                            test(false);
                        }
                        catch(Ice.TimeoutException)
                        {
                            // Connection got closed as well.
                        }
                    }
                    obj.ice_ping();

                    try
                    {
                        con = connect(to);
                        to.end_sleep(to.begin_sleep(750));
                        test(false);
                    }
                    catch(Ice.TimeoutException)
                    {
                        try
                        {
                            con.getInfo();
                            test(false);
                        }
                        catch(Ice.TimeoutException)
                        {
                            // Connection got closed as well.
                        }
                    }
                    obj.ice_ping();
                }
                output.WriteLine("ok");

                output.Write("testing close timeout... ");
                output.Flush();
                {
                    Test.TimeoutPrx to = Test.TimeoutPrxHelper.uncheckedCast(obj.ice_timeout(250));
                    var connection = connect(to);
                    controller.holdAdapter(-1);
                    connection.close(Ice.ConnectionClose.GracefullyWithWait);
                    try
                    {
                        connection.getInfo(); // getInfo() doesn't throw in the closing state.
                    }
                    catch(Ice.LocalException)
                    {
                        test(false);
                    }
                    while(true)
                    {
                        try
                        {
                            connection.getInfo();
                            Thread.Sleep(10);
                        }
                        catch(Ice.ConnectionManuallyClosedException ex)
                        {
                            // Expected.
                            test(ex.graceful);
                            break;
                        }
                    }
                    controller.resumeAdapter();
                    timeout.op(); // Ensure adapter is active.
                }
                output.WriteLine("ok");

                output.Write("testing timeout overrides... ");
                output.Flush();
                {
                    //
                    // Test Ice.Override.Timeout. This property overrides all
                    // endpoint timeouts.
                    //
                    var initData = new Ice.InitializationData();
                    initData.properties = communicator.getProperties().ice_clone_();
                    initData.properties.setProperty("Ice.Override.ConnectTimeout", "250");
                    initData.properties.setProperty("Ice.Override.Timeout", "100");
                    var comm = helper.initialize(initData);
                    Test.TimeoutPrx to = Test.TimeoutPrxHelper.uncheckedCast(comm.stringToProxy(sref));
                    connect(to);
                    controller.holdAdapter(-1);
                    try
                    {
                        to.sendData(seq);
                        test(false);
                    }
                    catch(Ice.TimeoutException)
                    {
                        // Expected.
                    }
                    controller.resumeAdapter();
                    timeout.op(); // Ensure adapter is active.

                    //
                    // Calling ice_timeout() should have no effect.
                    //
                    to = Test.TimeoutPrxHelper.uncheckedCast(to.ice_timeout(1000));
                    connect(to);
                    controller.holdAdapter(-1);
                    try
                    {
                        to.sendData(seq);
                        test(false);
                    }
                    catch(Ice.TimeoutException)
                    {
                        // Expected.
                    }
                    controller.resumeAdapter();
                    timeout.op(); // Ensure adapter is active.
                    comm.destroy();
                }
                {
                    //
                    // Test Ice.Override.ConnectTimeout.
                    //
                    var initData = new Ice.InitializationData();
                    initData.properties = communicator.getProperties().ice_clone_();
                    initData.properties.setProperty("Ice.Override.ConnectTimeout", "250");
                    var comm = helper.initialize(initData);
                    controller.holdAdapter(-1);
                    Test.TimeoutPrx to = Test.TimeoutPrxHelper.uncheckedCast(comm.stringToProxy(sref));
                    try
                    {
                        to.op();
                        test(false);
                    }
                    catch(Ice.ConnectTimeoutException)
                    {
                        // Expected.
                    }
                    controller.resumeAdapter();
                    timeout.op(); // Ensure adapter is active.

                    //
                    // Calling ice_timeout() should have no effect on the connect timeout.
                    //
                    controller.holdAdapter(-1);
                    to = Test.TimeoutPrxHelper.uncheckedCast(to.ice_timeout(1000));
                    try
                    {
                        to.op();
                        test(false);
                    }
                    catch(Ice.ConnectTimeoutException)
                    {
                        // Expected.
                    }
                    controller.resumeAdapter();
                    timeout.op(); // Ensure adapter is active.

                    //
                    // Verify that timeout set via ice_timeout() is still used for requests.
                    //
                    to = Test.TimeoutPrxHelper.uncheckedCast(to.ice_timeout(250));
                    connect(to);
                    controller.holdAdapter(-1);
                    try
                    {
                        to.sendData(seq);
                        test(false);
                    }
                    catch(Ice.TimeoutException)
                    {
                        // Expected.
                    }
                    controller.resumeAdapter();
                    timeout.op(); // Ensure adapter is active.
                    comm.destroy();
                }
                {
                    //
                    // Test Ice.Override.CloseTimeout.
                    //
                    var initData = new Ice.InitializationData();
                    initData.properties = communicator.getProperties().ice_clone_();
                    initData.properties.setProperty("Ice.Override.CloseTimeout", "100");
                    var comm = helper.initialize(initData);
                    comm.stringToProxy(sref).ice_getConnection();
                    controller.holdAdapter(-1);
                    long begin = System.DateTime.Now.Ticks;
                    comm.destroy();
                    test(((long)new System.TimeSpan(System.DateTime.Now.Ticks - begin).TotalMilliseconds - begin) < 1000);
                    controller.resumeAdapter();
                }
                output.WriteLine("ok");

                output.Write("testing invocation timeouts with collocated calls... ");
                output.Flush();
                {
                    communicator.getProperties().setProperty("TimeoutCollocated.AdapterId", "timeoutAdapter");

                    var adapter = communicator.createObjectAdapter("TimeoutCollocated");
                    adapter.activate();

                    Test.TimeoutPrx proxy = Test.TimeoutPrxHelper.uncheckedCast(adapter.addWithUUID(new TimeoutI()));
                    proxy =(Test.TimeoutPrx)proxy.ice_invocationTimeout(100);
                    try
                    {
                        proxy.sleep(500);
                        test(false);
                    }
                    catch(Ice.InvocationTimeoutException)
                    {
                    }

                    try
                    {
                        proxy.end_sleep(proxy.begin_sleep(500));
                        test(false);
                    }
                    catch(Ice.InvocationTimeoutException)
                    {
                    }

                    try
                    {
                       ((Test.TimeoutPrx)proxy.ice_invocationTimeout(-2)).ice_ping();
                       ((Test.TimeoutPrx)proxy.ice_invocationTimeout(-2)).begin_ice_ping().waitForCompleted();
                    }
                    catch(Ice.Exception)
                    {
                        test(false);
                    }

                    Test.TimeoutPrx batchTimeout =(Test.TimeoutPrx)proxy.ice_batchOneway();
                    batchTimeout.ice_ping();
                    batchTimeout.ice_ping();
                    batchTimeout.ice_ping();

                   ((Test.TimeoutPrx)proxy.ice_invocationTimeout(-1)).begin_sleep(300); // Keep the server thread pool busy.
                    try
                    {
                        batchTimeout.ice_flushBatchRequests();
                        test(false);
                    }
                    catch(Ice.InvocationTimeoutException)
                    {
                    }

                    batchTimeout.ice_ping();
                    batchTimeout.ice_ping();
                    batchTimeout.ice_ping();

                   ((Test.TimeoutPrx)proxy.ice_invocationTimeout(-1)).begin_sleep(300); // Keep the server thread pool busy.
                    try
                    {
                        batchTimeout.end_ice_flushBatchRequests(batchTimeout.begin_ice_flushBatchRequests());
                        test(false);
                    }
                    catch(Ice.InvocationTimeoutException)
                    {
                    }

                    adapter.destroy();
                }
                output.WriteLine("ok");

                controller.shutdown();
            }
        }
    }
}
